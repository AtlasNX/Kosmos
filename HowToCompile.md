This little guide will help you to compile everything that is included in this pack. Let us start simple:

# Dependencies

- Install DevkitPro with Switch support
- Install needed packages:
    - **Linux:** ```sudo dkp-pacman -S devkitA64 libnx switch-tools switch-curl switch-bzip2 switch-curl switch-freetype switch-libjpeg-turbo switch-sdl2 switch-sdl2_gfx switch-sdl2_image switch-sdl2_ttf switch-zlib switch-libpng```
    - **Windows** ```pacman -S devkitA64 libnx switch-tools switch-curl switch-bzip2 switch-curl switch-freetype switch-libjpeg-turbo switch-sdl2 switch-sdl2_gfx switch-sdl2_image switch-sdl2_ttf switch-zlib switch-libpng```
        - You may have to run this command through Msys2 if it doesn't work
- A clean clone of the repo
    - For that simple ```git clone https://github.com/tumGER/SDFilesSwitch.git``` to your desired location
        - There are some weird issues on Windows if you have spaces somewhere in the path to your location so make sure to clone it somewhere where the Path doesn't include spaces


# Updating Dependencies

1. Open / Cd your console to the folder you cloned the repo to
2. Type ```git submodule update --remote --force``` to update the submodules
3. Success, the submodules should now be updated to the latest commit of their origin

# Updating LibNX to newest commit

Atmosphere often uses features that aren't included in the pacman libnx version yet so it's important to compile LibNX before compiling something else.

1. Go into the LibNX submodule folder
2. Type ```make```
3. Wait till it finished compiling
4. Type ```make install```
5. You now updated to newest commit LibNX

**Atmosphere sometimes even uses features that aren't even in LibNX yet, in that case follow the previous steps but use the [Atmosphere LibNX fork](https://github.com/Atmosphere-NX/libnx/tree/for-atmosphere) by typing ```git clone https://github.com/Atmosphere-NX/libnx.git -b for-atmosphere``` into your desired location**

# Updating Atmosphere's Loader, SM and LayeredFS

Atmosphere needs some custom changes to it to work with current hekate and also to compile. While we don't need to compile Atmosphere completely, we will in this tutorial because it's easier and breaks much less (in my experience)

1. Go into "SDFilesSwitch\AtmosphereCFW\fusee\fusee-primary\src" and find "main.c"
2. Type ```#define I_KNOW_WHAT_I_AM_DOING``` into line 14 (or similar)
    - This is needed to make it actually compile - It's basically a small warning that will hopefully be removed some day
3. Go into "SDFilesSwitch\AtmosphereCFW\stratosphere\loader\source" and find "ldr_main.cpp"
4. Remove
    ```cpp 
        /* Check for exosphere API compatibility. */
        u64 exosphere_cfg;
        if (R_FAILED(splGetConfig((SplConfigItem)65000, &exosphere_cfg))) {
            //fatalSimple(0xCAFE << 4 | 0xFF);
            /* TODO: Does Loader need to know about target firmware/master key revision? If so, extract from exosphere_cfg. */
        }
    ```
located ~ at line 72
    - This removes the Exosphere API Check from teh Loader
5. Go into "SDFilesSwitch\AtmosphereCFW\stratosphere\fs_mitm" and find "fsmitm_main.cpp"
6. Remove
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

7. Go back into the root of the AtmosphereCFW submodule ("SDFilesSwitch\AtmosphereCFW")
8. Type "make"
9. Wait a bit, this will take some time
10. Go back into "SDFilesSwitch\AtmosphereCFW\stratosphere\fs_mitm"
11. Type "make"
12. Wait a bit, this will take some time
13. Copy "loader.kip" from "SDFilesSwitch\AtmosphereCFW\stratosphere\loader" and "sm.kip" from "SDFilesSwitch\AtmosphereCFW\stratosphere\sm" into:
    - "SDFilesSwitch/Compiled/Hekate/modules/newfirm/"
    - "SDFilesSwitch/Compiled/LayeredFS/SDFiles/modules/LayeredFS/"
    - "SDFilesSwitch/Compiled/SwitchBlade/modules/newfirm/"
14. Copy "fs_mitm.kip" from "SDFilesSwitch\AtmosphereCFW\stratosphere\fs_mitm" into "SDFilesSwitch/Compiled/LayeredFS/SDFiles/modules/LayeredFS/"
15. Copy the updated files for Hekate, SwitchBlade and LayeredFS to your SD and test them on your Switch
    - If everything worked, congrats! You compiled Atmosphere :)
    - If it didn't, you either messed something up or Atmosphere had some change to its code in the time I wrote this till now - In that case I'd guess contacting me over Twitter (@_tomGER [Or @tumGER since I check that more often]) is your best bet if I'm gone - If you're here because you just wanted to compile it yourself than contact me over tomGER
    \#7462 on Discord.

# Updating other submodules

Here comes the easy part :) Updating all other submodules is really really easy and all follow nearly the same steps:

1. Go into the submodule folder
2. Type "make"
    - You need to type "make nx" for **hbmenu**
3. It should now have compiled it - The location is often pretty easy to find:
    - You'll get a .nro for EdiZon, AppstoreNX, GagOrder, SDFilesUpdater and hbmenu
    - You'll get a .bin from Hekate-LayeredFS
    - You'll get a .kip from dreport
4. Replace the old one in Hekate's, LayeredFS's and SwitchBlade's compiled folder

**Gag Order Icon:**

1. Start [NRO Asset Editor](https://github.com/vgmoose/nro-asset-editor/releases)
2. Choose your newly compiled Gag Order build
3. Choose the icon to be the one located at "SDFilesSwitch\resources" called GagOrder.png

![The Icon](/resources/GagOrder.png)

4. Save you changes
5. Replace the old one in Hekate's, LayeredFS's and SwitchBlade's compiled folder

# SwitchBlade (and Hekate) Splash Screen

![Example](/resources/Steins;Splash.png)
![Example 2](/resources/Splash-tomNX.png)
![Example 3](/resources/splash.png)

So here is the part where you can get creative and my most favorite part nowadays ("Graphic Design Is My Passion")

- Start your photo editor of choice (I use GIMP because I'm poor and actually kinda like GIMP more currently)
A. Open one of the three .xcf files and change the version number to the number of the release you want to make
B. Create your own one

1. I like theming them after the release name so if you call the release "Pikachu", make on that is themed after Pikachu
2. Create a new 1280 x 720 canvas
3. Use your graphic design passion
    - If you want to update the repo, make sure to include Credits, a link to the Repo, version number and all the other stuff you also see on my splash screens 
4. Export it as a .png
5. Use the [Image Converter](https://github.com/StevenMattera/SwitchBlade-ImageConverter/releases) to convert it to the .bin for SwitchBlade
6. Put it on the root of your SD / root of "SDFilesSwitch/Compiled/SwitchBlade"

    
