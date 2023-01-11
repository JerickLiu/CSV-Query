#!/bin/sh
#This script deletes all sub directories in the parent directory.

# Checks each file in the parent directory
for file in `ls`; do

    # Checks if the current file is a sub directory of the parent
    if [ -d $file ]; then

        # Removes the sub directory and any files within them
        rm -r $file
        echo Removed ./$file
    fi
done