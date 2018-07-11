Start your photo editor of choice (I use GIMP because I'm poor and actually kinda like GIMP)

A. Open one of the .xcf files and change the version number to the number of the release you want to make
B. Create your own one

1. Create a new 720 x 1280 canvas
    - Make sure you picture is roated 90° counterclockwise
2. Use your graphic design passion
    - If you want to update the repo, make sure to include Credits, a link to the Repo, version number and all the other stuff you also see on my splash screens 
3. Export it as a .png
4. Use some online converter to convert the .png to a .bmp
    - We need to use a online converter because GIMP currently has some issues with it
5. Call it bootlogo.bmp if you want to use it as the standard one
    - You can also choose a specific bootlogo for each launch option by writting ```logopath=LinkToBootimage.bmp``` into your hekate-ipl.ini ([Example](https://github.com/tumGER/SDFilesSwitch/blob/8653108d41096f60c43f5dab56b41024fa785c5d/Compiled/hekate_ipl.ini#L60))
5. Put it on the root of your SD / root of "SDFilesSwitch/Compiled"
