#include "common.h"

static menu_s s_menu[2];
static menu_s s_menuFileassoc[2];
static bool s_curMenu, s_curMenuFileassoc;

menu_s* menuGetCurrent(void) {
    return &s_menu[s_curMenu];
}

menu_s* menuFileassocGetCurrent(void) {
    return &s_menuFileassoc[s_curMenuFileassoc];
}

menuEntry_s* menuCreateEntry(MenuEntryType type) {
    menuEntry_s* me = (menuEntry_s*)malloc(sizeof(menuEntry_s));
    menuEntryInit(me, type);
    return me;
}

void menuDeleteEntry(menuEntry_s* me, bool skip_icongfx) {
    menuEntryFree(me, skip_icongfx);
    free(me);
}

static void _menuAddEntry(menu_s *m, menuEntry_s* me) {
    me->menu = m;
    if (m->lastEntry)
    {
        m->lastEntry->next = me;
        m->lastEntry = me;
    } else
    {
        m->firstEntry = me;
        m->lastEntry = me;
    }
    m->xPos = 0;
    m->nEntries ++;
}

static void menuAddEntry(menuEntry_s* me) {
    _menuAddEntry(&s_menu[!s_curMenu], me);
}

void menuFileassocAddEntry(menuEntry_s* me) {
    _menuAddEntry(&s_menuFileassoc[!s_curMenuFileassoc], me);
}

static void menuAddEntryToFront(menuEntry_s* me) {
    menu_s* m = &s_menu[!s_curMenu];
    me->menu = m;
    if (m->lastEntry)
    {
        me->next = m->firstEntry;
        m->firstEntry = me;
    } else
    {
        m->firstEntry = me;
        m->lastEntry = me;
    }
    m->xPos = 0;
    m->nEntries ++;
}

static void _menuClear(menu_s* m) {
    menuEntry_s *cur, *next;
    for (cur = m->firstEntry; cur; cur = next)
    {
        next = cur->next;
        menuDeleteEntry(cur, 0);
    }
    memset(m, 0, sizeof(*m));
}

static void menuClear(void) {
    _menuClear(&s_menu[!s_curMenu]);
}

static void menuFileassocClear(void) {
    _menuClear(&s_menuFileassoc[!s_curMenuFileassoc]);
}

static int menuEntryCmp(const void *p1, const void *p2) {
    const menuEntry_s* lhs = *(menuEntry_s**)p1;
    const menuEntry_s* rhs = *(menuEntry_s**)p2;

    if(lhs->type == rhs->type)
        return strcasecmp(lhs->name, rhs->name);
    if(lhs->type == ENTRY_TYPE_FOLDER)
        return -1;
    return 1;
}

static void menuSort(void) {
    int i;
    menu_s* m = &s_menu[!s_curMenu];
    int nEntries = m->nEntries;
    if (nEntries==0) return;

    menuEntry_s** list = (menuEntry_s**)calloc(nEntries, sizeof(menuEntry_s*));
    if(list == NULL) return;

    menuEntry_s* p = m->firstEntry;
    for(i = 0; i < nEntries; ++i) {
        list[i] = p;
        p = p->next;
    }

    qsort(list, nEntries, sizeof(menuEntry_s*), menuEntryCmp);

    menuEntry_s** pp = &m->firstEntry;
    for(i = 0; i < nEntries; ++i) {
        *pp = list[i];
        pp = &(*pp)->next;
    }
    m->lastEntry = list[nEntries-1];
    *pp = NULL;

    free(list);
}

