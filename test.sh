#!/bin/bash

# This script waits for a specific command input
# and then executes "p testbin/forktest"

# Define the command that you want to wait for
expected_command="sh build_and_run.sh"

echo "Waiting for the specific command input..."

# Use a loop to continuously check for input
while true; do
    # Read the user input
    read -p "Input command: " input_command
    
    # Check if the input command matches the expected command
    if [ "$input_command" == "$expected_command" ]; then
        echo "Executing 'p testbin/forktest'..."
        # Execute the command
        p testbin/forktest
        # Exit the loop after executing the command
        break
    else
        echo "Input command does not match. Please try again."
    fi
done

echo "Script execution completed."




