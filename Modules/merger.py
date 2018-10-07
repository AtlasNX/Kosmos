#!/usr/bin/env python

import subprocess as sbp
import os

modules = ["appstore", "bootlogo", "dopus", "edizon", "es_patches", "fusee_atmosphere", "hbmenu", "hekate_payload", "must_have", "reinx", "sdfilesupdater", "sunpresence", "switchpresence", "sys-ftpd", "sys-netcheat", "xor.play", "sdfiles_toolkit"]
p2 = "compiled"

for path in modules:
    fol = os.listdir(path)
    for i in fol:
        p1 = os.path.join(path,i)
        p3 = 'cp -r ' + p1 +' ' + p2+'/.'
        sbp.Popen(p3,shell=True)

# Merges all modules together because I'm lazy ;^)