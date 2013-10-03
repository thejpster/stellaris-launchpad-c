#!/bin/bash

# This will fetch Ubuntu's Linux cross-compiler. It works fine for basic bare-metal compilation.

set -e

fetch_ubuntu_compiler()
{
    echo Installing gcc-arm-linux-gnueabi using apt-get
    sudo apt-get install gcc-arm-linux-gnueabi
}

# This will fetch a pre-build ARM toolchain designed for the Cortex-M4

fetch_arm_compiler()
{
    # We previously used https://launchpad.net/gcc-arm-embedded/4.6/4.6-2012-q4-update/+download/gcc-arm-none-eabi-4_6-2012q4-20121016.tar.bz2
    if [ ! -d ./gcc-arm ]
    then
        wget https://launchpad.net/gcc-arm-embedded/4.7/4.7-2013-q3-update/+download/gcc-arm-none-eabi-4_7-2013q3-20130916-linux.tar.bz2
        tar xvjf gcc-arm-none-eabi-4_7-2013q3-20130916-linux.tar.bz2
        rm gcc-arm-none-eabi-4_7-2013q3-20130916-linux.tar.bz2
        ln -s ./gcc-arm-none-eabi-4_7-2013q3 ./gcc-arm
    else
        echo Skipping ARM compiler - already installed
    fi
}

# This will fetch and install lm4tools so we can flash the Launchpad
fetch_lm4tools()
{
    if [ ! -d lm4tools ]
    then
        echo Installing libusb-1.0.0-dev using apt-get
        sudo apt-get install libusb-1.0.0-dev
        echo Fetching lm4tools
        git clone https://github.com/utzig/lm4tools.git
        cd lm4tools/lm4flash
        echo Building lm4tools
        make
    else
        echo Skipping LM4Tools - already installed
    fi
}

#fetch_ubuntu_compiler
fetch_arm_compiler
fetch_lm4tools
