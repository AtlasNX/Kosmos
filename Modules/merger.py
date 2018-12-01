#!/usr/bin/env python
# Merges all modules together because I'm lazy ;^)

import subprocess as sbp
import os
import shutil
import time

modules = ["appstore", "bootlogo", "checkpoint", "edizon", "es_patches", 
        "hbmenu", "hekate_payload", "must_have", "kosmosupdater", 
        "sys-ftpd", "sys-netcheat", "kosmos_toolkit", 
        "tinfoil", "ldn_mitm"] # Everything that will be merged together
p2 = "compiled" # How the merged folder should be called

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

if os.path.exists("compiled"):
    shutil.rmtree("compiled") # Delete Content of "compiled" if it exists!

os.makedirs("compiled") # Double check

for path in modules:
    fol = os.listdir(path)
    for i in fol:
        p1 = os.path.join(path,i)
        p3 = 'cp -r ' + p1 +' ' + p2+'/.'
        sbp.Popen(p3,shell=True)
        time.sleep(0.1)

print("Done!")
