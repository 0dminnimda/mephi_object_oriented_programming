#!/usr/bin/env bash

if [[ $OSTYPE =~ "android" ]]; then
    cp -f $1 ~/$1
    chmod +x ~/$1

    echo "~/$1"
else
    echo "$1"
fi
