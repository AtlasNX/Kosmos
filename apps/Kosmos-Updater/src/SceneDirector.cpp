// Kosmos Updater
// Copyright (C) 2018 Steven Mattera
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include <switch.h>
#include "SceneDirector.hpp"
#include "ConfigManager.hpp"

SceneDirector::SceneDirector() {
    romfsInit();
    setsysInitialize();
    plInitialize();

    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
        return;

    SceneDirector::window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_FULLSCREEN);
    if (!SceneDirector::window)
        return;

    SceneDirector::renderer = SDL_CreateRenderer(SceneDirector::window, 0, SDL_RENDERER_SOFTWARE);
    if (!SceneDirector::renderer)
        return;

    TTF_Init();

    Mix_AllocateChannels(2);
    Mix_OpenAudio(48000, AUDIO_S16, 2, 4096);

    _now = SDL_GetPerformanceCounter();
    _last = 0;
    _exFatWarningScene = NULL;
    _appUpdateScene = NULL;
    _packageSelectScene = NULL;
    _packageDownloadScene = NULL;
    _currentScene = NULL;

    if (!ConfigManager::getReceivedExFATWarning()) {
        currentScene = SCENE_EXFAT_WARNING;
    }
    else if (!ConfigManager::shouldAutoUpdate()) {
        currentScene = SCENE_PACKAGE_SELECT;
    }
}

SceneDirector::~SceneDirector() {
    if (_exFatWarningScene != NULL)
        delete _exFatWarningScene;

    if (_appUpdateScene != NULL)
        delete _appUpdateScene;

    if (_packageSelectScene != NULL)
        delete _packageSelectScene;

    if (_packageDownloadScene != NULL)
        delete _packageDownloadScene;

    Mix_HaltChannel(-1);
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();

    if (SceneDirector::renderer != NULL)
        SDL_DestroyRenderer(SceneDirector::renderer);

    if (SceneDirector::window != NULL)
        SDL_DestroyWindow(SceneDirector::window);

    IMG_Quit();
    SDL_Quit();

    plExit();
    setsysExit();
    romfsExit();
}

bool SceneDirector::direct() {
    _last = _now;
    _now = SDL_GetPerformanceCounter();
    double dTime = (double) ((_now - _last) * 1000 / SDL_GetPerformanceFrequency());

    hidScanInput();
    u32 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

    // TODO: Handle touch controls

    AssetManager::setRenderColor(AssetManager::background);
    SDL_RenderClear(SceneDirector::renderer);

    // Unload previous scenes
    switch(SceneDirector::currentScene) {
        case SCENE_APP_UPDATE:
            if (_exFatWarningScene != NULL) {
                delete _exFatWarningScene;
                _exFatWarningScene = NULL;
            }
            break;

        case SCENE_PACKAGE_SELECT:
            if (_appUpdateScene != NULL) {
                delete _appUpdateScene;
                _appUpdateScene = NULL;
            }
            break;

        case SCENE_PACKAGE_DOWNLOAD:
            if (_appUpdateScene != NULL) {
                delete _appUpdateScene;
                _appUpdateScene = NULL;
            }

            if (_packageSelectScene != NULL) {
                delete _packageSelectScene;
                _packageSelectScene = NULL;
            }
            break;

        default:
            /* Do Nothing */
            break;
    }

    // Load new scenes
    switch(SceneDirector::currentScene) {
        case SCENE_EXFAT_WARNING:
            if (_exFatWarningScene == NULL)
                _exFatWarningScene = new ExFatWarningScene();

            _currentScene = _exFatWarningScene;
            break;

        case SCENE_APP_UPDATE:
            if (_appUpdateScene == NULL)
                _appUpdateScene = new AppUpdateScene();
            
            _currentScene = _appUpdateScene;
            break;

        case SCENE_PACKAGE_SELECT:
            if (_packageSelectScene == NULL)
                _packageSelectScene = new PackageSelectScene();
            
            _currentScene = _packageSelectScene;
            break;

        case SCENE_PACKAGE_DOWNLOAD:
            if (_packageDownloadScene == NULL)
                _packageDownloadScene = new PackageDownloadScene();
            
            _currentScene = _packageDownloadScene;
            break;
    }

    if (modal != NULL) {
        modal->handleButton(kDown);
    } else {
        _currentScene->handleButton(kDown);
    }

    _currentScene->render({ 0, 0, 1280, 720 }, dTime);

    if (modal != NULL) {
        // Draw Faded background.
        AssetManager::setRenderColor(AssetManager::modal_faded_background);
        SDL_Rect fadedBGFrame = { 0, 0, 1280, 720 };
        SDL_SetRenderDrawBlendMode(SceneDirector::renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(SceneDirector::renderer, &fadedBGFrame);

        modal->render({ 0, 0, 1280, 720 }, dTime);
    }

    SDL_RenderPresent(SceneDirector::renderer);

    return !SceneDirector::exitApp;
}
