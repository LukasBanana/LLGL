#!/usr/bin/env python3

"""Translate HLSL shaders described by *.shaderinfo.yml files."""

import argparse
import importlib
import re
import shutil
import subprocess
import sys
from pathlib import Path

try:
    yaml = importlib.import_module("yaml")
except ModuleNotFoundError:
    yaml = None


STAGE_EXTENSIONS = {
    "vs": "vert",
    "ps": "frag",
    "gs": "geom",
    "hs": "tesc",
    "ds": "tese",
    "cs": "comp",
}
GLSL_TARGET_PATTERN = re.compile(r"^glsl(?P<version>\d+)(?P<flavor>core|es)$")
HIGHLIGHT_COLOR = "\033[1;33m"
ERROR_COLOR = "\033[1;31m"
RESET_COLOR = "\033[0m"


class ShaderInfoError(Exception):
    """Raised when a shader-info file does not match the supported schema."""


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Translate HLSL shaders described by *.shaderinfo.yml files."
    )
    parser.add_argument(
        "input_positional",
        nargs="?",
        metavar="DIR",
        help="Search folder; equivalent to --input.",
    )
    parser.add_argument(
        "-i", "--input",
        metavar="DIR",
        help="Search folder (default: examples/).",
    )
    parser.add_argument(
        "-s", "--search-depth",
        type=int,
        default=2,
        metavar="N",
        help="Maximum number of subfolder levels to search (default: 2).",
    )
    parser.add_argument(
        "-d", "--debug",
        action="store_true",
        help="Generate SPIR-V disassembly (*.spvasm) alongside SPIR-V binaries.",
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Print every generated output and compiler action.",
    )
    parser.add_argument(
        "-c", "--color",
        action="store_true",
        help="Highlight processed shader-info files with ANSI terminal colors.",
    )
    parser.add_argument(
        "-o", "--output",
        metavar="DIR",
        help=(
            "Output folder relative to each shader-info file. Use '.' to write beside "
            "the input source (default: Autogen/)."
        ),
    )
    parser.add_argument(
        "--trim-stem",
        default="VS,PS",
        metavar="ENTRIES",
        help="Entry names to omit from generated output stems (default: VS,PS).",
    )
    parser.add_argument(
        "-t", "--targets",
        metavar="TARGETS",
        help="Comma-separated output targets to enable (default: all configured targets).",
    )
    arguments = parser.parse_args()

    if arguments.input and arguments.input_positional:
        parser.error("specify the input directory either positionally or with --input, not both")
    if arguments.search_depth < 0:
        parser.error("--search-depth must be zero or greater")

    arguments.input = Path(arguments.input or arguments.input_positional or "examples")
    arguments.output = Path(arguments.output) if arguments.output else None
    if arguments.output and arguments.output.is_absolute():
        parser.error("--output must be a path relative to each shader-info file")
    arguments.trim_stem = {
        entry_name.strip()
        for entry_name in arguments.trim_stem.split(",")
        if entry_name.strip()
    }
    arguments.targets = (
        {
            target.strip()
            for target in arguments.targets.split(",")
            if target.strip()
        }
        if arguments.targets is not None
        else None
    )
    return arguments


def require_tool(tool: str, install_url: str) -> str:
    if shutil.which(tool) is None:
        raise RuntimeError(
            f"Required tool '{tool}' was not found on PATH. Install it from {install_url} "
            "and add its executable directory to PATH."
        )
    return tool


def require_glslang_tool() -> str:
    for tool in ("glslangValidator", "glslang"):
        if shutil.which(tool) is not None:
            return tool
    raise RuntimeError(
        "Required GLSL compiler 'glslangValidator' (or 'glslang') was not found on PATH. "
        "Install it from https://github.com/KhronosGroup/glslang and add its executable "
        "directory to PATH."
    )


def filter_targets(targets, enabled_targets):
    if enabled_targets is None:
        return targets
    return [target for target in targets if target in enabled_targets]


def verify_tools(debug, sources, enabled_targets) -> str | None:
    requested_targets = {
        target
        for source in sources
        for entry in source["entries"]
        for target in filter_targets(entry["targets"], enabled_targets)
    }
    has_hlsl_targets = any(
        Path(source["source"]).suffix.lower() == ".hlsl"
        and any(filter_targets(entry["targets"], enabled_targets) for entry in source["entries"])
        for source in sources
    )
    has_glsl_sources = any(
        Path(source["source"]).suffix.lower() != ".hlsl"
        and any(filter_targets(entry["targets"], enabled_targets) for entry in source["entries"])
        for source in sources
    )
    requires_spirv_cross = has_hlsl_targets or "metal" in requested_targets
    if has_hlsl_targets or "dxil" in requested_targets:
        require_tool("dxc", "https://github.com/microsoft/DirectXShaderCompiler")
    if requires_spirv_cross:
        require_tool("spirv-cross", "https://github.com/KhronosGroup/SPIRV-Cross")
    glslang_tool = require_glslang_tool() if has_glsl_sources else None
    if debug:
        require_tool("spirv-dis", "https://github.com/KhronosGroup/SPIRV-Tools")
    return glslang_tool


