#!/bin/bash

# Store current directory
original_dir=$(pwd)

# Change directory to ~/ece344/build
cd ~/ece344/build

# Run python3 test.sh
python3 test.py

# Change back to the original directory
cd "$original_dir"

