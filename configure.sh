#!/bin/bash

#define YAFELDIR and append it to the user's .bashrc if undefined
YD=$(pwd)
if [ $YD != "$YAFELDIR" ]; then
    echo >> ~/.bashrc
    echo "#=============================" >> ~/.bashrc
    echo "# Environment Setup for YAFEL" >> ~/.bashrc
    echo "export YAFELDIR=$YD" >> ~/.bashrc
    echo "#=============================" >> ~/.bashrc
fi

YAFELDIR=$YD
export YAFELDIR