def unquote_yaml_string(value: str) -> str:
    value = value.strip()
    if len(value) >= 2 and value[0] == value[-1] and value[0] in "\"'":
        return value[1:-1]
    return value


def parse_shader_info_without_yaml(filename: Path) -> dict:
    sources = []
    current_source = None
    current_entry = None
    in_targets = False

    for line_number, raw_line in enumerate(filename.read_text(encoding="utf-8").splitlines(), 1):
        line = raw_line.split("#", 1)[0].strip()
        if not line or line == "sources:":
            continue

        source_match = re.fullmatch(r"-\s*source\s*:\s*(.+)", line)
        if source_match:
            current_source = {"source": unquote_yaml_string(source_match.group(1)), "entries": []}
            sources.append(current_source)
            current_entry = None
            in_targets = False
            continue

        if line == "entries:":
            if current_source is None:
                raise ShaderInfoError(f"{filename}:{line_number}: entries must belong to a source")
            current_entry = None
            in_targets = False
            continue

        entry_match = re.fullmatch(r"-\s*entry\s*:\s*(.+)", line)
        if entry_match:
            if current_source is None:
                raise ShaderInfoError(f"{filename}:{line_number}: entry must belong to a source")
            current_entry = {"entry": unquote_yaml_string(entry_match.group(1)), "targets": {}}
            current_source["entries"].append(current_entry)
            in_targets = False
            continue

        if re.fullmatch(r"-\s*targets\s*:", line):
            if current_source is None:
                raise ShaderInfoError(f"{filename}:{line_number}: entry must belong to a source")
            current_entry = {"targets": {}}
            current_source["entries"].append(current_entry)
            in_targets = True
            continue

        if current_entry is None:
            raise ShaderInfoError(f"{filename}:{line_number}: expected source or entry")

        profile_match = re.fullmatch(r"profile\s*:\s*(.+)", line)
        if profile_match:
            current_entry["profile"] = unquote_yaml_string(profile_match.group(1))
            in_targets = False
            continue

        if line == "targets:":
            in_targets = True
            continue

        target_match = re.fullmatch(r"([A-Za-z0-9_]+)\s*:", line)
        if in_targets and target_match:
            current_entry["targets"][target_match.group(1)] = None
            continue

        raise ShaderInfoError(f"{filename}:{line_number}: unsupported YAML syntax without PyYAML")

    return {"sources": sources}


def parse_shader_info(filename: Path):
    if yaml is None:
        document = parse_shader_info_without_yaml(filename)
    else:
        try:
            document = yaml.safe_load(filename.read_text(encoding="utf-8"))
        except yaml.YAMLError as error:
            raise ShaderInfoError(f"{filename}: invalid YAML: {error}") from error

    if not isinstance(document, dict):
        raise ShaderInfoError(f"{filename}: root YAML value must be a mapping")

    sources = document.get("sources")
    if not isinstance(sources, list) or not sources:
        raise ShaderInfoError(f"{filename}: sources must be a non-empty list")

    parsed_sources = []
    for source_info in sources:
        if not isinstance(source_info, dict):
            raise ShaderInfoError(f"{filename}: each source must be a mapping")
        source = source_info.get("source")
        entries = source_info.get("entries")
        if not isinstance(source, str) or not source:
            raise ShaderInfoError(f"{filename}: each source needs a non-empty source field")
        if not isinstance(entries, list) or not entries:
            raise ShaderInfoError(f"{filename}: each source needs a non-empty entries list")

        parsed_entries = []
        is_hlsl = Path(source).suffix.lower() == ".hlsl"
        for entry in entries:
            if not isinstance(entry, dict):
                raise ShaderInfoError(f"{filename}: each entry must be a mapping")
            targets = entry.get("targets")
            if not isinstance(targets, dict) or not targets:
                raise ShaderInfoError(f"{filename}: each entry needs a non-empty targets mapping")
            if not all(isinstance(target, str) for target in targets):
                raise ShaderInfoError(f"{filename}: target names must be strings")
            parsed_entry = {"targets": list(targets)}
            if is_hlsl:
                entry_point = entry.get("entry")
                profile = entry.get("profile")
                if not isinstance(entry_point, str) or not entry_point:
                    raise ShaderInfoError(f"{filename}: each HLSL entry needs a non-empty entry field")
                if not isinstance(profile, str) or not profile:
                    raise ShaderInfoError(f"{filename}: each HLSL entry needs a non-empty profile field")
                parsed_entry.update({"entry": entry_point, "profile": profile})
            elif not set(parsed_entry["targets"]).issubset({"spirv", "metal"}):
                raise ShaderInfoError(f"{filename}: GLSL source entries may only target 'spirv' or 'metal'")
            parsed_entries.append(parsed_entry)
        parsed_sources.append({"source": source, "entries": parsed_entries})

    return parsed_sources


