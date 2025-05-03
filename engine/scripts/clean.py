#!/usr/bin/env python3

import os
import shutil
import sys
import subprocess
import platform

def main():
    print("Cleaning build files...")
    
    current_dir = os.path.dirname(os.path.abspath(__file__))
    parent_dir = os.path.dirname(current_dir)
    
    bin_dir = os.path.join(parent_dir, "bin")
    build_dir = os.path.join(parent_dir, "build")
    
    if os.path.exists(bin_dir):
        print(f"Removing {bin_dir}")
        try:
            shutil.rmtree(bin_dir)
        except Exception as e:
            print(f"Error removing bin directory: {e}")
    
    if os.path.exists(build_dir):
        print(f"Removing {build_dir}")
        try:
            shutil.rmtree(build_dir)
        except Exception as e:
            print(f"Error removing build directory: {e}")

if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)
