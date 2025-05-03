#!/usr/bin/env python3

import os
import sys
import json
import time
import subprocess
import re
from pathlib import Path

def find_project_root():
    current_path = Path.cwd()
    
    for _ in range(10):
       
        if current_path.name == "scripts" and (current_path.parent.name == "engine"):
            return current_path.parent.parent
        

        if (current_path / "engine").exists() and (current_path / "editor").exists():
            return current_path
            

        parent = current_path.parent
        if parent == current_path:  
            break
        current_path = parent
    
    print("ERROR: Could not find project root directory")
    return None

def get_error_summary(log_path):
    if not os.path.exists(log_path):
        return "Build log not found"
    
    with open(log_path, 'r', encoding='utf-8', errors='replace') as file:
        content = file.read()
    
    error_pattern = re.compile(r'(?:error:|failed|Error |make: \*\*\*)')
    matches = list(error_pattern.finditer(content))
    
    if matches:
        lines = []
        for match in matches[:5]:
            pos = match.start()
            line_start = content.rfind('\n', 0, pos) + 1
            line_end = content.find('\n', pos)
            if line_end == -1:
                line_end = len(content)
            lines.append(content[line_start:line_end].strip())
        return '\n'.join(lines)
    else:
        lines = content.splitlines()
        return '\n'.join(lines[-10:])

def write_status_file(output_file, status, message, details=None):
    data = {
        "status": status,
        "message": message,
        "timestamp": str(int(time.time()))
    }
    
    if details:
        data["details"] = details
    
    os.makedirs(os.path.dirname(output_file), exist_ok=True)
    with open(output_file, 'w') as f:
        json.dump(data, f)

def run_build(engine_scripts_path, build_log):
    os.chdir(engine_scripts_path)
    
    if os.name != 'nt': 
        parser_path = os.path.join(engine_scripts_path, "parser2.py")
        if os.path.exists(parser_path):
            os.chmod(parser_path, 0o755)
        
        build_py_path = os.path.join(engine_scripts_path, "build.py")
        if os.path.exists(build_py_path):
            os.chmod(build_py_path, 0o755)
    
    if os.path.exists(os.path.join(engine_scripts_path, "build.py")):
        build_command = "python3 build.py"
    else:
        build_command = "python3 parser2.py && cd .. && premake5 gmake && cd build && make"
    
    try:
        with open(build_log, 'w') as log_file:
            process = subprocess.Popen(
                build_command, 
                shell=True,
                stdout=subprocess.PIPE, 
                stderr=subprocess.STDOUT, 
                universal_newlines=True,
                bufsize=1  
            )
            
            for line in process.stdout:
                print(f"BUILD: {line.strip()}")
                log_file.write(line)
                log_file.flush()
            
            process.communicate()  
            return process.returncode
    except Exception as e:
        print(f"BUILD: Error executing build: {e}")
        with open(build_log, 'a') as log_file:
            log_file.write(f"\nBUILD ERROR: {e}\n")
        return 1

def main():
    project_root = find_project_root()
    if not project_root:
        sys.exit(1)
        
    engine_dir = project_root / "engine"
    engine_scripts_dir = engine_dir / "scripts"
    build_status_dir = engine_dir / "build_status"
    
    if not engine_scripts_dir.exists():
        print(f"ERROR: Engine scripts directory not found at {engine_scripts_dir}")
        sys.exit(1)
    
    output_file = build_status_dir / "build_status.json"
    build_log = build_status_dir / "build_output.log"
    
    os.makedirs(build_status_dir, exist_ok=True)
    
    print("BUILD: Starting build process...")
    write_status_file(output_file, "running", "Build started")
    
    build_exit_code = run_build(engine_scripts_dir, build_log)
    
    if build_exit_code == 0:
        print("BUILD: Completed successfully")
        write_status_file(output_file, "success", "Build completed successfully")
        return 0
    else:
        print(f"BUILD: Failed with exit code {build_exit_code}")
        error_summary = get_error_summary(build_log)
        error_json = error_summary.replace('\n', '\\n')
        
        write_status_file(
            output_file, 
            "failed",
            f"Build failed with exit code {build_exit_code}",
            error_json
        )
        
        print("BUILD ERROR SUMMARY:")
        print(error_summary)
        
        return build_exit_code

if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)