def find_shader_infos(input_directory, search_depth):
    for filename in sorted(input_directory.rglob("*.shaderinfo.yml")):
        relative_parent = filename.parent.relative_to(input_directory)
        if len(relative_parent.parts) <= search_depth:
            yield filename


def format_command_argument(argument, shader_info_directory: Path) -> str:
    if isinstance(argument, Path):
        try:
            return str(argument.relative_to(shader_info_directory))
        except ValueError:
            return str(argument)
    return str(argument)


def run_command(command, verbose, shader_info_directory: Path):
    if verbose:
        print("    " + " ".join(
            format_command_argument(argument, shader_info_directory)
            for argument in command
        ))
    subprocess.run(command, check=True)


def get_stage_extension(profile):
    stage, separator, _ = profile.partition("_")
    extension = STAGE_EXTENSIONS.get(stage)
    if not separator or extension is None:
        raise ShaderInfoError(f"unsupported HLSL profile '{profile}'")
    return extension


def output_stem(source, entry, trimmed_entries, target):
    entry_suffix = "" if entry in trimmed_entries else f".{entry}"
    target_suffix = "" if target is None else f".{target}"
    return f"{source.stem}{entry_suffix}{target_suffix}"


def translate_hlsl_entry(source_file, entry, output_directory, debug: bool, verbose: bool, shader_info_directory: Path, trimmed_entries, enabled_targets):
    targets = filter_targets(entry["targets"], enabled_targets)
    if not targets:
        return
    stage_extension = get_stage_extension(entry["profile"])
    output_directory.mkdir(parents=True, exist_ok=True)
    intermediate_spv = output_directory / f"{output_stem(source_file, entry['entry'], trimmed_entries, '450core')}.{stage_extension}.spv"

    optimization_args = [] if debug else ["-O3"]

    run_command(
        ["dxc", "-nologo", "-spirv", "-T", entry["profile"], "-E", entry["entry"], "-Fo", intermediate_spv, source_file] + optimization_args,
        verbose,
        shader_info_directory,
    )

    for target in targets:
        if target == "spirv":
            if verbose:
                print(f"    wrote {format_command_argument(intermediate_spv, shader_info_directory)}")
            if debug:
                run_command(
                    ["spirv-dis", intermediate_spv, "-o", intermediate_spv.with_suffix(".spvasm")],
                    verbose,
                    shader_info_directory,
                )
            continue

        if target == "metal":
            output_file = output_directory / f"{output_stem(source_file, entry['entry'], trimmed_entries, None)}.{stage_extension}.metal"
            run_command(
                ["spirv-cross", intermediate_spv, "--msl", "--output", output_file],
                verbose,
                shader_info_directory,
            )
            continue

        if target == "dxil":
            output_file = output_directory / f"{output_stem(source_file, entry['entry'], trimmed_entries, None)}.{stage_extension}.dxil"
            debug_args = ["-Zi", "-Fd", f"{output_file}.pdb"] if debug else []
            run_command(
                ["dxc", "-nologo", "-T", entry["profile"], "-E", entry["entry"], "-Fo", output_file, source_file] + optimization_args + debug_args,
                verbose,
                shader_info_directory,
            )
            continue

        if target == "dxbc":
            if shutil.which("fxc") is None:
                print(
                    f"warning: skipping DXBC output for {source_file.name} entry "
                    f"'{entry['entry']}': 'fxc' was not found on PATH. Install it from "
                    "https://learn.microsoft.com/en-us/windows/win32/direct3dtools/fxc",
                    file=sys.stderr,
                )
                continue
            output_file = output_directory / f"{output_stem(source_file, entry['entry'], trimmed_entries, None)}.{stage_extension}.dxbc"
            debug_args = ["/Zi", "/Fd", f"{output_file}.pdb"] if debug else []
            run_command(
                ["fxc", "/nologo", "/T", entry["profile"], "/E", entry["entry"], "/Fo", output_file, source_file] + debug_args,
                verbose,
                shader_info_directory,
            )
            continue

        target_match = GLSL_TARGET_PATTERN.fullmatch(target)
        if target_match is None:
            raise ShaderInfoError(f"unsupported target '{target}'; must follow the pattern 'glsl<version>[es|core]'!")
        output_file = output_directory / f"{output_stem(source_file, entry['entry'], trimmed_entries, target_match['version'] + target_match['flavor'])}.{stage_extension}"
        command = ["spirv-cross", intermediate_spv, "--version", target_match["version"]]
        if target_match["flavor"] == "es":
            command.append("--es")
        command.extend(["--output", output_file])
        run_command(command, verbose, shader_info_directory)

    # Clean up intermediate files that are not needed in the final outut
    if "spirv" not in targets:
        intermediate_spv.unlink()
        if verbose:
            print(
                "    removed intermediate "
                f"{format_command_argument(intermediate_spv, shader_info_directory)}"
            )


