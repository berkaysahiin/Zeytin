#!/usr/bin/env python3

import os
import re
import json
import glob
from pathlib import Path
from typing import Dict, List, Optional

class ActionParser:
    def __init__(self):
        self.action_pattern = re.compile(r'(?:struct|class)\s+(\w+)\s*:\s*(?:public\s+)?IAction')
        self.input_pattern = re.compile(r'(\w+(?:::\w+)*(?:\s*\*)?)\s+(\w+);\s*IN\(\);')
        self.output_pattern = re.compile(r'(\w+(?:::\w+)*(?:\s*\*)?)\s+(\w+);\s*OUT\(\);')

    def clean_content(self, content: str) -> str:
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
        
        lines = content.split('\n')
        cleaned_lines = []
        for line in lines:
            comment_pos = line.find('//')
            if comment_pos >= 0:
                line = line[:comment_pos]
            cleaned_lines.append(line)
        
        return '\n'.join(cleaned_lines)

    def parse_action_file(self, file_path: str) -> Optional[Dict]:
        try:
            with open(file_path, 'r') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading file {file_path}: {e}")
            return None
        
        content = self.clean_content(content)
        
        action_match = self.action_pattern.search(content)
        if not action_match:
            return None
        
        action_name = action_match.group(1)
        
        inputs = []
        for match in self.input_pattern.finditer(content):
            field_type = match.group(1).strip()
            field_name = match.group(2).strip()
            inputs.append({
                "name": field_name,
                "type": field_type
            })
        
        outputs = []
        for match in self.output_pattern.finditer(content):
            field_type = match.group(1).strip()
            field_name = match.group(2).strip()
            outputs.append({
                "name": field_name,
                "type": field_type
            })
        
        return {
            "name": action_name,
            "inputs": inputs,
            "outputs": outputs
        }

    def parse_directory(self, directory: str) -> List[Dict]:
        actions = []
        
        header_files = glob.glob(os.path.join(directory, "**/*.h"), recursive=True)
        header_files.extend(glob.glob(os.path.join(directory, "**/*.hpp"), recursive=True))
        
        for header_file in header_files:
            action_data = self.parse_action_file(header_file)
            if action_data:
                actions.append(action_data)
        
        return actions

    def generate_json(self, actions: List[Dict], output_file: Optional[str] = None) -> str:
        result = {
            "actions": actions,
            "count": len(actions)
        }
        
        json_str = json.dumps(result, indent=2)
        
        if output_file:
            try:
                dir_path = os.path.dirname(output_file)
                if dir_path:
                    os.makedirs(dir_path, exist_ok=True)
                
                with open(output_file, 'w') as f:
                    f.write(json_str)
                print(f"Actions JSON written to {output_file}")
            except Exception as e:
                print(f"Error writing to {output_file}: {e}")
        
        return json_str

def main():
    parser = ActionParser()
    
    import sys
    if len(sys.argv) > 1:
        directory = sys.argv[1]
    else:
        directory = "."
    
    if len(sys.argv) > 2:
        output_file = sys.argv[2]
    else:
        output_file = "actions.json"
    
    print(f"Parsing actions in directory: {directory}")
    
    actions = parser.parse_directory(directory)
    
    if actions:
        json_output = parser.generate_json(actions, output_file)
        print(f"Found {len(actions)} actions:")
        for action in actions:
            print(f"  - {action['name']}: {len(action['inputs'])} inputs, {len(action['outputs'])} outputs")
    else:
        print("No actions found")

if __name__ == "__main__":
    main()
