launchpad
=========

Starter project for TI's Stellaris Launchpad that doesn't require you to have StellarisWare installed. This saves you a 340MB download and various lengthly click-through EULAs.
 
The BSD-licensed header file for the Launchpad's microprocessor (LM4F120H5QR) is included in this project. All you need is the flash tool (lm4tools) and a compiler and there's a shell script that will fetch both of those for you.

Once you have the prerequisites (i.e. run prerequisities.sh), simply type:

> scons

to build and

> scons flash

to program. Connect a serial terminal of your choice to /dev/serial/by-id/usb-Texas* (probably a symlink to /dev/ttyACM0, but it depends on what else you have connected) to view the debug output. Press the buttons to change the colour of the LED.

All source code that is marked "Copyright (c) 201x theJPster" is subject to the following license:

> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Thanks must go to

* Mauro Scomparin <http://scompoprojects.wordpress.com>
* http://eehusky.wordpress.com/
