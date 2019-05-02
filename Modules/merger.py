#!/usr/bin/env python
# Merges all modules together because I'm lazy ;^)

import subprocess as sbp
import os
import shutil
import time

from distutils.dir_util import copy_tree

modules = ["appstore", "bootlogo", "edizon", "goldleaf", 
        "hbmenu", "hekate_payload", "must_have", "kosmosupdater", 
        "sys-ftpd", "sys-netcheat", "kosmos_toolkit", "emuiibo",
        "lockpick", "hid-mitm", "sys-clk", "ldn_mitm"] # Everything that will be merged together
output_dir = "compiled" # How the merged folder should be called

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
""") # Fancy stuff

if os.path.exists(output_dir):
    shutil.rmtree(output_dir) # Delete Content of "compiled" if it exists!

os.makedirs(output_dir) # Double check

# Copy modules to "compiled" directory
working_dir = os.path.dirname(os.path.realpath(__file__))
for path in modules:
    path = os.path.join(working_dir, path)
    copy_tree(path, output_dir)

print("Done!")