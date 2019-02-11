#!/usr/bin/env python
# Merges all modules together because I'm lazy ;^)

import os
import shutil

if os.path.exists("compiled"):
    shutil.rmtree("compiled") 

modules = [module for module in os.listdir() if os.path.isdir(module)]

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

print(r"""
                        https://github.com/AtlasNX/Kosmos
 ____  __.                                 
|    |/ _|____  ______ _____   ____  ______
|      < /  _ \/  ___//     \ /  _ \/  ___/
|    |  (  <_> )___ \|  Y Y  (  <_> )___ \ 
|____|__ \____/____  >__|_|  /\____/____  >
        \/         \/      \/           \/ 
                                                                    
                    By: @AtlasNX (AtlasNX/Kosmos on Github)""")

os.makedirs("compiled")

for path in modules:
    copyDir(path, "compiled")

print("Done!")
