#!/bin/sh
for i in `find src -name "*.h"`
do
	./gcc-arm/bin/arm-none-eabi-gcc -Isrc $i -o /dev/null
done
