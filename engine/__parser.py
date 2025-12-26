#!/usr/bin/env python3

import os
import re
import glob
import hashlib
import sys
from typing import Dict, List, Optional, Any
from collections import defaultdict

class ClassParser:
    def __init__(self):
        self.variant_pattern = re.compile(r'VARIANT\((\w+)\)')
        self.class_pattern = re.compile(r'(struct|class)\s+(\w+)\s*(?::\s*public\s+(\w+))?')
        self.property_pattern = re.compile(r'(\w+(?:::\w+)*(?:\s*\*)?)\s+(\w+)(?:\s*=\s*[^;]*)?;\s*PROPERTY\(\)(?:\s+SET_CALLBACK\((\w+)\))?')
        self.skip_classes = ["VariantCreateInfo", "VariantBase"]

    def clean_content(self, content: str) -> str:
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
        
        lines = content.split('\n')
        cleaned_lines = []
        for line in lines:
            comment_pos = line.find('//')
            if comment_pos >= 0:
                line = line[:comment_pos]
            cleaned_lines.append(line)
        
        content = '\n'.join(cleaned_lines)
        
        content = re.sub(r'enum\s+class\s+\w+\s*{[^}]*}', '', content, flags=re.DOTALL)
        
        return content

    def find_class_block(self, content: str, class_match: re.Match) -> str:
        class_start = class_match.start()
        class_block = content[class_start:]
        
        open_braces = 0
        close_pos = 0
        
        for i, char in enumerate(class_block):
            if char == '{':
                open_braces += 1
            elif char == '}':
                open_braces -= 1
                if open_braces == 0:
                    close_pos = i + 1
                    break
        
        if close_pos > 0:
            return class_block[:close_pos]
        return class_block

    def parse_variant_class(self, class_block: str, class_name: str, base_class: str) -> Optional[Dict[str, Any]]:
        if class_name in self.skip_classes:
            return None
        
        properties = []
        property_matches = self.property_pattern.finditer(class_block)
        for match in property_matches:
            prop_type = match.group(1).strip()
            prop_name = match.group(2).strip()
            callback_name = match.group(3)
            properties.append((prop_type, prop_name, callback_name))
        
        base_class = base_class if base_class else "VariantBase"
        
        return {
            'class_name': class_name,
            'base_class': base_class,
            'properties': properties,
            'is_variant': True
        }

    def parse_header(self, file_path: str) -> List[Dict[str, Any]]:
        try:
            with open(file_path, 'r') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading file {file_path}: {e}")
            return []
        
        content = self.clean_content(content)
        class_matches = list(self.class_pattern.finditer(content))
        variant_matches = list(self.variant_pattern.finditer(content))
        
        variant_classes = {match.group(1): True for match in variant_matches}
        results = []
        
        for class_match in class_matches:
            class_name = class_match.group(2)
            base_class = class_match.group(3)
            
            if '<' in class_name or '>' in class_name:
                continue
            
            if class_name not in variant_classes:
                continue
            
            class_block = self.find_class_block(content, class_match)
            variant_info = self.parse_variant_class(class_block, class_name, base_class)
            
            if variant_info:
                results.append(variant_info)
        
        return results


class CodeGenerator:
    @staticmethod
    def generate_base_classes_registration() -> str:
        return """RTTR_REGISTRATION
{
    rttr::registration::class_<VariantCreateInfo>("VariantCreateInfo")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("entity_id", &VariantCreateInfo::entity_id);
    
    rttr::registration::class_<VariantBase>("VariantBase")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("entity_id", &VariantBase::entity_id)(rttr::metadata("NO_SERIALIZE", true));
}
"""

    @staticmethod
    def generate_variant_class_registration(class_info: Dict[str, Any]) -> str:
        class_name = class_info["class_name"]
        
        code = f'RTTR_REGISTRATION\n{{\n'
        code += f'    rttr::registration::class_<{class_name}>("{class_name}")\n'
        code += '        .constructor<>()(rttr::policy::ctor::as_object)\n'
        code += '        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)'
        
        for prop_type, prop_name, callback_name in sorted(class_info['properties'], key=lambda x: x[1]):
            code += f'\n        .property("{prop_name}", &{class_name}::{prop_name})'
            if callback_name:
                code += f'(rttr::metadata("SET_CALLBACK", "{callback_name}"))'
        
        has_callbacks = any(callback_name for _, _, callback_name in class_info['properties'] if callback_name)
        if has_callbacks:
            code += '\n'
            for prop_type, prop_name, callback_name in sorted(class_info['properties'], key=lambda x: x[1]):
                if callback_name:
                    code += f'\n        .method("{callback_name}", &{class_name}::{callback_name})'
        
        code += ';\n}\n'
        return code

    @staticmethod
    def generate_file_registration(include_path: str, classes_info: List[Dict[str, Any]]) -> str:
        code = '#include "rttr/registration.h"\n'
        code += f'#include "{include_path}"\n\n'
        
        for class_info in sorted(classes_info, key=lambda x: x['class_name']):
            code += CodeGenerator.generate_variant_class_registration(class_info)
            code += '\n'
        
        return code


