"""
Wrapper around the main Scons file (./src/SConscript) which puts all the
intermediates in ./bin

Copyright (c) 2012 theJPster (github@thejpster.org.uk)
"""

# Use ./bin for building to keep the source tree clean
VariantDir('bin', 'src', duplicate=0)
# We say ./bin/X but we mean ./src/X, thanks to the line above
SConscript('bin/SConscript')
