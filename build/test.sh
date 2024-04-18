#!/bin/bash

# Run my_sh script in the background
sys161 kernel &

# Wait for 1 second
sleep 1

# Send "a" as input to my_sh
echo "a" > /proc/2626124/fd/0

# Wait for my_sh to finish
wait