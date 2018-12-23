#pragma once

typedef struct {
    bool activated;
    bool launch_app;
    bool transferring;
    bool sock_connected;
    menuEntry_s *me;
    size_t filelen, filetotal;
    char errormsg[1025];
} netloaderState;

int netloader_activate(void);
int netloader_deactivate(void);
int netloader_loop(struct sockaddr_in *sa_remote);

Result netloaderInit(void);
void netloaderExit(void);

void netloaderTask(void* arg);

void netloaderGetState(netloaderState *state);
void netloaderSignalExit(void);
