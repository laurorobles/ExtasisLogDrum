#!/usr/bin/env python3
"""
Validates documentation & open-source release consistency for ExtasisLogDrum.
"""
import os
import re
import sys

REQUIRED_FILES = [
    'LICENSE',
    'README.md',
    'MANUAL.md',
    'PRESENTATION.md',
    'TECHNICAL.md',
    'ARCHITECTURE.md',
    'CHANGELOG.md',
    'CMakeLists.txt',
    'assets/screenshot.png',
    'assets/logo.png',
]


def get_cmake_version(project_dir: str) -> str:
    cmake_path = os.path.join(project_dir, 'CMakeLists.txt')
    with open(cmake_path, 'r', encoding='utf-8') as f:
        match = re.search(r'project\s*\(\s*ExtasisLogDrum\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)', f.read())
        return match.group(1) if match else 'UNKNOWN'


def validate_docs() -> int:
    project_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    version = get_cmake_version(project_dir)
    print(f"ExtasisLogDrum Release Validation (CMake Version: {version})")
    print("-" * 55)

    errors = 0
    for rel_path in REQUIRED_FILES:
        full_path = os.path.join(project_dir, rel_path)
        if not os.path.exists(full_path):
            print(f"[MISSING] {rel_path}")
            errors += 1
        elif os.path.getsize(full_path) < 100 and rel_path.endswith('.md'):
            print(f"[TOO SHORT] {rel_path}")
            errors += 1
        else:
            print(f"[OK]      {rel_path}")

    for md in ('README.md', 'MANUAL.md', 'PRESENTATION.md', 'TECHNICAL.md', 'ARCHITECTURE.md', 'CHANGELOG.md'):
        full_path = os.path.join(project_dir, md)
        if os.path.exists(full_path):
            with open(full_path, 'r', encoding='utf-8') as f:
                content = f.read()
            if version != 'UNKNOWN' and version not in content:
                print(f"[WARN]    {md} does not mention version {version}")
                errors += 1

    print("-" * 55)
    if errors == 0:
        print("All documentation and release files are consistent and ready!")
        return 0
    print(f"Found {errors} issue(s).")
    return 1


if __name__ == '__main__':
    sys.exit(validate_docs())
