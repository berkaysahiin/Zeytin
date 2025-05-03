#!/usr/bin/env python3

import os
import sys
import subprocess
import platform
import signal
import time

def main():
    print("Looking for engine processes to terminate...")
    
    search_pattern = "./bin/EDITOR_MODE/Zeytin"
    
    try:
        if platform.system() == "Windows":
            output = subprocess.check_output(
                ["tasklist", "/FI", f"IMAGENAME eq Zeytin.exe", "/FO", "CSV"],
                universal_newlines=True
            )
            
            if "Zeytin.exe" in output:
                print("Terminating Zeytin processes...")
                subprocess.run(["taskkill", "/F", "/IM", "Zeytin.exe"], check=False)
                print("Engine processes terminated.")
            else:
                print("No engine processes found.")
        else:
            try:
                output = subprocess.check_output(
                    ["pgrep", "-f", search_pattern],
                    universal_newlines=True
                )
                
                pids = output.strip().split('\n')
                if pids and pids[0]:
                    print(f"Found {len(pids)} engine processes. Terminating...")
                    subprocess.run(["pkill", "-f", search_pattern], check=False)
                    print("Engine processes terminated.")
                else:
                    print("No engine processes found.")
            except subprocess.CalledProcessError:
                print("No engine processes found.")
    
    except Exception as e:
        print(f"Error while trying to terminate engine processes: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)
