#!/usr/bin/env python
# Merges all modules together because I'm lazy ;^)

import subprocess as sbp
import os
import shutil
import time
import json

with open("modules.json") as f:
    config = json.load(f)
modules = config.get("modules")


def copyDir(srcDir, dstDir):
    os.makedirs(dstDir, exist_ok=True)
    fds = os.listdir(srcDir)
    for fd in fds:
        srcfp = os.path.join(srcDir, fd)
        dstfp = os.path.join(dstDir, fd)

        if os.path.isdir(srcfp):
            copyDir(srcfp, dstfp)
        else:
            shutil.copy(srcfp, dstfp)

print("""
                        https://github.com/AtlasNX/Kosmos
 ____  __.                                 
|    |/ _|____  ______ _____   ____  ______
|      < /  _ \/  ___//     \ /  _ \/  ___/
|    |  (  <_> )___ \|  Y Y  (  <_> )___ \ 
|____|__ \____/____  >__|_|  /\____/____  >
        \/         \/      \/           \/ 
                                                                    
                    By: @AtlasNX (AtlasNX/Kosmos on Github)
It could be that a permissions error will pop up, fix it by restarting the python script! We don\'t really know why that happens!
""")  # Fancy stuff

if os.path.exists("compiled"):
    shutil.rmtree("compiled")  # Delete Content of "compiled" if it exists!

os.makedirs("compiled")  # Double check

for path in modules:
    copyDir(path, "compiled")

print("Done!")