class ProjectFinder:
    @staticmethod
    def find_root_folders():
        current_dir = os.path.abspath(os.path.curdir)
        engine_dir = None
        project_root = None
        
        max_depth = 5
        current_depth = 0
        search_path = current_dir
        
        while current_depth < max_depth:
            if os.path.isdir(os.path.join(search_path, "engine")) and os.path.isdir(os.path.join(search_path, "editor")):
                project_root = search_path
                engine_dir = os.path.join(search_path, "engine")
                break
            
            if os.path.isdir(os.path.join(search_path, "include")) and os.path.isdir(os.path.join(search_path, "source")):
                if "engine" in search_path.lower():
                    engine_dir = search_path
            
            parent_path = os.path.dirname(search_path)
            if parent_path == search_path:
                break
            
            search_path = parent_path
            current_depth += 1
        
        if engine_dir and not project_root:
            potential_project_root = os.path.dirname(engine_dir)
            if os.path.isdir(os.path.join(potential_project_root, "editor")):
                project_root = potential_project_root
        
        return {"engine_dir": engine_dir, "project_root": project_root}


class RTTRGenerator:
    def __init__(self):
        paths = ProjectFinder.find_root_folders()
        
        self.engine_dir = paths.get("engine_dir")
        if not self.engine_dir:
            print("Error: Could not locate the engine directory")
            print("Please run this script from within the project directory structure")
            sys.exit(1)
            
        self.headers_dir = os.path.join(self.engine_dir, "include")
        self.game_headers_dir = os.path.join(self.headers_dir, "game")
        self.game_source_dir = os.path.join(self.engine_dir, "source", "game")

        print(f"Found engine directory: {self.engine_dir}")
        print(f"Using headers directory: {self.headers_dir}")
        
        self.parser = ClassParser()
        self.header_classes = defaultdict(list)  # Maps header path to list of classes

    def process_headers(self) -> None:
        header_files = glob.glob(os.path.join(self.game_headers_dir, "**/*.h"), recursive=True)
        
        print(f"Processing {len(header_files)} header files...")
        
        for header_file in header_files:
            class_infos = self.parser.parse_header(header_file)
            
            if class_infos:
                self.header_classes[header_file] = class_infos

    def write_file_if_changed(self, file_path: str, content: str) -> bool:
        """Write file only if content has changed. Returns True if file was written."""
        if os.path.exists(file_path):
            try:
                with open(file_path, 'r') as f:
                    existing_content = f.read()
                    if hashlib.md5(existing_content.encode()).hexdigest() == hashlib.md5(content.encode()).hexdigest():
                        return False
            except Exception:
                pass
        
        try:
            os.makedirs(os.path.dirname(file_path), exist_ok=True)
            with open(file_path, "w") as f:
                f.write(content)
            return True
        except Exception as e:
            print(f"Error writing file {file_path}: {e}")
            return False

    def generate_registration_files(self) -> None:
        total_classes = 0
        files_written = 0
        files_unchanged = 0
        
        for header_path, classes_info in self.header_classes.items():
            # Calculate relative path from game_headers_dir to maintain directory structure
            relative_path = os.path.relpath(header_path, self.game_headers_dir)
            header_basename = os.path.splitext(os.path.basename(header_path))[0]
            relative_dir = os.path.dirname(relative_path)
            
            # Build path in source/game/generated maintaining directory structure
            output_dir = os.path.join(self.game_source_dir, "generated", relative_dir)
            register_path = os.path.join(output_dir, f"{header_basename}.register.cpp")
            
            # Get relative path from headers_dir for the include
            include_relative_path = os.path.relpath(header_path, self.headers_dir).replace('\\', '/')
            
            registration_code = CodeGenerator.generate_file_registration(include_relative_path, classes_info)
            
            if self.write_file_if_changed(register_path, registration_code):
                print(f"Generated: {register_path}")
                files_written += 1
            else:
                files_unchanged += 1
            
            total_classes += len(classes_info)
        
        print(f"\nGenerated {files_written} registration files ({files_unchanged} unchanged)")
        print(f"Total variant classes registered: {total_classes}")

    def generate_base_registration(self) -> None:
        output_path = os.path.join(self.game_source_dir, "generated/rttr_base_registration.cpp")
        base_registration = '#include "rttr/registration.h"\n'
        base_registration += '#include "variant/variant_base.h"\n\n'
        base_registration += CodeGenerator.generate_base_classes_registration()
        
        if self.write_file_if_changed(output_path, base_registration):
            print(f"Generated base registration: {output_path}")
        else:
            print(f"Base registration unchanged: {output_path}")

    def run(self) -> None:
        self.process_headers()
        self.generate_base_registration()
        self.generate_registration_files()


def main():
    generator = RTTRGenerator()
    generator.run()

if __name__ == "__main__":
    main()
