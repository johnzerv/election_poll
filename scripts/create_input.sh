#!/bin/bash

# Check number of command line arguments
if [ $# -ne 2 ]; then
    echo Wrong number of command line arguments
    exit -1
fi

# Check if given file in first argument exists
if ! [ -e $1 ]; then
    echo "File $1 doesn't exist"
    exit -1
fi

# Check if numLines > 0
if [ $2 -lt 1 ]; then
    echo "Number of lines must be greater than 0"
    exit -1
fi

output_file="../results/inputFile"

# If inputFile exists, remove it
if [ -e "$output_file" ]; then
    rm "$output_file"
fi

touch "$output_file" # Create "$output_file"

num_input_lines=$(wc -l < $1)   # Get number of lines of first argument file

for ((i=0; i < $2; i++)) do
    num_line_ceil=$(($num_input_lines + 1))
    num_line=`expr $RANDOM % $num_line_ceil + 1`    # Get a random number line of first argument file

    party=$(head -$num_line $1| tail -1)    # Extract the party from input file

    let length="3 + $RANDOM % 10"   # Get a random number in range [3, 12]

    characters=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ     # define all characters that can be included in firstname and lastname

    # Create an array and fill it with random characters in [a-z] and [A-Z]
    firstname=()
    for ((char=0; char < length; char++)) do
        firstname+=${characters:$((RANDOM % ${#characters})):1}   # Extract a random character from the array of characters

    done

    # Same job for lastname
    lastname=()
    for ((char=0; char < length; char++)) do
        lastname+=${characters:$(( RANDOM % ${#characters} )):1}

    done

    echo $firstname $lastname $party >> "$output_file"
done