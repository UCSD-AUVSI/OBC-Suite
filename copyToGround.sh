#!/bin/bash

if [ $# != 3 ]; then
    echo "Needs three arguments: remote host (user@ip), input (remote) folder, and output (local) folder"
else
    while true; do
        fnlist=""
        numfiles=0
        
        for fn in `diff <(ssh $1 "ls -1 $2") <(ls -1 $3) | grep -i '^< [0-9_]*\.[jt]' |grep -oi '[0-9_.a-z]*'`; do
            fnlist="$fn,$fnlist"
            numfiles=`expr $numfiles + 1`
        done
        fnlist=${fnlist%","}
        fnlist=\{$fnlist\}
        
        echo $fnlist
        if [ $numfiles -gt 0 ]; then
            scp -q $1:$2/$fnlist $3
        fi

        sleep 10
    done
fi
