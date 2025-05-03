#!/usr/bin/env python3

import os
import sys
import subprocess
import platform

def main():
    if platform.system() == "Windows":
        engine_path = os.path.join("..", "bin", "EDITOR_MODE", "Zeytin.exe")
    else:
        engine_path = os.path.join("..", "bin", "EDITOR_MODE", "Zeytin")
    
    if not os.path.exists(engine_path):
        print(f"Error: Engine executable not found at {engine_path}")
        return 1
    
    try:
        if platform.system() == "Windows":
            DETACHED_PROCESS = 0x00000008
            CREATE_NEW_PROCESS_GROUP = 0x00000200
            flags = DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP
            
            process = subprocess.Popen(
                [engine_path],
                creationflags=flags,
                close_fds=True,
                shell=False
            )
        else:
            process = subprocess.Popen(
                [engine_path],
                start_new_session=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                close_fds=True
            )
        
        print(f"Started engine process with PID {process.pid}")
        return 0
    except Exception as e:
        print(f"Error running engine: {e}")
        return 1

if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)