def translate_glsl_source(source_file, entries, output_directory, debug: bool, verbose: bool, shader_info_directory: Path, glslang_tool: str, enabled_targets):
    targets = {
        target
        for entry in entries
        for target in filter_targets(entry["targets"], enabled_targets)
    }
    if not targets:
        return
    output_directory.mkdir(parents=True, exist_ok=True)
    output_file = output_directory / f"{source_file.name}.spv"
    run_command(
        [glslang_tool, "-V", "-o", output_file, source_file],
        verbose,
        shader_info_directory,
    )
    if "metal" in targets:
        metal_file = output_directory / f"{source_file.name}.metal"
        run_command(
            ["spirv-cross", output_file, "--msl", "--output", metal_file],
            verbose,
            shader_info_directory,
        )
    if debug:
        run_command(
            ["spirv-dis", output_file, "-o", output_file.with_suffix(".spvasm")],
            verbose,
            shader_info_directory,
        )
    if "spirv" not in targets:
        output_file.unlink()
        if verbose:
            print(f"    removed intermediate {format_command_argument(output_file, shader_info_directory)}")


def output_directory_for(shader_info: Path, output_path: Path | None) -> Path:
    return shader_info.parent / (output_path or Path("Autogen"))


def print_source_compile(source_file: Path, source_type: str, verbose: bool) -> None:
    if verbose:
        print(f'  Compiling {source_type} source "{source_file.name}"')


def print_shader_info_path(shader_info: Path, input_directory: Path, color: bool) -> None:
    relative_path = shader_info.relative_to(input_directory)
    if color:
        print(f"{HIGHLIGHT_COLOR}{relative_path}{RESET_COLOR}")
    else:
        print(relative_path)


def print_error(message: str, color: bool, indent: int = 0) -> None:
    indent_str = " " * indent
    if color:
        print(f"{indent_str}{ERROR_COLOR}error:{RESET_COLOR} {message}", file=sys.stderr)
    else:
        print(f"{indent_str}error: {message}", file=sys.stderr)


def main():
    arguments = parse_arguments()
    input_directory = arguments.input.resolve()
    if not input_directory.is_dir():
        raise RuntimeError(f"input directory does not exist: {input_directory}")

    shader_infos = list(find_shader_infos(input_directory, arguments.search_depth))
    if not shader_infos:
        print(f"No *.shaderinfo.yml files found in {input_directory}")
        return

    if arguments.verbose:
        if yaml:
            print("Parsing with PyYAML")
        else:
            print("Parsing with built-in YAML parser")

    parsed_shader_infos = []
    for shader_info in shader_infos:
        try:
            parsed_shader_infos.append((shader_info, parse_shader_info(shader_info)))
        except ShaderInfoError as error:
            print_error(f"{error}", arguments.color, indent=2)

    all_sources = [source for _, sources in parsed_shader_infos for source in sources]
    if not all_sources:
        return
    glslang_tool = verify_tools(arguments.debug, all_sources, arguments.targets)

    for shader_info, sources in parsed_shader_infos:
        try:
            print_shader_info_path(shader_info, input_directory, arguments.color)
            output_directory = output_directory_for(shader_info, arguments.output)
            for source in sources:
                source_file = shader_info.parent / source["source"]
                if not source_file.is_file():
                    raise ShaderInfoError(f"{shader_info}: source file does not exist: {source_file}")
                if not any(filter_targets(entry["targets"], arguments.targets) for entry in source["entries"]):
                    continue
                if source_file.suffix.lower() == ".hlsl":
                    print_source_compile(source_file, "HLSL", arguments.verbose)
                    for entry in source["entries"]:
                        translate_hlsl_entry(
                            source_file,
                            entry,
                            output_directory,
                            arguments.debug,
                            arguments.verbose,
                            shader_info.parent,
                            arguments.trim_stem,
                            arguments.targets,
                        )
                else:
                    print_source_compile(source_file, "GLSL", arguments.verbose)
                    translate_glsl_source(
                        source_file,
                        source["entries"],
                        output_directory,
                        arguments.debug,
                        arguments.verbose,
                        shader_info.parent,
                        glslang_tool,
                        arguments.targets,
                    )
        except (ShaderInfoError, OSError, subprocess.CalledProcessError) as error:
            print_error(f"{error}", arguments.color, indent=2)


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, ShaderInfoError, subprocess.CalledProcessError) as error:
        print_error(f"{error}", False)
        sys.exit(1)