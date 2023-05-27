#!/bin/bash

# Check if given file in first argument exists
if ! [ -e $1 ]; then
    echo "File $1 doesn't exist"
    exit -1
fi

# Check if given file in first argument exists
if ! [ -r $1 ]; then
    echo "$1 doesn't have read permissions"
    exit -1
fi

output_file="../results/pollerResultsFile"

# If pollerResultsFile exists remove it
if [ -e "$output_file" ]; then
    rm "$output_file"
fi

touch "$output_file" # Create output file

num_input_lines=$(wc -l < $1)   # Get number lines of input file

declare -A parties_to_votes   # Associative array (map) that holds number of votes for each party
declare -A voters

# Loop over the lines of given file
for ((i=1; i <= num_input_lines; i++)) do
    curr_line=$(head -$i $1| tail -1)    # Extract the current line

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