#include "common.h"

void menuEntryInit(menuEntry_s* me, MenuEntryType type) {
    memset(me, 0, sizeof(*me));
    me->type = type;
}

void menuEntryFree(menuEntry_s* me, bool skip_icongfx) {
    me->icon_size = 0;
    if (me->icon) {
        free(me->icon);
        me->icon = NULL;
    }

    if (!skip_icongfx) {
        if (me->icon_gfx) {
            free(me->icon_gfx);
            me->icon_gfx = NULL;
        }

        if (me->icon_gfx_small) {
            free(me->icon_gfx_small);
            me->icon_gfx_small = NULL;
        }
    }

    if (me->nacp) {
        free(me->nacp);
        me->nacp = NULL;
    }
}

bool fileExists(const char* path) {
    struct stat st;
    return stat(path, &st)==0 && S_ISREG(st.st_mode);
}

bool fsobjExists(const char* path) {
    struct stat st;
    return stat(path, &st)==0;
}

static bool menuEntryLoadEmbeddedIcon(menuEntry_s* me) {
    NroHeader header;
    NroAssetHeader asset_header;

    FILE* f = fopen(me->path, "rb");
    if (!f) return false;

    fseek(f, sizeof(NroStart), SEEK_SET);
    if (fread(&header, sizeof(header), 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    fseek(f, header.size, SEEK_SET);

    if (fread(&asset_header, sizeof(asset_header), 1, f) != 1
        || asset_header.magic != NROASSETHEADER_MAGIC
        || asset_header.version > NROASSETHEADER_VERSION
        || asset_header.icon.offset == 0
        || asset_header.icon.size == 0)
    {
        fclose(f);
        return false;
    }

    me->icon_size = asset_header.icon.size;
    me->icon = (uint8_t*)malloc(me->icon_size);
    if (me->icon == NULL) {
        fclose(f);
        return false;
    }
    memset(me->icon, 0, me->icon_size);

    fseek(f, header.size + asset_header.icon.offset, SEEK_SET);
    bool ok = fread(me->icon, me->icon_size, 1, f) == 1;
    fclose(f);
    return ok;
}

static bool menuEntryLoadExternalIcon(menuEntry_s* me, const char* path) {
    struct stat st;

    if(stat(path, &st)==-1) return false;

    FILE* f = fopen(path, "rb");
    if (!f) return false;

    me->icon_size = st.st_size;
    me->icon = (uint8_t*)malloc(me->icon_size);
    if (me->icon == NULL) {
        fclose(f);
        return false;
    }
    memset(me->icon, 0, me->icon_size);

    bool ok = fread(me->icon, me->icon_size, 1, f) == 1;
    fclose(f);
    return ok;
}

static bool menuEntryImportIconGfx(menuEntry_s* me, uint8_t* icon_gfx, uint8_t* icon_gfx_small) {
    size_t tmpsize;

    if (icon_gfx == NULL || icon_gfx_small == NULL) return false;

    tmpsize = 256*256*3;
    me->icon_gfx = (uint8_t*)malloc(tmpsize);
    if (me->icon_gfx) memcpy(me->icon_gfx, icon_gfx, tmpsize);

    if (me->icon_gfx) {
        tmpsize = 140*140*3;
        me->icon_gfx_small = (uint8_t*)malloc(tmpsize);
        if (me->icon_gfx_small) memcpy(me->icon_gfx_small, icon_gfx_small, tmpsize);

        if (me->icon_gfx_small == NULL) {
            free(me->icon_gfx);
            me->icon_gfx = NULL;
        }
    }

    return me->icon_gfx && me->icon_gfx_small;
}

static bool menuEntryLoadEmbeddedNacp(menuEntry_s* me) {
    NroHeader header;
    NroAssetHeader asset_header;

    FILE* f = fopen(me->path, "rb");
    if (!f) return false;

    fseek(f, sizeof(NroStart), SEEK_SET);
    if (fread(&header, sizeof(header), 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    fseek(f, header.size, SEEK_SET);

    if (fread(&asset_header, sizeof(asset_header), 1, f) != 1
        || asset_header.magic != NROASSETHEADER_MAGIC
        || asset_header.version > NROASSETHEADER_VERSION
        || asset_header.nacp.offset == 0
        || asset_header.nacp.size == 0)
    {
        fclose(f);
        return false;
    }

    if (asset_header.nacp.size < sizeof(NacpStruct))
    {
        fclose(f);
        return false;
    }

    me->nacp = (NacpStruct*)malloc(sizeof(NacpStruct));
    if (me->nacp == NULL) {
        fclose(f);
        return false;
    }

    fseek(f, header.size + asset_header.nacp.offset, SEEK_SET);
    bool ok = fread(me->nacp, sizeof(NacpStruct), 1, f) == 1;
    fclose(f);
    return ok;
}

/*static void fixSpaceNewLine(char* buf) {
    char *outp = buf, *inp = buf;
    char lastc = 0;
    do
    {
        char c = *inp++;
        if (c == ' ' && lastc == ' ')
            outp[-1] = '\n';
        else
            *outp++ = c;
        lastc = c;
    } while (lastc);
}*/

bool menuEntryLoad(menuEntry_s* me, const char* name, bool shortcut) {
    int i=0, tmplen;
    menu_s *menu_fileassoc = menuFileassocGetCurrent();
    menuEntry_s* fileassoc_me = NULL;
    char *strptr = NULL;
    static char tempbuf[PATH_MAX+1];
    //bool isOldAppFolder = false;

    if (!fsobjExists(me->path)) return false;

    tempbuf[PATH_MAX] = 0;
    strcpy(me->name, name);
    
    if (me->type == ENTRY_TYPE_FOLDER)
    {
        //Check for <dirpath>/<dirname>.nro
        snprintf(tempbuf, sizeof(tempbuf)-1, "%.*s/%.*s.nro", (int)sizeof(tempbuf)/2, me->path, (int)sizeof(tempbuf)/2-7, name);
        bool found = fileExists(tempbuf);
        bool fileassoc_flag = 0;

        //Use the first .nro found in the directory, if there's only 1 NRO in the directory. Only used for paths starting with "sdmc:/switch/".
        tmplen = strlen(menuGetRootPath());
        if (!found && strncmp(me->path, menuGetRootPath(), tmplen)==0 && me->path[tmplen]=='/') {
            DIR* dir;
            struct dirent* dp;
            u32 nro_count=0;

            dir = opendir(me->path);
            if (dir) {
                while ((dp = readdir(dir))) {
                    if (dp->d_name[0]=='.')//Check this here so that it's consistent with menuScan().
                        continue;

                    const char* ext = getExtension(dp->d_name);
                    if (strcasecmp(ext, ".nro")==0) {
                        nro_count++;
                        if (nro_count>1) {
                            found = 0;
                            break;
                        }

                        snprintf(tempbuf, sizeof(tempbuf)-1, "%.*s/%.*s", (int)sizeof(tempbuf)/2, me->path, (int)sizeof(tempbuf)/2-7, dp->d_name);
                        found = fileExists(tempbuf);
                    }
                }
                closedir(dir);
            }

            if (!found && menu_fileassoc->nEntries > 0) {
                fileassoc_flag = 1;
                dir = opendir(me->path);
                if (dir) {
                    while ((dp = readdir(dir))) {
                        if (dp->d_name[0]=='.')//Check this here so that it's consistent with menuScan().
                            continue;

                        for (fileassoc_me = menu_fileassoc->firstEntry, i = 0; fileassoc_me; fileassoc_me = fileassoc_me->next, i++) {
                            if (!fileassoc_me->fileassoc_type) continue; //Only handle fileassoc entries for filenames, not file_extensions.

                            if (strcmp(dp->d_name, fileassoc_me->fileassoc_str)) continue;

                            snprintf(tempbuf, sizeof(tempbuf)-1, "%.*s/%.*s", (int)sizeof(tempbuf)/2, me->path, (int)sizeof(tempbuf)/2-7, dp->d_name);
                            found = fileExists(tempbuf);
                            if (found) break;
                        }
                        if (found) break;
                    }
                   closedir(dir);
                }
            }
        }

        if (found)
        {
            //isOldAppFolder = true;
            shortcut = false;
            me->type = fileassoc_flag ? ENTRY_TYPE_FILE_OTHER : ENTRY_TYPE_FILE;
            strcpy(me->path, tempbuf);
        }
    }

    if (me->type == ENTRY_TYPE_FILE)
    {
        //strcpy(me->name, name);//This is already done before both if statements
        strcpy(me->author, textGetString(StrId_DefaultPublisher));
        strcpy(me->version, "1.0.0");

        //shortcut_s sc;

        /*if (shortcut)
        {
            if (R_FAILED(shortcutCreate(&sc, me->path)))
                return false;
            if (!fileExists(sc.executable))
            {
                shortcutFree(&sc);
                return false;
            }
            strcpy(me->path, "sdmc:");
            strcat(me->path, sc.executable);
        }*/

        bool iconLoaded = false;

        // Load the icon
        /*if (shortcut)
        {
            FILE* f = sc.icon ? fopen(sc.icon, "rb") : NULL;
            if (f)
            {
                iconLoaded = fread(&me->smdh, sizeof(smdh_s), 1, f) == 1;
                fclose(f);
            }
        }*/

        if (!iconLoaded) do
        {
            // Attempt loading external icon
            /*strcpy(tempbuf, me->path);
            char* ext = getExtension(tempbuf);

            strcpy(ext, ".jpg");
            iconLoaded = menuEntryLoadExternalIcon(me, tempbuf);
            if (iconLoaded) break;

            if (isOldAppFolder)
            {
                char* slash = getSlash(tempbuf);

                strcpy(slash, "/icon.jpg");
                iconLoaded = menuEntryLoadExternalIcon(me, tempbuf);
                if (iconLoaded) break;
            }*/

            // Attempt loading the embedded icon
            if (!shortcut)
                iconLoaded = menuEntryLoadEmbeddedIcon(me);
        } while (0);

        if (iconLoaded)
        {
            menuEntryParseIcon(me);
        }

        bool nacpLoaded = false;

        nacpLoaded = menuEntryLoadEmbeddedNacp(me);

        if (nacpLoaded)
        {
            menuEntryParseNacp(me);

            // Fix description for some applications using multiple spaces to indicate newline
            //fixSpaceNewLine(me->description);
        }

        // Metadata overrides for shortcuts
        /*if (shortcut)
        {
            if (sc.name) strncpy(me->name, sc.name, ENTRY_NAMELENGTH);
            if (sc.description) strncpy(me->description, sc.description, ENTRY_DESCLENGTH);
            if (sc.author) strncpy(me->author, sc.author, ENTRY_AUTHORLENGTH);
        }*/

        // Load the descriptor
        /*if (shortcut && sc.descriptor && fileExists(sc.descriptor))
            descriptorLoad(&me->descriptor, sc.descriptor);
        else
        {
            strcpy(tempbuf, me->path);
            strcpy(getExtension(tempbuf), ".xml");
            bool found = fileExists(tempbuf);
            if (!found && isOldAppFolder)
            {
                strcpy(tempbuf, me->path);
                strcpy(getSlash(tempbuf), "/descriptor.xml");
                found = fileExists(tempbuf);
            }
            if (found)
                descriptorLoad(&me->descriptor, tempbuf);
        }*/

        // Initialize the argument data
        argData_s* ad = &me->args;
        ad->dst = (char*)&ad->buf[1];
        launchAddArg(ad, me->path);

        // Load the argument(s) from the shortcut
        /*if (shortcut && sc.arg && *sc.arg)
            launchAddArgsFromString(ad, sc.arg);*/

        /*if (shortcut)
            shortcutFree(&sc);*/
    }
    
    if (me->type == ENTRY_TYPE_THEME) {
        config_t cfg = {0};
        config_init(&cfg);
        config_setting_t *themeInfo;
        const char *name,
                   *author = textGetString(StrId_DefaultPublisher),
                   *version = "1.0.0";
                   
        if (config_read_file(&cfg, me->path)) {
            themeInfo = config_lookup(&cfg, "themeInfo");
            if (themeInfo != NULL) {
                if(config_setting_lookup_string(themeInfo, "name", &name))
                    strncpy(me->name, name, sizeof(me->name)-1);
                config_setting_lookup_string(themeInfo, "author", &author);
                config_setting_lookup_string(themeInfo, "version", &version);
            }
        }

        strncpy(me->author, author, sizeof(me->author)-1);
        strncpy(me->version, version, sizeof(me->version)-1);
        config_destroy(&cfg);
    }

    if (me->type == ENTRY_TYPE_FILE_OTHER)
    {
        if (menu_fileassoc->nEntries == 0) return false;

        for (fileassoc_me = menu_fileassoc->firstEntry, i = 0; fileassoc_me; fileassoc_me = fileassoc_me->next, i++) {
            //For fileassoc_type==0 compare the extension, otherwise compare the filename.
            if (!fileassoc_me->fileassoc_type) {
                strptr = getExtension(me->path);
            }
            if (fileassoc_me->fileassoc_type) {
                strptr = getSlash(me->path);
                if (strptr[0] == '/') strptr++;
            }

            if (strcmp(strptr, fileassoc_me->fileassoc_str)) continue;

            //At this point a match was found.

            me->type = ENTRY_TYPE_FILE;

            //Attempt to load the icon from {me->path filepath with extension .jpg}, then on failure use the icon data from fileassoc_me.
            memset(tempbuf, 0, sizeof(tempbuf));
            strncpy(tempbuf, me->path, sizeof(tempbuf));
            tempbuf[sizeof(tempbuf)-1] = 0;
            strptr = getExtension(tempbuf);
            strncpy(strptr, ".jpg", sizeof(tempbuf)-1 - ((ptrdiff_t)strptr - (ptrdiff_t)tempbuf));

            bool iconLoaded = false;

            iconLoaded = menuEntryLoadExternalIcon(me, tempbuf);

            if (iconLoaded) menuEntryParseIcon(me);

            if (iconLoaded && !(me->icon_gfx && me->icon_gfx_small)) iconLoaded = 0;

            if (!iconLoaded && fileassoc_me->icon_gfx && fileassoc_me->icon_gfx_small)
                iconLoaded = menuEntryImportIconGfx(me, fileassoc_me->icon_gfx, fileassoc_me->icon_gfx_small);

            strncpy(me->author, fileassoc_me->author, sizeof(me->author));
            me->author[sizeof(me->author)-1] = 0;

            strncpy(me->version, fileassoc_me->version, sizeof(me->version));
            me->version[sizeof(me->version)-1] = 0;

            // Initialize the argument data
            argData_s* ad = &me->args;
            ad->dst = (char*)&ad->buf[1];
            launchAddArg(ad, fileassoc_me->path);
            launchAddArg(ad, me->path);

            strncpy(me->path, fileassoc_me->path, sizeof(me->path));
            me->path[sizeof(me->path)-1] = 0;

            return true;
        }
        return false;
    }

    return true;
}

void menuEntryFileassocLoad(const char* filepath) {
    bool success=0, success2=0;
    menuEntry_s* me = NULL;

    config_setting_t *fileassoc = NULL, *targets = NULL, *target = NULL;
    config_t cfg = {0};
    int targets_len=0, i;
    const char *strptr = NULL;

    char app_path[PATH_MAX+8];
    char main_icon_path[PATH_MAX+1];
    char target_icon_path[PATH_MAX+1];
    char target_file_extension[PATH_MAX+1];
    char target_filename[PATH_MAX+1];

    char app_author[ENTRY_AUTHORLENGTH+1];
    char app_version[ENTRY_VERLENGTH+1];

    uint8_t *app_icon_gfx = NULL;
    uint8_t *app_icon_gfx_small = NULL;

    config_init(&cfg);

    memset(app_path, 0, sizeof(app_path));
    memset(main_icon_path, 0, sizeof(main_icon_path));
    memset(app_author, 0, sizeof(app_author));
    memset(app_version, 0, sizeof(app_version));

    if (!fileExists(filepath)) return;

    if (config_read_file(&cfg, filepath)) {
        fileassoc = config_lookup(&cfg, "fileassoc");

        if (fileassoc != NULL) {
            if (config_setting_lookup_string(fileassoc, "app_path", &strptr))
                snprintf(app_path, sizeof(app_path)-1, "%s%s", menuGetRootBasePath(), strptr);
            if (config_setting_lookup_string(fileassoc, "icon_path", &strptr))
                snprintf(main_icon_path, sizeof(main_icon_path)-1, "%s%s", menuGetRootBasePath(), strptr);
            targets = config_setting_get_member(fileassoc, "targets");

            if (app_path[0] && targets) {
                targets_len = config_setting_length(targets);

                if (targets_len > 0) {
                    //Load the author/version and icon data with the NRO app path.
                    me = menuCreateEntry(ENTRY_TYPE_FILE);
                    success = 0;
                    if (me) {
                        strncpy(me->path, app_path, sizeof(me->path)-1);
                        me->path[sizeof(me->path)-1] = 0;

                        strptr = getSlash(app_path);
                        if(strptr[0] == '/') strptr++;

                        if (menuEntryLoad(me, strptr, 0)) {
                            strncpy(app_author, me->author, sizeof(app_author));
                            app_author[sizeof(app_author)-1] = 0;
                            strncpy(app_version, me->version, sizeof(app_version));
                            app_version[sizeof(app_version)-1] = 0;
                            app_icon_gfx = me->icon_gfx;
                            app_icon_gfx_small = me->icon_gfx_small;
                            success = 1;
                        }

                        menuDeleteEntry(me, success);
                        me = NULL;
                    }

                    //Process the targets list.
                    if (success) {
                        for (i=0; i<targets_len; i++) {
                            target = config_setting_get_elem(targets, i);
                            if (target == NULL) continue;

                            memset(target_icon_path, 0, sizeof(target_icon_path));
                            memset(target_file_extension, 0, sizeof(target_file_extension));
                            memset(target_filename, 0, sizeof(target_filename));

                            if (config_setting_lookup_string(target, "icon_path", &strptr))
                                snprintf(target_icon_path, sizeof(target_icon_path)-1, "%s%s", menuGetRootBasePath(), strptr);
                            if (config_setting_lookup_string(target, "file_extension", &strptr))
                                strncpy(target_file_extension, strptr, sizeof(target_file_extension)-1);
                            if (config_setting_lookup_string(target, "filename", &strptr))
                                strncpy(target_filename, strptr, sizeof(target_filename)-1);

                            //string_is_set for target_file_extension and target_filename must differ: only 1 can be set, not both set or both not set.
                            if ((target_file_extension[0]!=0) == (target_filename[0]!=0)) continue;

                            me = menuCreateEntry(ENTRY_TYPE_FILEASSOC);
                            success2 = 0;

                            if (me) {
                                strncpy(me->path, app_path, sizeof(me->path));
                                me->path[sizeof(me->path)-1] = 0;
                                strncpy(me->author, app_author, sizeof(me->author));
                                me->author[sizeof(me->author)-1] = 0;
                                strncpy(me->version, app_version, sizeof(me->version));
                                me->version[sizeof(me->version)-1] = 0;

                                if (target_file_extension[0]) {
                                    me->fileassoc_type = 0;
                                    strncpy(me->fileassoc_str, target_file_extension, sizeof(me->fileassoc_str));
                                } else if (target_filename[0]) {
                                    me->fileassoc_type = 1;
                                    strncpy(me->fileassoc_str, target_filename, sizeof(me->fileassoc_str));
                                }
                                me->fileassoc_str[sizeof(me->fileassoc_str)-1] = 0;

                                if (target_icon_path[0]) success2 = menuEntryLoadExternalIcon(me, target_icon_path);
                                if (!success2 && main_icon_path[0]) success2 = menuEntryLoadExternalIcon(me, main_icon_path);

                                if (success2) {
                                    menuEntryParseIcon(me);
                                } else {
                                    success2 = menuEntryImportIconGfx(me, app_icon_gfx, app_icon_gfx_small);
                                }
                            }

                            if (me) {
                                if (success2)
                                    menuFileassocAddEntry(me);
                                else
                                    menuDeleteEntry(me, 0);
                            }
                        }
                    }
                }
            }
        }
    }

    if (success) {
        free(app_icon_gfx);
        free(app_icon_gfx_small);
    }

    config_destroy(&cfg);
}

void menuEntryParseIcon(menuEntry_s* me) {
    if (me->icon_size==0 || me->icon==NULL) return;

    int w,h,samp;
    size_t imagesize = 256*256*3;
    me->icon_gfx = (uint8_t*)malloc(imagesize);

    if (me->icon_gfx == NULL) {
        me->icon_size = 0;
        free(me->icon);
        me->icon = NULL;
        return;
    }

    tjhandle _jpegDecompressor = tjInitDecompress();

    if (_jpegDecompressor == NULL) {
        free(me->icon_gfx);
        me->icon_gfx = NULL;

        me->icon_size = 0;
        free(me->icon);
        me->icon = NULL;
        return;
    }

    if (tjDecompressHeader2(_jpegDecompressor, me->icon, me->icon_size, &w, &h, &samp) == -1) {
        free(me->icon_gfx);
        me->icon_gfx = NULL;

        me->icon_size = 0;
        free(me->icon);
        me->icon = NULL;
        tjDestroy(_jpegDecompressor);
        return;
    }

    if (w != 256 || h != 256 ) return;

    if (tjDecompress2(_jpegDecompressor, me->icon, me->icon_size, me->icon_gfx, w, 0, h, TJPF_RGB, TJFLAG_ACCURATEDCT) == -1) {
        free(me->icon_gfx);
        me->icon_gfx = NULL;

        me->icon_size = 0;
        free(me->icon);
        me->icon = NULL;
        tjDestroy(_jpegDecompressor);
        return;
    }

    me->icon_size = 0;
    free(me->icon);
    me->icon = NULL;

    tjDestroy(_jpegDecompressor);

    me->icon_gfx_small = downscaleImg(me->icon_gfx, 256, 256, 140, 140, IMAGE_MODE_RGB24);

    if (me->icon_gfx_small == NULL) {
        free(me->icon_gfx);
        me->icon_gfx = NULL;
    }
}

uint8_t *downscaleImg(const uint8_t *image, int srcWidth, int srcHeight, int destWidth, int destHeight, ImageMode mode) {
    uint8_t *out;

    switch (mode) {
        case IMAGE_MODE_RGBA32:
            out = (uint8_t*)malloc(destWidth*destHeight*4);
            break;

        default:
            out = (uint8_t*)malloc(destWidth*destHeight*3);
            break;
    }

    if (out == NULL) {
        return NULL;
    }

    int tmpx, tmpy;
    int pos;
    float sourceX, sourceY;
    float xScale = (float)srcWidth / (float)destWidth;
    float yScale = (float)srcHeight / (float)destHeight;
    int pixelX, pixelY;
    uint8_t r1, r2, r3, r4;
    uint8_t g1, g2, g3, g4;
    uint8_t b1, b2, b3, b4;
    uint8_t a1, a2, a3, a4;
    float fx, fy, fx1, fy1;
    int w1, w2, w3, w4;

    for (tmpx=0; tmpx<destWidth; tmpx++) {
        for (tmpy=0; tmpy<destHeight; tmpy++) {
            sourceX = tmpx * xScale;
            sourceY = tmpy * yScale;
            pixelX = (int)sourceX;
            pixelY = (int)sourceY;

            // get colours from four surrounding pixels
            if (mode == IMAGE_MODE_RGBA32) 
                pos = ((pixelY + 0) * srcWidth + pixelX + 0) * 4;
            else
                pos = ((pixelY + 0) * srcWidth + pixelX + 0) * 3;

            r1 = image[pos+0];
            g1 = image[pos+1];
            b1 = image[pos+2];
            
            if (mode == IMAGE_MODE_RGBA32) 
                a1 = image[pos+3];


            if (mode == IMAGE_MODE_RGBA32) 
                pos = ((pixelY + 0) * srcWidth + pixelX + 1) * 4;
            else
                pos = ((pixelY + 0) * srcWidth + pixelX + 1) * 3;

            r2 = image[pos+0];
            g2 = image[pos+1];
            b2 = image[pos+2];

            if (mode == IMAGE_MODE_RGBA32) 
                a2 = image[pos+3];


            if (mode == IMAGE_MODE_RGBA32) 
                pos = ((pixelY + 1) * srcWidth + pixelX + 0) * 4;
            else
                pos = ((pixelY + 1) * srcWidth + pixelX + 0) * 3;

            r3 = image[pos+0];
            g3 = image[pos+1];
            b3 = image[pos+2];

            if (mode == IMAGE_MODE_RGBA32) 
                a3 = image[pos+3];


            if (mode == IMAGE_MODE_RGBA32) 
                pos = ((pixelY + 1) * srcWidth + pixelX + 1) * 4;
            else
                pos = ((pixelY + 1) * srcWidth + pixelX + 1) * 3;

            r4 = image[pos+0];
            g4 = image[pos+1];
            b4 = image[pos+2];

            if (mode == IMAGE_MODE_RGBA32) 
                a4 = image[pos+3];

            // determine weights
            fx = sourceX - pixelX;
            fy = sourceY - pixelY;
            fx1 = 1.0f - fx;
            fy1 = 1.0f - fy;

            w1 = (int)(fx1*fy1*256.0);
            w2 = (int)(fx*fy1*256.0);
            w3 = (int)(fx1*fy*256.0);
            w4 = (int)(fx*fy*256.0);
 
            // set output pixels
            if (mode == IMAGE_MODE_RGBA32) 
                pos = ((tmpy*destWidth) + tmpx) * 4;
            else
                pos = ((tmpy*destWidth) + tmpx) * 3;

            out[pos+0] = (uint8_t)((r1 * w1 + r2 * w2 + r3 * w3 + r4 * w4) >> 8);
            out[pos+1] = (uint8_t)((g1 * w1 + g2 * w2 + g3 * w3 + g4 * w4) >> 8);
            out[pos+2] = (uint8_t)((b1 * w1 + b2 * w2 + b3 * w3 + b4 * w4) >> 8);

            if (mode == IMAGE_MODE_RGBA32) 
                out[pos+3] = (uint8_t)((a1 * w1 + a2 * w2 + a3 * w3 + a4 * w4) >> 8);
        }
    }

    return out;
}

void menuEntryParseNacp(menuEntry_s* me) {
    NacpLanguageEntry *langentry = NULL;

    if (me->nacp==NULL) return;

    strncpy(me->version, me->nacp->version, sizeof(me->version)-1);

    #ifdef __SWITCH__
    Result rc=0;
    rc = nacpGetLanguageEntry(me->nacp, &langentry);

    if (R_SUCCEEDED(rc) && langentry!=NULL) {
    #else
    langentry = &me->nacp->lang[0];
    if (1) {
    #endif
        strncpy(me->name, langentry->name, sizeof(me->name)-1);
        strncpy(me->author, langentry->author, sizeof(me->author)-1);
    }

    free(me->nacp);
    me->nacp = NULL;
}

