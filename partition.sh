#!/bin/sh
#This script partitions the pokemon.csv file into unique type csv's

# Defining the file handler
readonly FILE="./pokemon.csv"

# Checks if the file exists
if [ -f $FILE ]; then

    # Obtains the header line from the pokemon file
    read HEADER<$FILE

    # Loops through the pokemon file; omitting the header line
    sed 1d $FILE | while read LINE; do

        TYPE=`echo $LINE | cut -d, -f3`

        # Checks if first time seeing pokemon type
        if [ ! -d "$TYPE" ]; then

            # Makes a sub directory with corresponding csv file with header
            mkdir $TYPE
            echo $HEADER > ./$TYPE/$TYPE.csv 
        fi

        # Adds the pokemon to the corresponding csv file
        echo $LINE >> ./$TYPE/$TYPE.csv
    done
fi
