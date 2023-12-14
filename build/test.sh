#!/bin/bash
# Script to run raytracer.exe with each .txt file as an argument

# Loop through all .txt files in the current directory
for file in *.txt; do
    # Run raytracer.exe with the current file as the argument
    ./raytracer.exe "$file"
done