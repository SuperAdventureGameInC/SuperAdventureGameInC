import os

# This creates an environment to compile and link with.
# Using 'ENV = os.environ' tells SCons to use the paths in your current system to find tools like your compiler and linker.
environment = Environment(ENV = os.environ)

# SConscript executes the build script specified.
SConscript(dirs=["src"], exports=["environment"])
