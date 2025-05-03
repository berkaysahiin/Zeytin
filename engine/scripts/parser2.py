#!/usr/bin/env python3

import os
import re
import glob
import json
import sys
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Set, Any, Union

class ClassParser:
    def __init__(self):
        self.variant_pattern = re.compile(r'VARIANT\((\w+)\)')
        self.class_pattern = re.compile(r'(struct|class)\s+(\w+)\s*(?::\s*public\s+(\w+))?')
        self.requires_pattern = re.compile(r'REQUIRES\s*\(\s*(.*?)\s*\)')
        self.ignore_queries_pattern = re.compile(r'IGNORE_QUERIES\s*\(\s*\)')

        self.property_pattern = re.compile(r'(\w+(?:::\w+)*(?:\s*\*)?)\s+(\w+)(?:\s*=\s*[^;]*)?;\s*PROPERTY\(\)(?:\s+SET_CALLBACK\((\w+)\))?')
        
        self.regular_property_pattern = re.compile(r'^\s*(?:public|private|protected)?:?\s*(\w+(?:::\w+)*(?:\s*\*)?)\s+(\w+)\s*;')
        
        self.query_get_pattern = re.compile(r'Query::get<([\w,\s]+)>\(this\)')
        self.query_read_pattern = re.compile(r'Query::read<([\w,\s]+)>\(this\)')
        self.query_try_get_pattern = re.compile(r'Query::try_get<([\w,\s]+)>\(this\)')
        
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
        
        ignore_queries = False
        ignore_queries_match = self.ignore_queries_pattern.search(class_block)
        if ignore_queries_match:
            ignore_queries = True
        
        required_variants = []
        requires_matches = list(self.requires_pattern.finditer(class_block))
        for requires_match in requires_matches:
            requirements = requires_match.group(1)
            for req in requirements.split(','):
                req_type = req.strip()
                if req_type and req_type not in required_variants:
                    required_variants.append(req_type)
        
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
            'required_variants': required_variants,
            'is_variant': True,
            'ignore_queries': ignore_queries
        }

    def parse_regular_class(self, class_block: str, class_name: str, base_class: str) -> Optional[Dict[str, Any]]:
        properties = []
        
        for line in class_block.split('\n'):
            property_match = self.property_pattern.search(line)
            if property_match:
                prop_type = property_match.group(1).strip()
                prop_name = property_match.group(2).strip()
                callback_name = property_match.group(3)
                
                properties.append((prop_type, prop_name, callback_name))
                continue
            
            regular_match = self.regular_property_pattern.search(line)
            if regular_match:
                prop_type = regular_match.group(1).strip()
                prop_name = regular_match.group(2).strip()
                
                if '(' in line or 'operator' in line:
                    continue
                
                properties.append((prop_type, prop_name, None))
        
        if not properties:
            return None
        
        return {
            'class_name': class_name,
            'base_class': base_class,
            'properties': properties,
            'required_variants': [],
            'is_variant': False,
            'ignore_queries': False
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
        
        variant_classes = {}
        for variant_match in variant_matches:
            variant_class_name = variant_match.group(1)
            variant_classes[variant_class_name] = True
        
        results = []
        
        for class_match in class_matches:
            class_type = class_match.group(1)  
            class_name = class_match.group(2)
            base_class = class_match.group(3)
            
            if '<' in class_name or '>' in class_name:
                continue
            
            class_block = self.find_class_block(content, class_match)
            
            is_variant = class_name in variant_classes
            
            if is_variant:
                variant_info = self.parse_variant_class(class_block, class_name, base_class)
                if variant_info:
                    results.append(variant_info)
            else:
                regular_info = self.parse_regular_class(class_block, class_name, base_class)
                if regular_info:
                    results.append(regular_info)
        
        return results
    
    def find_cpp_file(self, header_path: str, source_root: str) -> Optional[str]:
        header_basename = os.path.basename(os.path.splitext(header_path)[0])
        cpp_filename = header_basename + ".cpp"
        
        header_dir_parts = os.path.dirname(header_path).split(os.sep)
        category_dir = None
        for part in header_dir_parts:
            if part != "include" and part != "engine" and part != "editor":
                category_dir = part
                break
        
        potential_paths = []
        
        if category_dir:
            potential_paths.append(os.path.join(source_root, category_dir, cpp_filename))
        
        potential_paths.append(os.path.join(source_root, cpp_filename))
        
        for path in potential_paths:
            if os.path.exists(path):
                return path
        
        for root, _, files in os.walk(source_root):
            if cpp_filename in files:
                return os.path.join(root, cpp_filename)
        
        return None
    
    def extract_query_dependencies(self, cpp_content: str) -> Set[str]:
        dependencies = set()
        
        cpp_content = self.clean_content(cpp_content)
        
        for match in self.query_get_pattern.finditer(cpp_content):
            template_params = match.group(1)
            variants = [param.strip() for param in template_params.split(',')]
            dependencies.update(variants)
        
        for match in self.query_read_pattern.finditer(cpp_content):
            template_params = match.group(1)
            variants = [param.strip() for param in template_params.split(',')]
            dependencies.update(variants)
        
        for match in self.query_try_get_pattern.finditer(cpp_content):
            template_params = match.group(1)
            variants = [param.strip() for param in template_params.split(',')]
            dependencies.update(variants)
        
        return dependencies


class CodeGenerator:
    @staticmethod
    def generate_base_classes_registration() -> str:
        return """    rttr::registration::class_<VariantCreateInfo>("VariantCreateInfo")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("entity_id", &VariantCreateInfo::entity_id);
    
    rttr::registration::class_<VariantBase>("VariantBase")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("entity_id", &VariantBase::entity_id)(rttr::metadata("NO_SERIALIZE", true));\n\n"""

    @staticmethod
    def generate_raylib_registration() -> str:
        return """    rttr::registration::class_<Vector2>("Vector2")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("x", &Vector2::x)
        .property("y", &Vector2::y)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<Vector3>("Vector3")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("x", &Vector3::x)
        .property("y", &Vector3::y)
        .property("z", &Vector3::z)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<Rectangle>("Rectangle")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("x", &Rectangle::x)
        .property("y", &Rectangle::y)
        .property("width", &Rectangle::width)
        .property("height", &Rectangle::height)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<Color>("Color")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("r", &Color::r)
        .property("g", &Color::g)
        .property("b", &Color::b)
        .property("a", &Color::a)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<Camera2D>("Camera2D")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("offset", &Camera2D::offset)
        .property("target", &Camera2D::target)
        .property("rotation", &Camera2D::rotation)
        .property("zoom", &Camera2D::zoom)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<Texture2D>("Texture2D")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("id", &Texture2D::id)
        .property("width", &Texture2D::width)
        .property("height", &Texture2D::height)
        .property("mipmaps", &Texture2D::mipmaps)
        .property("format", &Texture2D::format)
        (rttr::metadata("NO_VARIANT", true));\n\n"""

    @staticmethod
    def generate_variant_class_registration(class_info: Dict[str, Any]) -> str:
        class_name = class_info["class_name"]
        
        code = f'    rttr::registration::class_<{class_name}>("{class_name}")\n'
        code += '        .constructor<>()(rttr::policy::ctor::as_object)\n'
        code += '        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)'
        
        for prop_type, prop_name, callback_name in class_info['properties']:
            code += f'\n        .property("{prop_name}", &{class_name}::{prop_name})'
            
            if callback_name:
                code += f'(rttr::metadata("SET_CALLBACK", "{callback_name}"))'
        
        has_callbacks = any(callback_name for _, _, callback_name in class_info['properties'] if callback_name)
        if has_callbacks:
            code += '\n'
            for prop_type, prop_name, callback_name in class_info['properties']:
                if callback_name:
                    code += f'\n        .method("{callback_name}", &{class_name}::{callback_name})'
        
        code += ';\n\n'
        return code

    @staticmethod
    def generate_regular_class_registration(class_info: Dict[str, Any]) -> str:
        class_name = class_info["class_name"]
        
        code = f'    rttr::registration::class_<{class_name}>("{class_name}")\n'
        code += '        .constructor<>()(rttr::policy::ctor::as_object)'
        
        for prop_type, prop_name, _ in class_info['properties']:
            code += f'\n        .property("{prop_name}", &{class_name}::{prop_name})'
        
        code += '\n        (rttr::metadata("NO_VARIANT", true))'
        
        code += ';\n\n'
        return code

    @staticmethod
    def generate_full_registration(classes_info: List[Dict[str, Any]]) -> str:
        registration_code = "RTTR_REGISTRATION\n{\n"
        
        registration_code += CodeGenerator.generate_base_classes_registration()
        
        registration_code += CodeGenerator.generate_raylib_registration()
        
        for class_info in classes_info:
            if class_info['is_variant']:
                registration_code += CodeGenerator.generate_variant_class_registration(class_info)
            else:
                registration_code += CodeGenerator.generate_regular_class_registration(class_info)
        
        registration_code += "}\n"
        return registration_code

    @staticmethod
    def generate_requires_file(class_name: str, required_variants: List[str], output_dir: str) -> None:
        if not required_variants:
            return
            
        requires_file_path = os.path.join(output_dir, f"{class_name}.requires")
        
        requires_data = {
            "requires": required_variants
        }
        
        try:
            os.makedirs(output_dir, exist_ok=True)
            with open(requires_file_path, 'w') as f:
                json.dump(requires_data, f, indent=4)
            print(f"Generated requirements file: {requires_file_path}")
        except Exception as e:
            print(f"Error writing requirements file {requires_file_path}: {e}")


class ProjectFinder:
    @staticmethod
    def find_root_folders():
        current_dir = os.path.abspath(os.path.curdir)
        
        engine_dir = None
        project_root = None
        shared_resources_dir = None
        
        max_depth = 5
        current_depth = 0
        search_path = current_dir
        
        while current_depth < max_depth:
            if os.path.isdir(os.path.join(search_path, "engine")) and os.path.isdir(os.path.join(search_path, "editor")):
                project_root = search_path
                engine_dir = os.path.join(search_path, "engine")
                
                shared_resources_path = os.path.join(search_path, "shared_resources")
                if os.path.isdir(shared_resources_path):
                    shared_resources_dir = shared_resources_path
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
        
        if project_root and not shared_resources_dir:
            shared_resources_dir = os.path.join(project_root, "shared_resources")
            os.makedirs(shared_resources_dir, exist_ok=True)
        
        return {
            "engine_dir": engine_dir,
            "project_root": project_root,
            "shared_resources_dir": shared_resources_dir
        }


class RTTRGenerator:
    def __init__(self):
        paths = ProjectFinder.find_root_folders()
        
        self.engine_dir = paths.get("engine_dir")
        if not self.engine_dir:
            print("Error: Could not locate the engine directory")
            print("Please run this script from within the project directory structure")
            sys.exit(1)
            
        self.headers_dir = os.path.join(self.engine_dir, "include")
        self.source_dir = os.path.join(self.engine_dir, "source")
        self.game_headers_dir = os.path.join(self.headers_dir, "game")
        self.game_source_dir = os.path.join(self.source_dir, "game")

        self.shared_resources_dir = paths.get("shared_resources_dir")

        print(f"Found engine directory: {self.engine_dir}")
        print(f"Using headers directory: {self.headers_dir}")
        print(f"Using source directory: {self.source_dir}")
        print(f"Using shared resources directory: {self.shared_resources_dir}")
        
        self.parser = ClassParser()
        self.classes_info = []
        self.includes = set()
        
        self.includes.add('#include "raylib.h"')
        self.includes.add('#include "rttr/registration.h"')

    def process_headers(self) -> None:
        header_files = glob.glob(os.path.join(self.game_headers_dir, "**/*.h"), recursive=True)
        
        print(f"Processing {len(header_files)} header files...")
        
        for header_file in header_files:
            relative_path = os.path.normpath(header_file).replace('\\', '/')
            
            relative_path = os.path.relpath(header_file, self.headers_dir)
            
            class_infos = self.parser.parse_header(header_file)
            if class_infos:
                for class_info in class_infos:
                    self.classes_info.append(class_info)
                    self.includes.add(f'#include "{relative_path}"')

    def analyze_implementation_files(self) -> None:
        print("Analyzing implementation files for dependencies...")
        for class_info in self.classes_info:
            if not class_info['is_variant'] or class_info['ignore_queries']:
                if class_info['is_variant'] and class_info['ignore_queries']:
                    print(f"Skipping dependency analysis for {class_info['class_name']} (IGNORE_QUERIES is set)")
                continue
                
            class_name = class_info['class_name'].lower()
            
            for header_file in glob.glob(os.path.join(self.headers_dir, "**/*.h"), recursive=True):
                if os.path.basename(header_file).lower() == f"{class_name}.h":
                    cpp_file = self.parser.find_cpp_file(header_file, self.source_dir)
                    if cpp_file:
                        try:
                            with open(cpp_file, 'r') as f:
                                cpp_content = f.read()
                            
                            dependencies = self.parser.extract_query_dependencies(cpp_content)
                            
                            for dep in dependencies:
                                if dep not in class_info['required_variants'] and dep != class_info['class_name']:
                                    class_info['required_variants'].append(dep)
                                    print(f"Added auto-detected dependency: {class_info['class_name']} requires {dep}")
                        except Exception as e:
                            print(f"Error analyzing cpp file {cpp_file}: {e}")
                    break

    def generate_rttr_header(self, output_path: str) -> None:
        includes_code = "\n".join(sorted(self.includes)) + "\n\n"
        registration_code = CodeGenerator.generate_full_registration(self.classes_info)
        final_code = includes_code + registration_code
        
        try:
            os.makedirs(os.path.dirname(output_path), exist_ok=True)
            with open(output_path, "w") as f:
                f.write(final_code)
            
            variant_count = sum(1 for c in self.classes_info if c['is_variant'])
            regular_count = sum(1 for c in self.classes_info if not c['is_variant'])
            
            print(f"Generated RTTR registration code for {len(self.classes_info)} classes:")
            print(f"  - {variant_count} variant classes")
            print(f"  - {regular_count} regular classes")
            print(f"Output written to {output_path}")
        except Exception as e:
            print(f"Error writing RTTR header {output_path}: {e}")

    def generate_requires_files(self, requires_dir: str) -> None:
        if os.path.exists(requires_dir):
            print(f"Clearing existing requires directory: {requires_dir}")
            try:
                for file in os.listdir(requires_dir):
                    file_path = os.path.join(requires_dir, file)
                    if os.path.isfile(file_path) and file.endswith('.requires'):
                        os.remove(file_path)
            except Exception as e:
                print(f"Error clearing requires directory: {e}")
        
        os.makedirs(requires_dir, exist_ok=True)
        
        for class_info in self.classes_info:
            if class_info['is_variant'] and class_info['required_variants']:
                CodeGenerator.generate_requires_file(
                    class_info['class_name'], 
                    class_info['required_variants'], 
                    requires_dir
                )
        
        print(f"Requirements files written to {requires_dir}")

    def run(self) -> None:
        self.process_headers()
        self.analyze_implementation_files()

        output_path = os.path.join(self.game_headers_dir, "generated/rttr_registration.h")
        
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        
        self.generate_rttr_header(output_path)
        
        requires_dir = os.path.join(self.shared_resources_dir, "variants", "requires")
        self.generate_requires_files(requires_dir)


def main():
    generator = RTTRGenerator()
    generator.run()

if __name__ == "__main__":
    main()
