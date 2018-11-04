#!/usr/bin/env python
# Merges all modules together because I'm lazy ;^)

import subprocess as sbp
import os
import shutil

modules = ["appstore", "bootlogo", "checkpoint", "edizon", "es_patches", 
        "fusee_atmosphere", "hbmenu", "hekate_payload", "must_have", 
        "reinx", "sdfilesupdater", "sunpresence", "switchpresence", 
        "sys-ftpd", "sys-netcheat", "xor.play", "sdfiles_toolkit", 
        "kip_patches", "tinfoil"] # Everything that will be merged together
p2 = "compiled" # How the merged folder should be called

print("""
                        https://github.com/tumGER/
   _____ _____  ______ _ _            _____         _ _       _     
  / ____|  __ \|  ____(_) |          / ____|       (_) |     | |    
 | (___ | |  | | |__   _| | ___  ___| (_____      ___| |_ ___| |__  
  \___ \| |  | |  __| | | |/ _ \/ __|\___ \ \ /\ / / | __/ __| '_ \ 
  ____) | |__| | |    | | |  __/\__ \____) \ V  V /| | || (__| | | |
 |_____/|_____/|_|    |_|_|\___||___/_____/ \_/\_/ |_|\__\___|_| |_|
                                                                    
                    By: @_tomGER (tumGER on Github)

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

print("Done!")