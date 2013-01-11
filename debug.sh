#!/bin/bash

if [ "x$1" == "x" ]
then
	echo "Run as $0 <path_to_binary>"
	echo "e.g. $0 ./bin/start.elf"
	exit
fi

# location of GDB
GDB=./gcc-arm-none-eabi-4_6-2012q4/bin/arm-none-eabi-gdb

# location of OpenOCD binary
OPENOCD=/usr/local/bin/openocd

# location of OpenOCD Board .cfg files (only used with 'make program')
OPENOCD_BOARD_DIR=/usr/local/share/openocd/scripts/board

# start xterm with openocd in the background
xterm -e $OPENOCD -f $OPENOCD_BOARD_DIR/ek-lm4f120xl.cfg &

# save the PID of the background process
XTERM_PID=$!

# wait a bit to be sure the hardware is ready
sleep 2

# execute some initialisation commands via gdb
$GDB --batch --command=init.gdb $1

# start the gdb gui
nemiver --remote=localhost:3333 --gdb-binary="$GDB" $1

# close xterm when the user has exited nemiver
kill $XTERM_PID