int menuScan(const char* target) {
    int pos;
    char dirsep[8];

    if (chdir(target) < 0) return 1;
    if (getcwd(s_menu[!s_curMenu].dirname, PATH_MAX+1) == NULL)
        return 1;

    memset(dirsep, 0, sizeof(dirsep));
    dirsep[0] = '/';

    //While cwd will not have '/' at the end normally, it will have it when cwd is the root dir ("sdmc:/"). Don't add '/' to the path below when it's already present.
    pos = strlen(s_menu[!s_curMenu].dirname);
    if (pos > 0) {
        if (s_menu[!s_curMenu].dirname[pos-1] == '/') dirsep[0] = 0;
    }

    DIR* dir;
    struct dirent* dp;
    char tmp_path[PATH_MAX+1];
    dir = opendir(s_menu[!s_curMenu].dirname);
    if (!dir) return 2;

    while ((dp = readdir(dir)))
    {
        menuEntry_s* me = NULL;
        bool shortcut = false;
        if (dp->d_name[0]=='.')
            continue;

        bool entrytype=0;

        memset(tmp_path, 0, sizeof(tmp_path));
        snprintf(tmp_path, sizeof(tmp_path)-1, "%s%s%s", s_menu[!s_curMenu].dirname, dirsep, dp->d_name);

        #ifdef __SWITCH__
        fsdev_dir_t* dirSt = (fsdev_dir_t*)dir->dirData->dirStruct;
        FsDirectoryEntry* entry = &dirSt->entry_data[dirSt->index];

        entrytype = entry->type == ENTRYTYPE_DIR;
        #else
        struct stat tmpstat;

        if(stat(tmp_path, &tmpstat)==-1)
            continue;

        entrytype = (tmpstat.st_mode & S_IFMT) != S_IFREG;
        #endif

        if (entrytype)
            me = menuCreateEntry(ENTRY_TYPE_FOLDER);
        else
        {
            const char* ext = getExtension(dp->d_name);
            if (strcasecmp(ext, ".nro")==0/* || (shortcut = strcasecmp(ext, ".xml")==0)*/)
                me = menuCreateEntry(ENTRY_TYPE_FILE);

            if (!me)
                me = menuCreateEntry(ENTRY_TYPE_FILE_OTHER);
        }

        if (!me)
            continue;

        strncpy(me->path, tmp_path, sizeof(me->path)-1);
        me->path[sizeof(me->path)-1] = 0;

        if (menuEntryLoad(me, dp->d_name, shortcut))
            menuAddEntry(me);
        else
            menuDeleteEntry(me, 0);
    }

    closedir(dir);
    menuSort();

    // Swap the menu and clear the previous menu
    s_curMenu = !s_curMenu;
    menuClear();
    return 0;
}

int themeMenuScan(const char* target) {
    menuClear();
    if (chdir(target) < 0) return 1;
    if (getcwd(s_menu[!s_curMenu].dirname, PATH_MAX+1) == NULL)
        return 1;
    DIR* dir;
    struct dirent* dp;
    char tmp_path[PATH_MAX+1];
    dir = opendir(s_menu[!s_curMenu].dirname);
    if (!dir) return 2;

    while ((dp = readdir(dir)))
    {
        menuEntry_s* me = NULL;

        bool shortcut = false;
        if (dp->d_name[0]=='.')
            continue;

        memset(tmp_path, 0, sizeof(tmp_path));
        snprintf(tmp_path, sizeof(tmp_path)-1, "%s/%s", s_menu[!s_curMenu].dirname, dp->d_name);

        const char* ext = getExtension(dp->d_name);
        if (strcasecmp(ext, ".cfg")==0)
            me = menuCreateEntry(ENTRY_TYPE_THEME);

        if (!me)
            continue;

        strncpy(me->path, tmp_path, sizeof(me->path)-1);
        me->path[sizeof(me->path)-1] = 0;
        if (menuEntryLoad(me, dp->d_name, shortcut))
            menuAddEntry(me);
        else
            menuDeleteEntry(me, 0);
    }

    closedir(dir);
    menuSort();

    menuEntry_s* me = menuCreateEntry(ENTRY_TYPE_THEME);

    if(me) {
        if(menuEntryLoad(me, textGetString(StrId_DefaultThemeName), false))//Create Default theme Menu Entry
            menuAddEntryToFront(me);
        else
            menuDeleteEntry(me, 0);
    }
    // Swap the menu and clear the previous menu
    s_curMenu = !s_curMenu;
    menuClear();
    return 0;
}

int menuFileassocScan(const char* target) {
    menuFileassocClear();
    if (chdir(target) < 0) return 1;
    if (getcwd(s_menuFileassoc[!s_curMenuFileassoc].dirname, PATH_MAX+1) == NULL)
        return 1;
    DIR* dir;
    struct dirent* dp;
    char tmp_path[PATH_MAX+1];
    dir = opendir(s_menuFileassoc[!s_curMenuFileassoc].dirname);
    if (!dir) return 2;

    while ((dp = readdir(dir)))
    {
        if (dp->d_name[0]=='.')
            continue;

        memset(tmp_path, 0, sizeof(tmp_path));
        snprintf(tmp_path, sizeof(tmp_path)-1, "%s/%s", s_menuFileassoc[!s_curMenuFileassoc].dirname, dp->d_name);

        const char* ext = getExtension(dp->d_name);
        if (strcasecmp(ext, ".cfg")!=0)
            continue;

        menuEntryFileassocLoad(tmp_path);
    }

    closedir(dir);

    // Swap the menu and clear the previous menu
    s_curMenuFileassoc = !s_curMenuFileassoc;
    menuFileassocClear();
    return 0;
}
