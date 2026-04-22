#!/usr/bin/env python3
#
# __main__.py (BackendGen)
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

import os
import sys
import argparse
import shutil
from llgl_api import *
from llgl_backend import *

def query_yes_no(question, default="no"):
    valid = {"yes": True, "y": True, "ye": True, "no": False, "n": False}
    if default is None:
        prompt = " [y/n] "
    elif default == "yes":
        prompt = " [Y/n] "
    elif default == "no":
        prompt = " [y/N] "
    else:
        raise ValueError(f"Invalid default answer: '{default}'")

    while True:
        sys.stdout.write(question + prompt)
        choice = input().lower()
        if default is not None and choice == "":
            return valid[default]
        elif choice in valid:
            return valid[choice]
        else:
            sys.stdout.write("Please respond with 'yes' or 'no' (or 'y' or 'n').\n")


def fatal(msg):
    print(f"Error: {msg}")
    sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description="LLGL Backend Scaffolder")
    parser.add_argument("--name", required=True, help="Internal name (e.g., WebGPU)")
    parser.add_argument("--prefix", required=True, help="Class prefix (e.g., WebGPU)")
    parser.add_argument("--root", default=".", help="Path to LLGL root directory")
    parser.add_argument("-p", "--priority", default=100, type=int, help="Priority over other backends (default: 100)")
    parser.add_argument("-f", "--force", action="store_true", help="Automatically override existing destination folder")

    args = parser.parse_args()

    # Resolve absolute paths
    root_path = os.path.abspath(args.root)
    backend_dir = os.path.join(root_path, "sources/Renderer", args.name)
    inl_base_path = os.path.join(root_path, "include/LLGL/Backend")

    # Ensure the script is run from the correct directory
    if not os.path.exists(inl_base_path):
        fatal(f"Failed to find include directory: {inl_base_path}")

    # Check if destination exists
    if os.path.exists(backend_dir):
        if args.force:
            print(f"Force flag detected. Overriding existing folder at {backend_dir}...")
            shutil.rmtree(backend_dir)
        else:
            confirm = query_yes_no(f"Folder '{backend_dir}' already exists. Override it?", default="no")
            if confirm:
                print(f"Overriding folder...")
                shutil.rmtree(backend_dir)
            else:
                print("Operation cancelled by user.")
                sys.exit(0)

    os.makedirs(backend_dir)
    print(f"Created directory: {backend_dir}")

    for info in INTERFACES:
        print(f"Processing interface: {info.name}...")
        
        dest_dir = info.get_dest_dir(backend_dir)
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)

        # Extract signatures if an .inl exists
        signatures = []
        if info.has_inl:
            signatures = parse_interface_signatures(inl_base_path, args.prefix, info.name)
        
        # Write header and source files for interface
        write_interface_header(dest_dir, args.prefix, info, inl_base_path=inl_base_path)
        write_interface_source(dest_dir, args.prefix, info, signatures)

    # Write extended files
    print(f"Processing extended files...")
    write_extended_files(
        backend_dir=backend_dir,
        project_name=args.name,
        prefix=args.prefix,
        priority=args.priority
    )

    print(f"\nSuccess: Backend '{args.name}' scaffolded with prefix '{args.prefix}'.")


if __name__ == "__main__":
    main()
