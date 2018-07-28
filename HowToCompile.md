How To Compile SDFiles for Hekate
=================================================================================
The unoffical offical guide by @tumGER

Subject | Topic
--------|--------
**Preparations** | [Dependencies](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#dependencies)
. | [Updating Dependencies](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-dependencies)
. | [Installing/Updating LibNX](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#libnx)
**Atmosphere** | [Atmosphere with Patches](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-atmospheres-loader-sm-and-layeredfs-with-patches)
. | [Atmopshere without Patches ("Complete Atmosphere")](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-atmosphere-complete)
**Homebrew and Module** | [General](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-homebrew-and-modules)
. | [Homebrew Loader](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#homebrew-loader)
. | [Adding custom Icons (GagOrder)](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#gag-order-icon)
**Extra** | [Bootlogos](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#hekate-bootlogo)

# Dependencies

- Install [DevkitPro](https://github.com/devkitPro/installer) with Switch support
- Install needed packages:
    - **Linux:** ```sudo dkp-pacman -S devkitA64 switch-tools switch-curl switch-bzip2 switch-freetype switch-libjpeg-turbo switch-sdl2 switch-sdl2_gfx switch-sdl2_image switch-sdl2_ttf switch-zlib switch-libpng```
    - **Windows** ```pacman -S devkitA64 switch-tools switch-curl switch-bzip2 switch-freetype switch-libjpeg-turbo switch-sdl2 switch-sdl2_gfx switch-sdl2_image switch-sdl2_ttf switch-zlib switch-libpng```
        - You may have to run this command through Msys2 if it doesn't work
- A clean clone of the compiler repo - **This repo holds all submodules mentioned in this guide!**
    - For that simply ```git clone https://github.com/tumGER/SDFilesCompiler.git``` to your desired location
        - There are some weird issues on Windows if you have spaces somewhere in the path to your location so make sure to clone it somewhere where the Path doesn't include spaces
- [LibNX](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#libnx)
- The "Compiled" and "Resources" folder from https://github.com/tumGER/SDFilesSwitch
    - Simply ```git clone https://github.com/tumGER/SDFilesSwitch.git``` to your desired location
        - We have to use the "compiled" folder as some homebrew uses data that is otherwise not found when compiling it yourself

# Updating Dependencies

1. Open your console in the root of the repo you just cloned
2. Type ```git submodule update --remote --force``` to update the submodules
3. Success, the submodules should now be updated to the latest commit of their origin

# LibNX

**Make sure to [Update Dependencies](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-dependencies) before updating LibNX**

1. Go into the LibNX submodule folder
2. Type ```make```
3. Wait till it finished compiling
4. Type ```make install```

# Atmosphere

## Updating Atmosphere's Loader, SM and LayeredFS with Patches

**Atmosphere sometimes even uses features that aren't even in LibNX yet, in that case follow the previous steps but use the [Atmosphere LibNX fork](https://github.com/Atmosphere-NX/libnx/tree/for-atmosphere) by typing ```git clone https://github.com/Atmosphere-NX/libnx.git -b for-atmosphere``` into your desired location**

**Make sure to [Update Dependencies](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-dependencies) before compiling Atmosphere**

**Make sure to [Update LibNX](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#libnx) before compiling Atmosphere**

Atmosphere needs some custom changes to it to work with current hekate and also to compile. While we don't need to compile Atmosphere completely, we will in this tutorial because it's easier and breaks much less (in my experience)

1. Go into "Atmosphere\stratosphere\loader\source" and find "ldr_main.cpp"
2. Remove
    ```cpp 
        /* Check for exosphere API compatibility. */
        u64 exosphere_cfg;
        if (R_FAILED(splGetConfig((SplConfigItem)65000, &exosphere_cfg))) {
            //fatalSimple(0xCAFE << 4 | 0xFF);
            /* TODO: Does Loader need to know about target firmware/master key revision? If so, extract from exosphere_cfg. */
        }
    ```
located ~ at line 72
    - This removes the Exosphere API Check from the Loader
    
3. Go into "Atmosphere\stratosphere\fs_mitm" and find "fsmitm_main.cpp"
4. Remove
```cpp
    /* Check for exosphere API compatibility. */
    u64 exosphere_cfg;
    if (R_SUCCEEDED(splGetConfig((SplConfigItem)65000, &exosphere_cfg))) {
        /* MitM requires Atmosphere API 0.1. */
        u16 api_version = (exosphere_cfg >> 16) & 0xFFFF;
        if (api_version < 0x0001) {
            fatalSimple(0xCAFE << 4 | 0xFE);
        }
    } else {
        fatalSimple(0xCAFE << 4 | 0xFF);
    }
```
located ~ at line 67

5. Go back into the root of the Atmosphere submodule
6. Type "make"
7. Wait a bit, this will take some time
8. Go back into "Atmosphere\stratosphere\fs_mitm"
9. Type "make"
10. Wait a bit, this will take some time
11. Copy "loader.kip" from "Atmosphere\stratosphere\loader" into:
    - "SDFilesSwitch/Compiled/modules/newfirm/"
12. Copy "fs_mitm.kip" from "Atmosphere\stratosphere\fs_mitm" into:    
    - "SDFilesSwitch/Compiled/modules/newfirm/"
13. Copy "sm.kip" from "Atmosphere\stratosphere\sm" into:
    - "SDFilesSwitch/Compiled/modules/newfirm/"
14. Copy the updated files to your SD and test them on your Switch
    - If everything worked, congrats! You compiled Atmosphere's patched Loader, SM and LayeredFS :)
    - If it didn't, you either messed something up or Atmosphere had some change to its code in the time I wrote this till now - In that case I'd guess contacting me over Twitter (@_tomGER [Or @tumGER since I check that more often]) is your best bet if I'm gone - If you're here because you just wanted to compile it yourself than contact me over tomGER
    \#7462 on Discord.

## Updating Atmosphere ("Complete")

**Make sure to [Update Dependencies](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-dependencies) before compiling Atmosphere**

**Make sure to [Update LibNX](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#libnx) before compiling Atmosphere**

**Atmosphere sometimes even uses features that aren't even in LibNX yet, in that case follow [the previous steps](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#libnx) but use the [Atmosphere LibNX fork](https://github.com/Atmosphere-NX/libnx/tree/for-atmosphere) by typing ```git clone https://github.com/Atmosphere-NX/libnx.git -b for-atmosphere``` into your desired location**

1. Go into "Atmosphere\fusee\fusee-primary\src" and find "main.c"
2. Type make on the root of the Atmosphere submodule
3. Copy "sm.kip" from "Atmosphere\stratosphere\sm", "fs_mitm.kip" from "Atmosphere\stratosphere\fs_mitm", "loader.kip" from "Atmosphere\stratosphere\loader" and "exosphere.bin" from "Atmosphere/exosphere" into "Compiled/modules/atmosphere"

# Updating Homebrew and Modules

**Make sure to [Update Dependencies](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-dependencies) before compiling submodules**

**Make sure to [Update LibNX](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#libnx) before compiling submodules**

Here comes the easy part :) Updating all other submodules is really really easy and all follow nearly the same steps:

1. Go into each submodule folder
2. Type "make"
    - You need to type "make nx" for **hbmenu**
3. It should now have compiled it - The location is often pretty easy to find:
    - You'll get a .nro for EdiZon, AppstoreNX, GagOrder, SDFilesUpdater and hbmenu
    - You'll get a .kip from dreport, sys-ftpd
4. Replace the old one in the compiled folder

**Some files are called slightly different in the compiled folder - Please rename your newly compiled files in that case!**

Submodule | Folder it has to go to
----------|-----------------------
EdiZon | Replace the .nacp and .nro in compiled/switch/edizon
AppstoreNX | Replace the .nro in compiled/switch/appstore
GagOrder | Replace the .nro in compiled/switch
SDFilesUpdater | Replace the .nro in compiled/switch/SDFilesUpdater
hbmenu | Replace the .nro on root
dreport | Replace the .kip in compiled/modules
sys-ftpd | Replace the .kip in compiled/modules


## **Homebrew Loader**

1. Go into the Homebrew Loader folder (hbl) and type ```make```
2. Copy hbl.nsp to "compiled/atmosphere/titles/010000000000100D/exefs/" and rename it to main (No extension, just main)
3. Copy hbl.npdm to "compiled/atmosphere/titles/010000000000100D/exefs/" and rename it to main.npdm


## **Gag Order Icon:**

1. Start [NRO Asset Editor](https://github.com/vgmoose/nro-asset-editor/releases)
2. Choose your newly compiled Gag Order build
3. Choose the icon to be the one located at "SDFilesSwitch\resources\icons" called GagOrder.png

![The Icon](/Resources/Icons/GagOrder.png)

4. Save you changes
5. Replace the old one in the compiled folder

# Hekate Bootlogo

- Start your photo editor of choice (I use GIMP because I'm poor and actually kinda like GIMP)
A. Open one of the three .xcf files and change the version number to the number of the release you want to make
B. Create your own one

1. Create a new 720 x 1280 canvas
    - Make sure you picture is roated 90° counterclockwise
2. Use your graphic design passion
    - If you want to update the repo, make sure to include Credits, a link to the Repo, version number and all the other stuff you also see on my splash screens 
3. Export it as a .png
4. Use some online converter to convert the .png to a .bmp
    - We need to use a online converter because GIMP currently has some issues with it
5. Call it bootlogo.bmp if you want to use it as the standard one
    - You can also choose a specific bootlogo for each launch option by writting ```logopath=/Link/To/Bootimage.bmp``` into your hekate-ipl.ini ([Example](https://github.com/tumGER/SDFilesSwitch/blob/8653108d41096f60c43f5dab56b41024fa785c5d/Compiled/hekate_ipl.ini#L60))
5. Put it on the root of your SD / root of "SDFilesSwitch/Compiled"

    
