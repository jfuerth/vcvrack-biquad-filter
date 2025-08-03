#!/bin/bash
# Script to rebuild with bear to update compile_commands.json

set -e

echo "Cleaning previous build..."
make clean

echo "Building with bear to generate compile_commands.json..."
bear -- make

echo "Build complete! compile_commands.json has been updated."
echo "You may need to restart VS Code or run 'clangd: restart language server' command."
