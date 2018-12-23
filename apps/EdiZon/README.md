# EdiZon
<p align="center"><img src="https://raw.githubusercontent.com/thomasnet-mc/EdiZon/master/icon.jpg"><br />
	<a href="https://discord.gg/qyA38T8"><img src="https://discordapp.com/api/guilds/465980502206054400/embed.png" alt="Discord Server" /></a>
  <a href="https://travis-ci.com/WerWolv98/EdiZon"><img src="https://travis-ci.com/WerWolv98/EdiZon.svg?branch=master" alt="Build Status" /></a>
</p>

A Homebrew save file dumper, injector and on-console editor for Horizon, the OS of the Nintendo Switch.

# Images
<p align="center"><img src="https://puu.sh/BvBBc/584e11a744.jpg"></p>
<p align="center"><img src="https://puu.sh/BvBCI/0d1f1e334e.jpg"></p>

# EdiZon Editor Config and Script files

To download working Editor Config and Editor Script files, visit [this repository](https://github.com/WerWolv98/EdiZon_ConfigsAndScripts/tree/master)

Check out our [Wiki page](https://github.com/thomasnet-mc/EdiZon/wiki) for more information on how to build your own Editor Config and Editor Script files.

# How to compile

- The devkitPro toolchain with some dependencies of it are needed. Download it from here: https://devkitpro.org/
- To compile, libNX, libcurl, zlib and freetype2 are required. Download them by using pacman to download the switch-portlibs bundle, which includes them.
- The rest of the compilation works using the `make` command.

# Discord
If you need any help with the usage of EdiZon or if you want to start developing your own Editor Configs and Editor Scripts, feel free to join our Discord server: https://discord.gg/qyA38T8

# Credits
- [3096](https://github.com/3096) for [save dumping/injecting](https://github.com/3096/nut)
- [Bernardo Giordano](https://github.com/BernardoGiordano) for some [Checkpoint](https://github.com/BernardoGiordano/Checkpoint) code
- [SwitchBrew](https://switchbrew.org/) for the [Homebrew Launcher](https://github.com/switchbrew/nx-hbmenu) GUI and shared font code
- [thomasnet-mc](https://github.com/thomasnet-mc/) for most of the save backup and restore code and the updater script
- [trueicecold](https://github.com/trueicecold) for batch backups and the editable-only mode
- [onepiecefreak](https://github.com/onepiecefreak3) for the edizon debugger and LOTS of reviewing implementations
- [Jojo](https://github.com/drdrjojo) for the Travis CI configuration and the config creator 
- **All config creators** for bringing this project to life!
