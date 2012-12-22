#!/bin/sh

sudo apt-get install gcc-arm-linux-gnueabi libusb-1.0.0-dev
git clone https://github.com/utzig/lm4tools.git
cd lm4tools/lm4flash
make
