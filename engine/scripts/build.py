#!/usr/bin/env python3
import os
import sys
import subprocess
import platform

def main():
    verbose = False
    config = None

    for arg in sys.argv[1:]:
        if arg == "--verbose":
            verbose = True
        elif arg.startswith("config="):
            config = arg.split("=")[1]

    script_path = os.path.abspath(__file__)
    script_dir = os.path.dirname(script_path)
    
    if os.path.basename(script_dir) == "scripts":
        engine_dir = os.path.dirname(script_dir)
    else:
        current_dir = os.getcwd()
        if os.path.exists(os.path.join(current_dir, "scripts", "parser2.py")):
            engine_dir = current_dir
            script_dir = os.path.join(current_dir, "scripts")
        else:
            print("Error: Cannot determine engine directory structure.")
            print(f"This script should be run from the engine directory or the scripts subdirectory.")
            return 1
    
    os.chdir(script_dir)
    print(f"Changed directory to: {script_dir}")
    
    print("Running parser...")
    parser_result = subprocess.run(["python3", "parser2.py"], check=False)
    if parser_result.returncode != 0:
        print(f"Parser failed with code {parser_result.returncode}")
        return parser_result.returncode

    os.chdir(engine_dir)
    print(f"Changed directory to: {engine_dir}")

    print("Running premake5...")
    premake_exe = "premake5.exe" if platform.system() == "Windows" else "premake5"
    premake_result = subprocess.run([premake_exe, "gmake"], check=False)
    if premake_result.returncode != 0:
        print(f"Premake failed with code {premake_result.returncode}")
        return premake_result.returncode


    build_dir = os.path.join(engine_dir, "build")
    os.chdir(build_dir)
    print(f"Changed directory to: {build_dir}")

    print("Running make...")
    make_args = ["make"]
    if verbose:
        make_args.append("VERBOSE=1")
    if config:
        make_args.append(f"config={config}")

    print(f"Make command: {' '.join(make_args)}")
    make_result = subprocess.run(make_args, check=False)
    return make_result.returncode

if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)
