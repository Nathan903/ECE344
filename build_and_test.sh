#!/bin/bash
# git fetch origin        # Fetches the latest changes from the remote repository
# git reset --hard origin/master  # Resets your local branch to match the remote branch
# git clean -df           # Removes untracked files and directories (use with cgaution)
# git pull origin master  # Pulls the latest changes from the remote repository

set -e
print_error() {
    echo -e "\e[91mERROR\e[0m"
}



cd ~/ece344/os161
./configure --werror --ostree=$HOME/ece344/build > /dev/null
make > /dev/null
cd ~/ece344/os161
cd kern/conf
./config ASST3 > /dev/null

cd ~/ece344/os161/kern
cd compile/ASST3
make depend > /dev/null
make > /dev/null
make install > /dev/null

cd ~/ece344/build
cp /cad2/ece344s/tester/sysconfig/sys161-asst3.conf sys161.conf
sed -i '0,/autoseed/s//seed=1/' sys161.conf
cd ..
cd build
os161-tester 3 2>&1 | tee ../os161/testlog

