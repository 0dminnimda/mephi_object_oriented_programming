#!/usr/bin/env bash

NAME=$(basename "$1")
if [[ $2 == "--use_dir" && $3 ]]; then
    DIR="$3"
else
    DIR=""
fi

if [[ $OSTYPE =~ "android" ]]; then
    if [[ $DIR ]]; then
        cp -f -r $DIR ~/$DIR
    fi
    cp -f $1 ~/$NAME
    chmod +x ~/$NAME

    echo "~/$NAME"
else
    echo "$1"
fi
