# Homebrew App Store [![travis](https://travis-ci.org/vgmoose/hb-appstore.svg?branch=master)](https://travis-ci.org/vgmoose/hb-appstore)
A graphical frontend to the [get package manager](https://github.com/vgmoose/libget) for downloading and managing homebrew on video game consoles, such as the Nintendo Switch. This is a replacement to the [Wii U Homebrew App Store](https://github.com/vgmoose/wiiu-hbas).

## Supported Platforms
### Nintendo Switch
To run this program, a Nintendo Switch with access to the Homebrew Menu is required (currently requires an sd card).
- [1.x-5.x hbmenu instructions](https://gbatemp.net/threads/switch-hacking-101-how-to-launch-the-homebrew-menu-on-4-x-5-x.504012/)
- [older 3.0.0 hbmenu instructions](https://switchbrew.github.io/nx-hbl/) -

A quick summary of how to run it, provided you can get to hbmenu, is also included below. For newer firmwares, see the link to the 4.x-5.x instructions above.

#### Quick summary
- download latest [appstore folder](https://github.com/vgmoose/hb-appstore/releases) to `sd:/switch/appstore/`
   - make sure that the `res` folder is next to `appstore.nro` inside of the `appstore` folder
- download latest [hbmenu.nro](https://github.com/switchbrew/nx-hbmenu/releases/latest) to `sd:/hbmenu.nro`
- put the SD card (recommended formatted FAT32) in the Switch, and enter hbmenu (see instructions above)
- run "hb App Store" from within hbmenu
   - when you're done hit home to exit (and album again to go back to hbmenu)

### Wii U
To run this program, a Wii U with access to the Homebrew Launcher is required. This can be done on any firmware. For more information, see the tutorial [here](https://wiiu.hacks.guide). For a stable release, see [here](https://github.com/vgmoose/wiiu-hbas/releases).

## Maintaining a repo
See [get's instructions](https://github.com/vgmoose/get#setting-up-repos) for setting up a repository. Everything is designed to be statically hosted. If there's no repo provided in the `repos.json` config file, then it will generate a default one pointing to [switchbru.com/appstore](http://switchbru.com/appstore/).

A new project called [Barkeep](https://github.com/vgmoose/barkeep) is being worked on to allow this to be done without using a simple script.

## Compilation instructions
This program is written using SDL2 and has dependencies on libsdl, libcurl, and zlib. It also makes use of libget which is included in this repo as a submodule.

### Building for Switch (with libnx)
1. Install [dkp-pacman](https://devkitpro.org/viewtopic.php?f=13&t=8702)
3. Install devkitA64 and needed Switch dependencies via dkp-pacman:
```
sudo dkp-pacman -S devkitA64 libnx switch-tools switch-curl switch-bzip2 switch-freetype switch-libjpeg-turbo switch-sdl2 switch-sdl2_gfx switch-sdl2_image switch-sdl2_ttf switch-zlib switch-libpng
```
3. Once it's all setup, recursively clone the repo and run make:
```
git clone --recursive https://github.com/vgmoose/hb-appstore.git
cd hb-appstore
make -f Makefile.switch
```

If all goes well, `appstore.nro` should be sitting in the current directory.

### Building for Wii U (with WUT)
[See here](https://github.com/vgmoose/hb-appstore/pull/19) for info on how to setup the Wii U environment. The Wii U build also makes use of the tinyxml library to look up HBL apps that may already be installed.

Once the environment is setup:
```
git clone --recursive https://github.com/vgmoose/hb-appstore.git
cd hb-appstore
make -f Makefile.wiiu clean
make -f Makefile.wiiu
```

### Building for PC
There's a separate makefile for building the SDL2 app for PC. Below instructions are for Ubuntu, but should be similar on other platforms:
```
sudo apt-get install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-gfx-dev zlib1g-dev gcc g++ git
git clone --recursive https://github.com/vgmoose/hb-appstore.git
cd hb-appstore
make -f Makefile.pc
```

### License
This software is licensed under the GPLv3.

#### Maintainers
- vgmoose
- rw-r-r_0644
