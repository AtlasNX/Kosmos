# This guide is outdated - A lot has changed and thus I only recommend reading it for historical purposes






# How To Compile SDFiles for Hekate
## The Automated Way™
Use [this](https://github.com/ThatNerdyPikachu/LaunchpadNX/releases/latest)!

## The Manual Way

Subject | Topic
--------|--------
**Preparations** | [Dependencies](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#dependencies)
. | [Updating Dependencies](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-dependencies)
. | [Installing/Updating LibNX](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#libnx)
**Atmosphere** | [Atmosphere with Patches](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-atmospheres-loader-sm-and-layeredfs-with-patches)
. | [Atmopshere without Patches ("Complete Atmosphere")](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-atmosphere-complete)
**Homebrew and Module** | [General](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-homebrew-and-modules)
. | [Homebrew Loader](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#homebrew-loader)
. | [Creport](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#creport)
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
- The Hekate/ and "Resources" folder from https://github.com/tumGER/SDFilesSwitch
    - Simply ```git clone https://github.com/tumGER/SDFilesSwitch.git``` to your desired location
        - We have to use the Hekate/ folder as some homebrew uses data that is otherwise not found when compiling it yourself

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

**A. Use the patch**
1. Copy the atmosphere.patch from "Resources\Patches"
2. cd into the Atmosphere submodule
3. Paste the script onto the root of that folder
4. Type ```patch -p1 < atmosphere.patch```

**B. Do it manually**
1. Go into "Atmosphere\stratosphere\loader\source" and find "ldr_main.cpp"
2. Remove
    ```
    cpp
        /* Check for exosphere API compatibility. */
        u64 exosphere_cfg;
        if (R_FAILED(splGetConfig((SplConfigItem)65000, &exosphere_cfg))) {
            //fatalSimple(0xCAFE << 4 | 0xFF);
            /* TODO: Does Loader need to know about target firmware/master key revision? If so, extract from exosphere_cfg. */
        }
    ```
    - This removes the Exosphere API Check from the Loader

4. In the same directory, find "ldr_npdm.cpp"
5. Find this function:
```
cpp
FILE *NpdmUtils::OpenNpdm(u64 title_id) {
    FILE *f_out = OpenNpdmFromSdCard(title_id);
    if (f_out != NULL) {
        return f_out;
    }
    return OpenNpdmFromExeFS();

    }
```
  and replace it by this one:
```
cpp
FILE *NpdmUtils::OpenNpdm(u64 title_id) {
    if (title_id == 0x010000000000100D) {
        Result rc;
        rc = hidInitialize();
        if (R_FAILED(rc)){
            fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));
        }
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if((kDown & KEY_R) == 0) {
          hidExit();
          FILE *f_out = OpenNpdmFromSdCard(title_id);
          if (f_out != NULL) {
           return f_out;
        }
          return OpenNpdmFromExeFS();
        }
        else {
         hidExit();
         return OpenNpdmFromExeFS();
        }
    }
    else {
    FILE *f_out = OpenNpdmFromSdCard(title_id);
    if (f_out != NULL) {
        return f_out;
    }
    return OpenNpdmFromExeFS();
    }
}
```
6. In the same directory, find "ldr_nso.cpp"
7. Find this function:
```
cpp
FILE *NsoUtils::OpenNso(unsigned int index, u64 title_id) {
    FILE *f_out = OpenNsoFromSdCard(index, title_id);
    if (f_out != NULL) {
        return f_out;
    } else if (CheckNsoStubbed(index, title_id)) {
        return NULL;
    } else {
        return OpenNsoFromExeFS(index);
    }
}
```
and replace it by this one:
```
cpp
FILE *NsoUtils::OpenNso(unsigned int index, u64 title_id) {
    if (title_id == 0x010000000000100D) {
        Result rc;
        rc = hidInitialize();
        if (R_FAILED(rc)){
            fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));
        }
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if((kDown & KEY_R) == 0) {
              hidExit();
              FILE *f_out = OpenNsoFromSdCard(index, title_id);
              if (f_out != NULL) {
                return f_out;
              } else if (CheckNsoStubbed(index, title_id)) {
               return NULL;
              } else {
               return OpenNsoFromExeFS(index);
              }
        }
        else {
            hidExit();
            return OpenNsoFromExeFS(index); }
    }
     else {        
    FILE *f_out = OpenNsoFromSdCard(index, title_id);
    if (f_out != NULL) {
        return f_out;
    } else if (CheckNsoStubbed(index, title_id)) {
        return NULL;
    } else {
        return OpenNsoFromExeFS(index);
    }
    }
}
```
8. Go into "Atmosphere\stratosphere\fs_mitm\source" and find "fsmitm_main.cpp"
9. Remove
```
cpp
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
10. Go into"Atmosphere\stratosphere\pm\source" and find "pm_main.cpp"
11. Remove
```
/* Check for exosphere API compatibility. */
u64 exosphere_cfg;
if (R_FAILED(splGetConfig((SplConfigItem)65000, &exosphere_cfg))) {
    fatalSimple(0xCAFE << 4 | 0xFF);
    /* TODO: Does PM need to know about target firmware/master key revision? If so, extract from exosphere_cfg. */
}
```

9. Go back into the root of the Atmosphere submodule
10. Type "make"
11. Wait a bit, this will take some time
12. Copy "loader.kip" from "Atmosphere\stratosphere\loader" into:
    - "SDFilesSwitch/Hekate/modules/newfirm/"
13. Copy "fs_mitm.kip" from "Atmosphere\stratosphere\fs_mitm" into:    
    - "SDFilesSwitch/Hekate/modules/newfirm/"
14. Copy "sm.kip" from "Atmosphere\stratosphere\sm" into:
    - "SDFilesSwitch/Hekate/modules/newfirm/"
15. Copy "pm.kip" from "Atmosphere\stratosphere\pm" into:
    - "SDFilesSwitch/Hekate/modules/newfirm/"
16. Copy the updated files to your SD and test them on your Switch
    - If everything worked, congrats! You compiled Atmosphere's patched Loader, SM and LayeredFS :)
    - If it didn't, you either messed something up or Atmosphere had some change to its code in the time I wrote this till now - In that case I'd guess contacting me over Twitter (@_tomGER [Or @tumGER since I check that more often]) is your best bet if I'm gone - If you're here because you just wanted to compile it yourself than contact me over tomGER
    \#7462 on Discord.

## Updating Atmosphere ("Complete")

**Make sure to [Update Dependencies](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-dependencies) before compiling Atmosphere**

**Make sure to [Update LibNX](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#libnx) before compiling Atmosphere**

**Atmosphere sometimes even uses features that aren't even in LibNX yet, in that case follow [the previous steps](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#libnx) but use the [Atmosphere LibNX fork](https://github.com/Atmosphere-NX/libnx/tree/for-atmosphere) by typing ```git clone https://github.com/Atmosphere-NX/libnx.git -b for-atmosphere``` into your desired location**

1. Apply steps 4-7 from [Updating Atmosphere](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-atmospheres-loader-sm-and-layeredfs-with-patches)
2. Type make on the root of the Atmosphere submodule
3. Copy "sm.kip" from "Atmosphere\stratosphere\sm", "fs_mitm.kip" from "Atmosphere\stratosphere\fs_mitm", "pm.kip" from "Atmosphere\stratosphere\pm", "loader.kip" from "Atmosphere\stratosphere\loader" and "exosphere.bin" from "Atmosphere/exosphere" into "Hekate/modules/atmosphere"

# Updating Homebrew and Modules

**Make sure to [Update Dependencies](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#updating-dependencies) before compiling submodules**

**Make sure to [Update LibNX](https://github.com/tumGER/SDFilesSwitch/blob/master/HowToCompile.md#libnx) before compiling submodules**

Here comes the easy part :) Updating all other submodules is really really easy and all follow nearly the same steps:

1. Go into each submodule folder
2. Type "make"
    - You need to type "make nx" for **hbmenu**
3. It should now have compiled it - The location is often pretty easy to find:
    - You'll get a .nro for EdiZon, AppstoreNX, SDFilesUpdater and hbmenu
    - You'll get a .kip from sys-ftpd
4. Replace the old one in the Hekate folder

**Some files are called slightly different in the compiled folder - Please rename your newly compiled files in that case!**

Submodule | Folder it has to go to
----------|-----------------------
EdiZon | Replace the .nacp and .nro in Hekate/switch/edizon
AppstoreNX | Replace the .nro in Hekate/switch/appstore
SDFilesUpdater | Replace the .nro in Hekate/switch/SDFilesUpdater
hbmenu | Replace the .nro on root
sys-ftpd | Replace the .kip in Hekate/modules


## **Homebrew Loader**

1. Go into the Homebrew Loader folder (hbl) and type ```make```
2. Copy hbl.nso to "Hekate/atmosphere/titles/010000000000100D/exefs/" and rename it to main (No extension, just main)
3. Copy hbl.npdm to "Hekate/atmosphere/titles/010000000000100D/exefs/" and rename it to main.npdm

## **Creport**

1. Go into atmosphere/stratosphere/creport
2. Type ```make```
3. Copy creport.nso to "Hekate\atmosphere\titles\0100000000000036\exefs" and rename it to main (No extension, just main)
4. Copy creport.npdm to "Hekate\atmosphere\titles\0100000000000036\exefs" and rename it to main.npdm

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
5. Put it on the root of your SD / root of "SDFilesSwitch/Hekate"
