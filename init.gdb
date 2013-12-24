# Specify remote target
target extended-remote :3333

# Reset to known state
monitor reset halt
load
monitor reset init

# Set a breakpoint at main().
break main
break _sbrk
break hardfault_handler

# Run to the breakpoint.
continue
