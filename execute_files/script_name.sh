#!/bin/bash

# Specify the path to the directory
directory="/home/loig/MocoLoco/remap_hg_bed"

# Check if the directory exists
if [ -d "$directory" ]; then
    # Change to the specified directory
    cd "$directory"

    # Flag to keep track of the first subdirectory
    first_subdir=true

    # Loop through all directories in the current directory
    for dir in */; do
        # Remove the trailing slash from the directory name
        dir_name="${dir%/}"

        # Check if it is the first subdirectory
        if $first_subdir; then
            first_subdir=false
        else
            # Navigate to the directory
            cd "$dir_name"

            # Loop through all files in the directory
            for file in *; do
                if [ -f "$file" ]; then
                    # Check if the file name ends with ".bed"
                    if [[ "$file" == *.bed ]]; then
                        echo "Skipping $file, already has the .bed extension."
                    else
                        # Append ".bed" to the file name
                        new_name="${file}.bed"
                
                        # Rename the file
                        mv "$file" "$new_name"
                        echo "Renamed $file to $new_name"
                    fi
                fi
            done

            echo "File renaming complete."
            # Move back to the parent directory
            cd ..
        fi
    done
else
    echo "Directory not found."
fi
