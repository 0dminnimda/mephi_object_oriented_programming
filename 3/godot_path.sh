#!/usr/bin/env bash

if [[ $OSTYPE =~ "android" ]]; then
    echo "Oops"
elif [[ $OSTYPE =~ "msys" ]]; then
    echo "C:\Godot\Godot_v4.1.1-stable_win64\Godot_v4.1.1-stable_win64_console.exe"
else
    echo "Oops"
fi
