#!/bin/bash

if [[ $OSTYPE =~ "msys" ]]; then
    (python ../run_windows.py $@)
    # (MSYS_NO_PATHCONV=1 cmd /C python -c "\
    #     import subprocess, sys, os;\
    #     subprocess.Popen(['cmd.exe', '/C' + ' '.join(sys.argv[1:])], env={OS=os.environ['OS'], PATH=os.environ['PATH']})\
    # " $@)
    # (MSYS_NO_PATHCONV=1 cmd /C python -c "import os; print(list(os.environ.keys()))" $@)
    # (python -c "import os; print(list(os.environ.keys()))" $@)
    # MSYS_NO_PATHCONV=1 cmd /C "$@"
elif [[ $SHELL != "" ]]; then
    $@
else
    echo "Whoops, shit has hit the fan, where is yar 'SHELL'?"
    exit 1
fi
