#!/usr/bin/env bash

NAME=$(basename "$1")
DIR=$(dirname "$1")
USE_DIR=$(( $2 == "--use_dir" ? 1 : 0 ))


if [[ $OSTYPE =~ "android" ]]; then
    if [[ $USE_DIR ]]; then
        cp -f -r $DIR ~/
    else
        cp -f $1 ~/$NAME
    fi
    chmod +x ~/$NAME

    echo "~/$NAME"
else
    echo "$1"
fi
