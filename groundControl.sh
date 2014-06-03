#!/bin/bash

DEF_REMOTE="auvsi@192.168.163.23"
DEF_FOLDER="/home/auvsi/AUVSI/Desert/CCOut"

if [ $# == 3 ]; then
    if [ $3 == 0 ]; then
        ssh $1 "touch $2/stop.txt"
    elif [ $3 == 1 ]; then
        ssh $1 "touch $2/start.txt"
    else
        echo -e "Stop/Start not correctly specified!"
        echo "Needs three arguments: remote host (user@ip), output (remote) folder, and whether to stop or start (0 or 1, respectively)"
    fi

elif [ $# == 2 ]; then
    if [ $2 == 0 ]; then
        ssh $DEF_REMOTE "touch $1/stop.txt"
    elif [ $2 == 1 ]; then
        ssh $DEF_REMOTE "touch $1/start.txt"
    else
        echo -e "Stop/Start not correctly specified!"
        echo "Needs two arguments: output (remote) folder, and whether to stop or start (0 or 1, respectively). Uses $DEF_REMOTE as remote host"
    fi

elif [ $# == 1 ]; then
    if [ $1 == 0 ]; then
        ssh $DEF_REMOTE "touch $DEF_FOLDER/stop.txt"
    elif [ $1 == 1 ]; then
        ssh $DEF_REMOTE "touch $DEF_FOLDER/start.txt"
    else
        echo -e "Stop/Start not correctly specified!"
        echo "Needs one argument: Whether to stop or start (0 or 1, respectively). Uses $DEF_REMOTE as remote host, $DEF_FOLDER as output (remote) folder"
    fi
else
    echo -e "\nUsage:\n Three Args: remote host (user@ip), output (remote) folder, and whether to stop or start (0 or 1, respectively)\tOR\n"
    echo -e "Two Args: Output (remote) folder, and whether to stop or start (0 or 1, respectively). Uses $DEF_REMOTE as remote host\tOR\n"
    echo -e "One Arg: Whether to stop or start (0 or 1, respectively). Uses $DEF_REMOTE as remote host, $DEF_FOLDER as output (remote) folder\n"
fi
