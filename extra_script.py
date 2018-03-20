from os.path import join
import os
import sys

Import("env")

platform = env.PioPlatform()

FRAMEWORK_DIR = platform.get_package_dir("framework-arduinoespressif32")

def before_partitioning(source, target, env):
    print "DEBUG - before partitioning"
    # do some actions
    print "DEBUG --------------"
    #print env.Dump()

    #boardConfig = env.BoardConfig()
    #sys.stderr.write('\nPath: ')
    #sys.stderr.write(boardConfig)
    #sys.stderr.write('\n')
    print "DEBUG --------------"

def after_partitioning(source, target, env):
    print "DEBUG - after partitioning"
    # do some actions
    path = join(env.get("PROJECTBUILD_DIR"), env.get("PIOENV"), "partitions.bin")
    sys.stderr.write('\nPath: ')
    sys.stderr.write(path)
    sys.stderr.write('\n')
    fileToRemove = join(env.get("PROJECTBUILD_DIR"), env.get("PIOENV"), "partitions.bin")
    sys.stderr.write("Deleting: ")
    sys.stderr.write(fileToRemove)
    sys.stderr.write('\n')
    os.remove(fileToRemove)
    env.Command(
    join("$BUILD_DIR", "partitions.bin"),
    join(FRAMEWORK_DIR, "tools", "partitions",
         "maxapp.csv"),
    env.VerboseAction('"$PYTHONEXE" "%s" -q $SOURCE $TARGET' % join(
        FRAMEWORK_DIR, "tools", "gen_esp32part.py"),
                      "Generating partitions $TARGET")
		)
    print "DEBUG --------------"

env.AddPreAction("$BUILD_DIR/partitions.bin", before_partitioning)
env.AddPostAction("$BUILD_DIR/partitions.bin", after_partitioning)
print env.Dump()