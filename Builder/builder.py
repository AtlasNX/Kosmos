#!/usr/bin/env python
"""Kosmos Builder v3.0.0

Usage:
    builder.py kosmos <output> [--version=<version>]
    builder.py sdsetup <output> [--version=<version>] [-a | --auto]
    builder.py (-h | --help)

Options:
    -h --help               Show this screen.
    --version=<version>     Overides the Kosmos Version from the config file.
    -a --auto               Perform an auto build.

"""

import config
from docopt import docopt
from modules import Modules
import os
import uuid
import shutil

def createTempDirectory():
    directory = f"{os.getcwd()}/tmp/{str(uuid.uuid4())}"
    os.makedirs(directory)
    return directory

def getKosmosVersion(args):
    if args["--version"] is not None:
        return args["--version"]
    return config.version

def initVersionMessages(args):
    if args["kosmos"]:
        return [ f"Kosmos {kosmos_version} built with:" ]
    elif args["sdsetup"] and not args["--auto"]:
        return [ "SDSetup Modules built with:" ]
    return []

def deleteFileOrFolder(path):
    if os.path.exists(path):
        if os.path.isfile(path):
            os.remove(path)
        else:
            shutil.rmtree(path)

if __name__ == "__main__":
    args = docopt(__doc__)

    temp_directory = createTempDirectory()
    kosmos_version = getKosmosVersion(args)
    version_messages = initVersionMessages(args)

    showPrompts = args["kosmos"] or (args["sdsetup"] and not args["--auto"])

    if showPrompts:
        print("Downloading Atmosphere...")
    Modules.download_atmosphere(temp_directory)

    if showPrompts:
        print("Downloading Hekate...")
    Modules.download_hekate(temp_directory, kosmos_version)

    if args["kosmos"] or (args["sdsetup"] and args["--auto"]):
        if showPrompts:
            print("Downloading Homebrew App Store...")
        Modules.download_appstore(temp_directory)

    if args["kosmos"] or (args["sdsetup"] and args["--auto"]):
        if showPrompts:
            print("Downloading EdiZon...")
        Modules.download_edizon(temp_directory)

    if showPrompts:
        print("Downloading Emuiibo...")
    Modules.download_emuiibo(temp_directory)

    if args["kosmos"] or (args["sdsetup"] and args["--auto"]):
        if showPrompts:
            print("Downloading Goldleaf...")
        Modules.download_goldleaf(temp_directory)

    if showPrompts:
        print("Downloading Kosmos Toolbox...")
    Modules.download_kosmos_toolbox(temp_directory)

    if showPrompts:
        print("Downloading Kosmos Updater...")
    Modules.download_kosmos_updater(temp_directory, kosmos_version)

    if showPrompts:
        print("Downloading ldn_mitm...")
    Modules.download_ldn_mitm(temp_directory)

    if args["kosmos"] or (args["sdsetup"] and args["--auto"]):
        if showPrompts:
            print("Downloading Lockpick...")
        Modules.download_lockpick(temp_directory)

    if showPrompts:
        print("Downloading Lockpick_RCM...")
    Modules.download_lockpick_rcm(temp_directory)

    if args["kosmos"]:
        print("Downloading nxdumptool...")
        Modules.download_nxdumptool(temp_directory)

    if args["kosmos"]:
        print("Downloading nx-ovlloader...")
        Modules.download_nx_ovlloader(temp_directory)

    if args["kosmos"]:
        print("Downloading ovlSysModules...")
        Modules.download_ovl_sysmodules(temp_directory)

    if showPrompts:
        print("Downloading sys-clk...")
    Modules.download_sys_clk(temp_directory)

    if showPrompts:
        print("Downloading sys-con...")
    Modules.download_sys_con(temp_directory)

    if showPrompts:
        print("Downloading sys-ftpd-light...")
    Modules.download_sys_ftpd_light(temp_directory)

    if args["kosmos"]:
        print("Downloading Tesla-Menu...")
        Modules.download_tesla_menu(temp_directory)

    deleteFileOrFolder(args["<output>"])

    if args["kosmos"]:
        shutil.make_archive(
            os.path.splitext(args["<output>"])[0],
            "zip",
            temp_directory)
    elif args["sdsetup"]:
        print("TODO")

    deleteFileOrFolder(f"{os.getcwd()}/tmp")

    for message in version_messages:
        print(message)
