#!/bin/bash

# Check if  inputFile exists
input_file="../results/inputFile"

if ! [ -e $input_file ]; then
    echo "inputFile doesn't exist"
    exit -1
fi

# Check if inputFile exists
if ! [ -r $input_file ]; then
    echo "inputFile doesn't have read permissions"
    exit -1
fi

output_file="../results/$1"

# If given file exists remove it
if [ -e "$output_file" ]; then
    rm "$output_file"
fi

touch "$output_file"    # Create file with given name

num_input_lines=$(wc -l < $input_file)   # Get number lines of input file

declare -A parties_to_votes   # Associative array (map) that holds number of votes for each party
declare -A voters

# Loop over the lines of inputFile
for ((i=1; i <= num_input_lines; i++)) do
    curr_line=$(head -$i $input_file| tail -1)    # Extract the current line

    # firstname=$(echo $curr_line | cut -d ' ' -f 1)  # Extract first column (firstname)
    # lastname=$(echo $curr_line | cut -d ' ' -f 2)   # Extract second column (lastname)
    name=$(echo $curr_line | cut -d ' ' -f 1,2)     # Extract first and second columnt (firstname and lastname)

    if [[ "${voters[$name]}" ]]; then   # Check for duplicates, if name has appeared continue the loop without 
        continue                        # count the new vote
    else
        voters[$name]=1
    fi

    party=$(echo $curr_line | cut -d ' ' -f 3-)     # Extract rest columns (party)

    if [[ "${parties_to_votes[$party]}" ]]; then # Checks if value's length associated with the current party is zero
        tmp=$((${parties_to_votes[$party]} + 1))    # Current party's value++
        parties_to_votes[$party]=$tmp               # Update the array
    else
        parties_to_votes[$party]=1                  # If this is the first time party appeard, initialize the value with 1
    fi
done

# Loop over the keys and print key and value to given file
for key in "${!parties_to_votes[@]}"; do    # Using @ instead of * to include parties with character space
    echo "$key ${parties_to_votes[$key]}"  >> "$output_file"
done

echo >> "$output_file"    # New line at the end of the file