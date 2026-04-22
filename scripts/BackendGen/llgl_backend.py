#
# llgl_backend.py
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

import os
import re
from llgl_api import *
from llgl_template import *


def get_ordered_inl_files(inl_base_path, interface_name):
    """
    Reads the master INTERFACE.inl file to determine the order of sub-inl files.
    If no master file exists or it doesn't contain includes, returns all 
    matching files sorted alphabetically as a fallback.
    """
    master_file = f"{interface_name}.inl"
    master_path = os.path.join(inl_base_path, master_file)
    
    # Start with all files that match the pattern
    all_matching = [
        f for f in os.listdir(inl_base_path)
        if f.startswith(interface_name) and f[len(interface_name)] == '.' and f.endswith('.inl')
    ]

    if not os.path.exists(master_path):
        return sorted(all_matching)

    with open(master_path, 'r') as f:
        content = f.read()

    # Find all lines like: #include <LLGL/Backend/CommandBuffer.Drawing.inl>
    # We capture the filename part at the end
    include_pattern = re.compile(r'#include\s+<LLGL/Backend/(?P<file>.+?\.inl)>')
    included_files = include_pattern.findall(content)

    # Filter out files that don't actually exist on disk (safety check)
    ordered_files = [f for f in included_files if f in all_matching]

    # If the master file was just a list of functions and didn't include other files,
    # or we missed some files that weren't included in the master, 
    # append the remaining files alphabetically.
    seen = set(ordered_files)
    if master_file not in seen:
        # Usually the master file itself contains the primary interface signatures
        ordered_files.insert(0, master_file)
        seen.add(master_file)

    for f in sorted(all_matching):
        if f not in seen:
            ordered_files.append(f)

    return ordered_files



def parse_interface_signatures(inl_base_path, prefix, interface_name) -> list[LLGLFunctionSignature]:
    """
    Finds all .inl files starting with the interface name (e.g., CommandBuffer*.inl)
    and extracts function signatures from all of them.
    """
    if not os.path.exists(inl_base_path):
        return []

    # Get files in the specific order defined by the master .inl includes
    ordered_files = get_ordered_inl_files(inl_base_path, interface_name)

    signatures = []
    # 1. Matches 'virtual'
    # 2. Captures return type (handling spaces, pointers, and refs)
    # 3. Captures function name
    # 4. Captures arguments (including multi-line and default values)
    # 5. Handles 'const' qualifier
    # 6. Handles 'override', 'final', and '= 0' in any combination
    pattern = re.compile(
        r"virtual\s+(?P<ret>[\w\*\&\<\>\s:]+?)\s+(?P<name>\w+)\s*\((?P<args>.*?)\)\s*(?P<const>const)?\s*override\s*(?:final)?\s*(?:\s*=\s*0)?\s*;",
        re.MULTILINE | re.DOTALL
    )

    def sanitize_scopes(expr):
        return expr.replace('LLGL::', '')

    for filename in ordered_files:
        inl_path = os.path.join(inl_base_path, filename)
        with open(inl_path, 'r') as f:
            content = f.read()

        for match in pattern.finditer(content):
            ret_type = match.group(1).strip()
            func_name = match.group(2).strip()

            # Clean up multi-line arguments and extra whitespace
            args = re.sub(r'\s+', ' ', match.group(3).strip())

            # Remove default argument expressions (e.g., " = nullptr", " = 0", " = {}")
            # This regex looks for an '=' followed by anything that isn't a comma or closing paren
            # It also handles potential spaces around the '='
            args = re.sub(r'\s*=\s*[^,)]+', '', args)

            is_const = match.group(4)
            sig = LLGLFunctionSignature(sanitize_scopes(ret_type), f'{prefix}{interface_name}', func_name, sanitize_scopes(args), is_const)

            signatures.append(sig)

    return signatures


def write_file(dest_path: str, content: str):
    with open(dest_path, 'w', encoding='utf-8') as f:
        f.write(content)


def to_header_guard(name: str) -> str:
    """
    Converts a camel-case string into a C/C++ header guard format.
    Example: 'RenderSystem' -> 'RENDER_SYSTEM'
    """
    # Use regex to find lowercase followed by uppercase
    # \1 refers to the first group (lowercase), \2 to the second (uppercase)
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    
    # Handle cases like 'WGPUBuffer' where multiple uppercase letters exist
    # This ensures 'WGPU' and 'Buffer' are separated: 'WGPU_BUFFER'
    return re.sub('([a-z0-0])([A-Z])', r'\1_\2', s1).upper()


def to_include_directives(include_files: list[str], prefix: str = None) -> str:
    include_directives = ""
    for inc_file in include_files:
        if len(include_directives) > 0:
            include_directives += "\n"
        if len(inc_file) > 0:
            include_directives += f"#include {inc_file.format(prefix=prefix) if prefix is not None else inc_file}"
    return include_directives


def does_inl_header_exist(inl_base_path: str, interface_name: str) -> bool:
    return os.path.exists(os.path.join(inl_base_path, f'{interface_name}.inl'))


