#!/usr/bin/env bash

cp -f $1 ~/$1
chmod +x ~/$1

(cd ~/ ; $2 ./$1)
