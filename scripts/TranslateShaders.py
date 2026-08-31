#!/usr/bin/env python3

"""
TranslateShaders.py

Part of the LLGL project
Written by L. Hermanns 8/30/2026

Translate HLSL shaders described by *.shaderinfo.yml files.
"""

import argparse
import importlib
import json
import os
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
        metavar="INPUT",
        help="Search folder; equivalent to --input.",
    )
    parser.add_argument(
        "-i", "--input",
        metavar="INPUT",
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
        "-q", "--quiet",
        action="store_true",
        help="Suppress output for processed shader-info files.",
    )
    parser.add_argument(
        "-o", "--output",
        metavar="OUTPUT",
        help=(
            "Output folder relative to each shader-info file. Use '.' to write beside "
            "the input source (default: .autogen/). Absolute path is also allowed."
        ),
    )
    parser.add_argument(
        "--trim-stem",
        default="VS,PS,GS,HS,DS",
        metavar="ENTRIES",
        help="Entry names to omit from generated output stems (default: VS,PS,GS,HS,DS).",
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


def find_fxc_in_sdk_bin(sdk_bin: Path, architecture: str = "x64") -> Path | None:
    if not sdk_bin.is_dir():
        return None
    version_directories = sorted(
        (directory for directory in sdk_bin.iterdir() if directory.name.startswith("10.")),
        key=lambda directory: tuple(int(part) for part in directory.name.split(".")),
        reverse=True,
    )
    for version_directory in version_directories:
        fxc_path = version_directory / architecture / "fxc.exe"
        if fxc_path.is_file():
            return fxc_path
    return None


def find_fxc_via_vswhere(architecture: str = "x64") -> Path | None:
    program_files_x86 = Path(os.environ.get("ProgramFiles(x86)", "C:/Program Files (x86)"))
    vswhere_path = program_files_x86 / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    if not vswhere_path.is_file():
        return None

    result = subprocess.run(
        [
            vswhere_path,
            "-latest",
            "-requires", "Microsoft.VisualStudio.Component.Windows10SDK",
            "-format", "json",
        ],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0 or not result.stdout.strip():
        return None
    try:
        if not json.loads(result.stdout):
            return None
    except json.JSONDecodeError:
        return None

    return find_fxc_in_sdk_bin(program_files_x86 / "Windows Kits" / "10" / "bin", architecture)


def find_fxc_tool_path() -> Path | None:
    fxc_path = shutil.which("fxc")
    if fxc_path is not None:
        return Path(fxc_path)

    fxc_path = find_fxc_via_vswhere()
    if fxc_path is not None:
        return fxc_path

    sdk_roots = [
        Path(os.environ.get("ProgramFiles(x86)", "C:/Program Files (x86)")) / "Windows Kits" / "10" / "bin",
        Path(os.environ.get("ProgramFiles", "C:/Program Files")) / "Windows Kits" / "10" / "bin",
    ]
    for sdk_root in sdk_roots:
        fxc_path = find_fxc_in_sdk_bin(sdk_root)
        if fxc_path is not None:
            return fxc_path
    return None


def find_fxc_tool(verbose: bool) -> Path | None:
    # Skip FXC outside of Windows environment
    if sys.platform != "win32":
        if verbose:
            print("FXC is only available on Windows")
        return None

    fxc_path = find_fxc_tool_path()
    if verbose:
        if fxc_path is None:
            print("FXC was not found on PATH or in the Windows SDK")
        else:
            print(f"Found FXC: {fxc_path}")
    return fxc_path


def filter_targets(targets, enabled_targets):
    if enabled_targets is None:
        return targets
    return [target for target in targets if target in enabled_targets]


def verify_tools(debug, sources, enabled_targets, verbose: bool) -> tuple[str | None, Path | None]:
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
    fxc_tool = find_fxc_tool(verbose) if "dxbc" in requested_targets else None
    if debug:
        require_tool("spirv-dis", "https://github.com/KhronosGroup/SPIRV-Tools")
    return glslang_tool, fxc_tool


def unquote_yaml_string(value: str) -> str:
    value = value.strip()
    if len(value) >= 2 and value[0] == value[-1] and value[0] in "\"'":
        return value[1:-1]
    return value


# Built-in YAML parser with more restricted syntax.
# This serves as a fallback when PyYAML is not available.
def parse_shader_info_without_yaml(filename: Path) -> dict:
    sources = []
    current_source = None
    current_entry = None
    in_targets = False
    in_macros = False

    for line_number, raw_line in enumerate(filename.read_text(encoding="utf-8").splitlines(), 1):
        line = raw_line.split("#", 1)[0].strip()
        if not line or line == "sources:":
            continue

        source_match = re.fullmatch(r"-\s*source\s*:\s*(.+)", line)
        if source_match:
            current_source = {
                "source": unquote_yaml_string(source_match.group(1)),
                "entries": [],
            }
            sources.append(current_source)
            current_entry = None
            in_targets = False
            in_macros = False
            continue

        if line == "permutation:":
            if current_source is None:
                raise ShaderInfoError(f"{filename}:{line_number}: permutation must belong to a source")
            current_source["permutation"] = {"macros": {}}
            current_entry = None
            in_targets = False
            in_macros = False
            continue

        if line == "entries:":
            if current_source is None:
                raise ShaderInfoError(f"{filename}:{line_number}: entries must belong to a source")
            current_entry = None
            in_targets = False
            in_macros = False
            continue

        if current_source is not None and "permutation" in current_source:
            override_match = re.fullmatch(r"override\s*:\s*(.+)", line)
            if override_match:
                current_source["permutation"]["override"] = unquote_yaml_string(override_match.group(1))
                in_macros = False
                continue
            if line == "macros:":
                in_macros = True
                continue
            macro_match = re.fullmatch(r"([A-Za-z_][A-Za-z0-9_]*)\s*:\s*(.+)", line)
            if in_macros and macro_match:
                current_source["permutation"]["macros"][macro_match.group(1)] = unquote_yaml_string(macro_match.group(2))
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

        inline_targets_match = re.fullmatch(r"targets\s*:\s*(.+)", line)
        if inline_targets_match:
            target_value = inline_targets_match.group(1).strip()
            if target_value.startswith("[") and target_value.endswith("]"):
                target_names = [
                    unquote_yaml_string(target.strip())
                    for target in target_value[1:-1].split(",")
                    if target.strip()
                ]
            else:
                target_names = target_value.split()
            current_entry["targets"] = {
                target: None
                for target in target_names
            }
            in_targets = False
            continue

        target_match = re.fullmatch(r"([A-Za-z0-9_]+)\s*:", line)
        if in_targets and target_match:
            current_entry["targets"][target_match.group(1)] = None
            continue

        raise ShaderInfoError(f"{filename}:{line_number}: unsupported YAML syntax without PyYAML")

    return {"sources": sources}


def normalize_targets(filename: Path, targets) -> list[str]:
    if isinstance(targets, dict):
        target_names = list(targets)
    elif isinstance(targets, str):
        target_value = targets.strip()
        if target_value.startswith("[") and target_value.endswith("]"):
            target_names = [
                unquote_yaml_string(target.strip())
                for target in target_value[1:-1].split(",")
                if target.strip()
            ]
        else:
            target_names = target_value.split()
    elif isinstance(targets, list):
        target_names = targets
    else:
        raise ShaderInfoError(f"{filename}: each entry needs a non-empty targets mapping, list, or string")
    if not target_names or not all(isinstance(target, str) and target for target in target_names):
        raise ShaderInfoError(f"{filename}: target names must be non-empty strings")
    return target_names


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
        return None

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

        permutation = source_info.get("permutation")
        if permutation is not None:
            if not isinstance(permutation, dict):
                raise ShaderInfoError(f"{filename}: permutation must be a mapping")
            override = permutation.get("override")
            macros = permutation.get("macros")
            if not isinstance(override, str) or not override:
                raise ShaderInfoError(f"{filename}: permutation needs a non-empty override field")
            if not isinstance(macros, dict):
                raise ShaderInfoError(f"{filename}: permutation macros must be a mapping")
            if not all(isinstance(name, str) and name for name in macros):
                raise ShaderInfoError(f"{filename}: permutation macro names must be non-empty strings")
            if not all(isinstance(value, (str, int, float, bool)) for value in macros.values()):
                raise ShaderInfoError(f"{filename}: permutation macro values must be scalar values")
            permutation = {
                "override": override,
                "macros": {name: str(value) for name, value in macros.items()},
            }

        parsed_entries = []
        is_hlsl = Path(source).suffix.lower() == ".hlsl"
        for entry in entries:
            if not isinstance(entry, dict):
                raise ShaderInfoError(f"{filename}: each entry must be a mapping")
            targets = normalize_targets(filename, entry.get("targets"))
            parsed_entry = {"targets": targets}
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
        parsed_source = {"source": source, "entries": parsed_entries}
        if permutation is not None:
            parsed_source["permutation"] = permutation
        parsed_sources.append(parsed_source)

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


def clamp_dxc_profile(profile: str) -> str:
    match = re.fullmatch(r"([a-z]+)_(\d+)_(\d+)", profile)
    if match is None:
        raise ShaderInfoError(f"unsupported HLSL profile '{profile}'")
    stage, major_version, minor_version = match.groups()
    if (int(major_version), int(minor_version)) < (6, 0):
        return f"{stage}_6_0"
    return profile


def output_stem(source, entry, trimmed_entries, target = None, stage = None, override = None):
    override_suffix = f".{override}" if override else ""
    if target and stage:
        entry_suffix = "" if entry in trimmed_entries else f".{entry}"
        return f"{source.stem}{override_suffix}{entry_suffix}.{target}.{stage}"
    else:
        return f"{source.stem}{override_suffix}.{entry}"


def sanitize_glsl_output(output_file: Path):
    with open(output_file, "r", encoding="utf-8") as file:
        content = file.read()

    is_vertex_shader = output_file.suffix == ".vert"
    is_fragment_shader = output_file.suffix == ".frag"

    # Remove all 'in_var_' prefixes from vertex shader inputs
    if is_vertex_shader:
        content = content.replace("in_var_", "")
        content = content.replace("out_var_", "v_")
    elif is_fragment_shader:
        content = content.replace("in_var_", "v_")
        content = content.replace("out_var_", "")

    # Rename SPIRV-Cross UBO types and remove their instance aliases.
    for uniform_match in re.finditer(
        r"uniform\s+type_(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*\{.*?\}\s*(?P=name)\s*;",
        content,
        re.DOTALL,
    ):
        uniform_name = uniform_match.group("name")
        sanitized_uniform = re.sub(
            rf"\s*\}}\s*{re.escape(uniform_name)}\s*;$",
            "\n};",
            uniform_match.group(0),
        ).replace(f"type_{uniform_name}", uniform_name, 1)
        content = content.replace(uniform_match.group(0), sanitized_uniform)
        content = content.replace(f"{uniform_name}.", "")

    # Write back into the same output file
    with open(output_file, "w", encoding="utf-8") as file:
        file.write(content)


def compile_spirv(source, output, profile, extra_args, shader_info_directory: Path, verbose: bool, in_entry: str, out_entry: str = None):
    dxc_args = [
        "dxc",
        "-nologo",
        "-no-warnings",
        "-spirv",
        "-fspv-reflect",
        "-fvk-auto-shift-bindings",
        "-T", clamp_dxc_profile(profile),
        "-E", in_entry,
        "-Fo", output,
        source
    ] + extra_args

    if out_entry:
        dxc_args.append(f"-fspv-entrypoint-name={out_entry}")

    run_command(dxc_args, verbose, shader_info_directory)


def translate_hlsl_entry(source_file, entry, output_directory, debug: bool, verbose: bool, shader_info_directory: Path, trimmed_entries, enabled_targets, fxc_tool: Path | None, permutation):
    targets = filter_targets(entry["targets"], enabled_targets)
    if not targets:
        return
    stage_extension = get_stage_extension(entry["profile"])
    output_directory.mkdir(parents=True, exist_ok=True)
    override = permutation["override"] if permutation else None
    macros = permutation["macros"] if permutation else {}
    dxc_macro_args = [f"-D{name}={value}" for name, value in macros.items()]
    fxc_macro_args = [f"/D{name}={value}" for name, value in macros.items()]
    intermediate_spv = output_directory / f"{output_stem(source_file, entry['entry'], trimmed_entries, '450core', stage_extension, override)}.temp.spv"

    optimization_args = [] if debug else ["-O3"]

    # Compile HLSL to SPIR-V using DXC
    compile_spirv(
        source = source_file,
        output = intermediate_spv,
        profile = entry["profile"],
        extra_args = optimization_args + dxc_macro_args,
        shader_info_directory = shader_info_directory,
        verbose = verbose,
        in_entry = entry["entry"]
    )

    for target in targets:
        if target == "spirv":
            # Compile to SPIR-V and set entry point to "main()" as LLGL's examples don't use custom entry points
            output_file = output_directory / f"{output_stem(source_file, entry['entry'], trimmed_entries, '450core', stage_extension, override)}.spv"
            compile_spirv(
                source = source_file,
                output = output_file,
                profile = entry["profile"],
                extra_args = optimization_args + dxc_macro_args,
                shader_info_directory = shader_info_directory,
                verbose = verbose,
                in_entry = entry["entry"],
                out_entry = "main"
            )

            # Produce SPIR-V disassembly as debug output
            if debug:
                run_command(
                    [
                        "spirv-dis",
                        output_file,
                        "-o", output_file.with_suffix(".spvasm")
                    ],
                    verbose,
                    shader_info_directory,
                )
            continue

        if target == "metal":
            # Compile to Metal using SPIRV-Cross
            output_file = output_directory / f"{output_stem(source_file, entry['entry'], trimmed_entries, None, stage_extension, override)}.metal"
            run_command(
                [
                    "spirv-cross",
                    intermediate_spv,
                    "--msl",
                    "--msl-decoration-binding", # LLGL examples maintain the same binding locations for all languages
                    "--output", output_file
                ],
                verbose,
                shader_info_directory,
            )
            continue

        if target == "dxil":
            # Compile to DXIL bytecode using DXC
            output_file = output_directory / f"{output_stem(source_file, entry['entry'], trimmed_entries, None, stage_extension, override)}.dxil"
            debug_args = ["-Zi", "-Fd", f"{output_file}.pdb"] if debug else []
            run_command(
                [
                    "dxc",
                    "-nologo",
                    "-no-warnings",
                    "-T", clamp_dxc_profile(entry["profile"]),
                    "-E", entry["entry"],
                    "-Fo", output_file,
                    source_file
                ] + optimization_args + debug_args + dxc_macro_args,
                verbose,
                shader_info_directory,
            )
            continue

        if target == "dxbc":
            # Compile to DXBC bytecode using FXC
            if fxc_tool is None:
                print(
                    f"warning: skipping DXBC output for {source_file.name} entry "
                    f"'{entry['entry']}': 'fxc' was not found on PATH or in the Windows SDK. Install it from "
                    "https://learn.microsoft.com/en-us/windows/win32/direct3dtools/fxc",
                    file=sys.stderr,
                )
                continue
            output_file = output_directory / f"{output_stem(source_file, entry['entry'], trimmed_entries, None, stage_extension, override)}.dxbc"
            debug_args = ["/Zi", "/Fd", f"{output_file}.pdb"] if debug else []
            run_command(
                [fxc_tool, "/nologo", "/T", entry["profile"], "/E", entry["entry"], "/Fo", output_file, source_file] + debug_args + fxc_macro_args,
                verbose,
                shader_info_directory,
            )
            continue

        # Compile to GLSL using SPIRV-Cross
        target_match = GLSL_TARGET_PATTERN.fullmatch(target)
        if target_match is None:
            raise ShaderInfoError(f"unsupported target '{target}'; must follow the pattern 'glsl<version>[es|core]'!")
        output_file = output_directory / f"{output_stem(source_file, entry['entry'], trimmed_entries, target_match['version'] + target_match['flavor'], stage_extension, override)}"

        command = [
            "spirv-cross",
            "--no-420pack-extension",
            "--combined-samplers-inherit-bindings",
            intermediate_spv,
            "--version", target_match["version"]
        ]
        if target_match["flavor"] == "es":
            command.append("--es")
        command.extend(["--output", output_file])
        run_command(command, verbose, shader_info_directory)

        # Patch GLSL output to make it work with the LLGL example projects
        sanitize_glsl_output(output_file)

    # Clean up intermediate files that are not needed in the final outut
    intermediate_spv.unlink()
    if verbose:
        print(f"    removed intermediate {format_command_argument(intermediate_spv, shader_info_directory)}")


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
    if output_path and output_path.is_absolute():
        return output_path
    else:
        return shader_info.parent / (output_path or Path(".autogen"))


def print_source_compile(source_file: Path, source_type: str, verbose: bool) -> None:
    if verbose:
        print(f'  Compiling {source_type} source "{source_file.name}"')


def print_shader_info_path(shader_info: Path, input_directory: Path, color: bool) -> None:
    relative_path = shader_info.relative_to(input_directory)
    if color:
        print(f"Processing: {HIGHLIGHT_COLOR}{relative_path}{RESET_COLOR}")
    else:
        print(f"Processing: {relative_path}")


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
            shader_info_source = parse_shader_info(shader_info)
            if shader_info_source:
                parsed_shader_infos.append((shader_info, shader_info_source))
        except ShaderInfoError as error:
            print_error(f"{error}", arguments.color, indent=2)

    all_sources = [source for _, sources in parsed_shader_infos for source in sources]
    if not all_sources:
        return
    glslang_tool, fxc_tool = verify_tools(
        arguments.debug,
        all_sources,
        arguments.targets,
        arguments.verbose,
    )

    for shader_info, sources in parsed_shader_infos:
        try:
            if not arguments.quiet:
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
                            fxc_tool,
                            source.get("permutation"),
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