def write_interface_header(dest_path: str, prefix: str, interface: LLGLInterfaceInfo, inl_base_path: str):
    base_file = os.path.join(dest_path, f"{prefix}{interface.name}")
    forward_decls = interface.forward_decls.format(prefix=prefix) if interface.forward_decls is not None else ""
    header_template = HEADER_TEMPLATE_INTERFACE if does_inl_header_exist(inl_base_path, interface.name) else HEADER_TEMPLATE_INTERFACE_MINIMAL
    ctor_decl = CTOR_SIGNATURE_TEMPLATE.format(prefix=prefix, interface=interface.name)
    write_file(
        dest_path=f"{base_file}.h",
        content=HEADER_TEMPLATE.format(
            file=os.path.basename(base_file),
            guard=f"LLGL_{prefix.upper()}_{to_header_guard(interface.name)}_H",
            include_directives=to_include_directives([f"<LLGL/{interface.name}.h>"] + interface.additional_include_files, prefix=prefix),
            content=forward_decls + header_template.format(
                prefix=prefix,
                interface=interface.name,
                ctor=(f"{ctor_decl};" if interface.has_default_ctor else f"/* {ctor_decl}; */"),
                private_fields=(interface.private_decls.format(prefix=prefix) if interface.private_decls is not None else "// private fields ...")
            )
        )
    )


def write_common_header(dest_path: str, prefix: str, name: str, content: str, include_files: list[str] = ['<LLGL/ForwardDecls.h>']):
    base_file = os.path.join(dest_path, f"{prefix}{name}")
    write_file(
        dest_path=f"{base_file}.h",
        content=HEADER_TEMPLATE.format(
            file=os.path.basename(base_file),
            guard=f"LLGL_{prefix.upper()}_{to_header_guard(name)}_H",
            include_directives=to_include_directives(include_files),
            content=content
        )
    )


def write_interface_source(dest_path: str, prefix: str, interface: LLGLInterfaceInfo, signatures: list[LLGLFunctionSignature]):
    base_file = os.path.join(dest_path, f"{prefix}{interface.name}")

    ctor_decl = CTOR_SIGNATURE_IMPL_TEMPLATE.format(prefix=prefix, interface=interface.name)

    functions_code = ""
    for sig in signatures:
        functions_code += "\n\n"
        functions_code += f"{sig}\n"
        functions_code += "{\n"
        if sig.has_dummy_body():
            ret_stmt = sig.print_default_return()
            if ret_stmt:
                functions_code += f"    return {ret_stmt}; // dummy\n"
            else:
                functions_code += "    // dummy\n"
        else:
            functions_code += "    LLGL_TRAP_NOT_IMPLEMENTED();\n"
        functions_code += "}"

    if interface.private_defs is not None:
        if len(functions_code) > 0:
            functions_code += "\n\n"
        functions_code += SOURCE_TEMPLATE_PRIVATE
        functions_code += interface.private_defs.format(prefix=prefix)

    write_file(
        dest_path=f"{base_file}.cpp",
        content=SOURCE_TEMPLATE.format(
            file=os.path.basename(base_file),
            include_directives=interface.print_assert_include_directive(),
            content=SOURCE_TEMPLATE_INTERFACE.format(
                prefix=prefix,
                interface=interface.name,
                ctor=(f"{ctor_decl} {{ /* ... */ }}" if interface.has_default_ctor else f"/* {ctor_decl} {{ ... }} */"),
                functions=functions_code
            )
        )
    )


def write_common_source(dest_path: str, prefix: str, name: str, content: str):
    base_file = os.path.join(dest_path, f"{prefix}{name}")

    write_file(
        dest_path=f"{base_file}.cpp",
        content=SOURCE_TEMPLATE.format(
            file=os.path.basename(base_file),
            include_directives='',
            content=content
        )
    )



def write_extended_files(backend_dir: str, project_name: str, prefix: str, priority: int = 100):
    # Write CMakeLists.txt file
    write_file(
        dest_path=os.path.join(backend_dir, "CMakeLists.txt"),
        content=CMAKELISTS_TEMPLATE.format(
            project=project_name,
            project_ucase=project_name.upper()
        )
    )

    # Write module interface file
    write_file(
        dest_path=os.path.join(backend_dir, f"{prefix}ModuleInterface.cpp"),
        content=MODULE_INTERFACE_SOURCE_TEMPLATE.format(
            prefix=prefix,
            project=project_name,
            priority=str(priority)
        )
    )

    # Write extended files for Command/ folder
    command_path=os.path.join(backend_dir, "Command")
    write_common_header(
        dest_path=command_path,
        prefix=prefix,
        name="Command",
        content=HEADER_TEMPLATE_COMMAND.format(prefix=prefix),
        include_files=['<cstdint>']
    )
    write_common_header(
        dest_path=command_path,
        prefix=prefix,
        name="CommandOpcode",
        content=HEADER_TEMPLATE_COMMAND_OPCODE.format(prefix=prefix),
        include_files=['<cstdint>']
    )
    write_common_header(
        dest_path=command_path,
        prefix=prefix,
        name="CommandExecutor",
        content=HEADER_TEMPLATE_COMMAND_EXECUTOR.format(prefix=prefix),
        include_files=[
            f'"{prefix}CommandBuffer.h"',
            f'"{prefix}Command.h"'
        ]
    )
    write_common_source(
        dest_path=command_path,
        prefix=prefix,
        name="CommandExecutor",
        content=SOURCE_TEMPLATE_COMMAND_EXECUTOR.format(prefix=prefix)
    )
