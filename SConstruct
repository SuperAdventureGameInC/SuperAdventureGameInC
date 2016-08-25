import os
import sys

# This creates an environment to compile and link with.
# Using 'ENV = os.environ' tells SCons to use the paths in your current system to find tools like your compiler and linker.
environment = Environment(ENV = os.environ)

if os.name == "posix":
	environment.Append(CFLAGS = " -g -Werror ", LINKFLAGS = " -g ")

turbojson = SConscript(dirs=["TurboJSON"], exports=["environment"])

environment.Append(CPPPATH = os.getcwd())

def EnabledString(that):
    if that:
        return "enabled"
    else:
        return "disabled"

def EnableableOption(name, default):
    AddOption("--enable-" + name,
        action="store_true", dest="en_" + name, default=False,
        help="Enables " + name)
    AddOption("--disable-" + name,
        action="store_true", dest="dis_" + name, default=False,
        help="Disables " + name)
    enable = GetOption("en_" + name)
    disable = GetOption("dis_" + name)
    if enable and disable:
        print("Warning: Both enable and disable " + name + " specified.")
        return default
    if disable:
        return False
    elif enable:
        return True
    else:
        return default

use_audio = EnableableOption("audio", not (("win" in sys.platform) or ("msys" in sys.platform)))

# SConscript executes the build script specified.
SConscript(dirs=["src"], exports=["environment", "turbojson", "use_audio"])
