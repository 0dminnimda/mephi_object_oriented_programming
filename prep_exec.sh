#!/usr/bin/env bash

NAME=$(basename "$1")

if [[ $OSTYPE =~ "android" ]]; then
    cp -f $1 ~/$NAME
    chmod +x ~/$NAME

    echo "~/$NAME"
else
    echo "$1"
fi
