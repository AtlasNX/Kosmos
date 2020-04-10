#
# Kosmos Builder
# Copyright (C) 2020 Nichole Mattera
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
# 02110-1301, USA.
#

import common
import config
from github import Github
from gitlab import Gitlab
import json
import os
import re
import shutil
import urllib.request
import uuid
import zipfile

gh = Github(config.github_username, config.github_password)
gl = Gitlab('https://gitlab.com', private_token=config.gitlab_private_access_token)
gl.auth()

def get_latest_release(module):
    if common.GitService(module['git']['service']) == common.GitService.GitHub:
        try:
            repo = gh.get_repo(f'{module["git"]["org_name"]}/{module["git"]["repo_name"]}')
        except:
            print(f'[Error] Unable to find repo: {module["git"]["org_name"]}/{module["git"]["repo_name"]}')
            return None
        
        releases = repo.get_releases()
        if releases.totalCount == 0:
            print(f'[Error] Unable to find any releases for repo: {module["git"]["org_name"]}/{module["git"]["repo_name"]}')
            return None
        
        return releases[0]
    else:
        try:
            project = gl.projects.get(f'{module["git"]["org_name"]}/{module["git"]["repo_name"]}')
        except:
            print(f'[Error] Unable to find repo: {module["git"]["org_name"]}/{module["git"]["repo_name"]}')
            return None

        tags = project.tags.list()
        for tag in tags:
            if tag.release is not None:
                return tag

        print(f'[Error] Unable to find any releases for repo: {module["git"]["org_name"]}/{module["git"]["repo_name"]}')
        return None

def download_asset(module, release, index):
    pattern = module['git']['asset_patterns'][index]

    if common.GitService(module['git']['service']) == common.GitService.GitHub:
        if release is None:
            return None
        
        matched_asset = None
        for asset in release.get_assets():
            if re.search(pattern, asset.name):
                matched_asset = asset
                break

        if matched_asset is None:
            print(f'[Error] Unable to find asset that match pattern: "{pattern}"')
            return None

        download_path = common.generate_temp_path()
        urllib.request.urlretrieve(matched_asset.browser_download_url, download_path)

        return download_path
    else:
        group = module['git']['group']

        match = re.search(pattern, release.release['description'])
        if match is None:
            return None

        groups = match.groups()
        if len(groups) <= group:
            return None

        download_path = common.generate_temp_path()
        urllib.request.urlretrieve(f'https://gitlab.com/{module["git"]["org_name"]}/{module["git"]["repo_name"]}{groups[group]}', download_path)

        return download_path

def find_asset(release, pattern):
    for asset in release.get_assets():
        if re.search(pattern, asset.name):
            return asset

    return None

def download_atmosphere(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None

    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)
    
    common.delete_path(bundle_path)
    common.delete_path(os.path.join(temp_directory, 'switch', 'reboot_to_payload.nro'))
    common.delete_path(os.path.join(temp_directory, 'atmosphere', 'reboot_payload.bin'))
    
    payload_path = download_asset(module, release, 1)
    if payload_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'bootloader', 'payloads'))
    shutil.move(payload_path, os.path.join(temp_directory, 'bootloader', 'payloads', 'fusee-primary.bin'))

    common.copy_module_file('atmosphere', 'system_settings.ini', os.path.join(temp_directory, 'atmosphere', 'config', 'system_settings.ini'))
    common.copy_module_folder('atmosphere', 'exefs_patches', os.path.join(temp_directory, 'atmosphere', 'exefs_patches'))

    if not kosmos_build:
        common.delete_path(os.path.join(temp_directory, 'hbmenu.nro'))

    return release.tag_name

def download_hekate(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None

    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)

    common.delete_path(bundle_path)
    
    common.copy_module_file('hekate', 'bootlogo.bmp', os.path.join(temp_directory, 'bootloader', 'bootlogo.bmp'))
    common.copy_module_file('hekate', 'hekate_ipl.ini', os.path.join(temp_directory, 'bootloader', 'hekate_ipl.ini'))
    common.sed('KOSMOS_VERSION', kosmos_version, os.path.join(temp_directory, 'bootloader', 'hekate_ipl.ini'))

    payload = common.find_file(os.path.join(temp_directory, 'hekate_ctcaer_*.bin'))
    if len(payload) != 0:
        shutil.copyfile(payload[0], os.path.join(temp_directory, 'bootloader', 'update.bin'))
        common.mkdir(os.path.join(temp_directory, 'atmosphere'))
        shutil.copyfile(payload[0], os.path.join(temp_directory, 'atmosphere', 'reboot_payload.bin'))

    if not kosmos_build:
        common.mkdir(os.path.join(temp_directory, '..', 'must_have'))
        shutil.move(os.path.join(temp_directory, 'bootloader'), os.path.join(temp_directory, '..', 'must_have'))
        shutil.move(os.path.join(temp_directory, 'atmosphere'), os.path.join(temp_directory, '..', 'must_have'))

    return release.tag_name

def download_appstore(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None

    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)
    
    common.delete_path(bundle_path)
    common.mkdir(os.path.join(temp_directory, 'switch', 'appstore'))
    shutil.move(os.path.join(temp_directory, 'appstore.nro'), os.path.join(temp_directory, 'switch', 'appstore', 'appstore.nro'))

    return release.name

def download_edizon(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'EdiZon'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'EdiZon', 'EdiZon.nro'))

    return release.tag_name

def download_emuiibo(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)

    common.delete_path(bundle_path)
    common.mkdir(os.path.join(temp_directory, 'atmosphere', 'contents'))
    shutil.move(os.path.join(temp_directory, 'contents', '0100000000000352'), os.path.join(temp_directory, 'atmosphere', 'contents', '0100000000000352'))
    common.delete_path(os.path.join(temp_directory, 'contents'))
    if kosmos_build:
        common.delete_path(os.path.join(temp_directory, 'atmosphere', 'contents', '0100000000000352', 'flags', 'boot2.flag'))
    common.copy_module_file('emuiibo', 'toolbox.json', os.path.join(temp_directory, 'atmosphere', 'contents', '0100000000000352', 'toolbox.json'))

    return release.tag_name

def download_goldleaf(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'Goldleaf'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'Goldleaf', 'Goldleaf.nro'))

    return release.tag_name

def download_kosmos_toolbox(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'KosmosToolbox'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'KosmosToolbox', 'KosmosToolbox.nro'))
    common.copy_module_file('kosmos-toolbox', 'config.json', os.path.join(temp_directory, 'switch', 'KosmosToolbox', 'config.json'))

    return release.tag_name

def download_kosmos_updater(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'KosmosUpdater'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'KosmosUpdater', 'KosmosUpdater.nro'))
    common.copy_module_file('kosmos-updater', 'internal.db', os.path.join(temp_directory, 'switch', 'KosmosUpdater', 'internal.db'))
    common.sed('KOSMOS_VERSION', kosmos_version, os.path.join(temp_directory, 'switch', 'KosmosUpdater', 'internal.db'))

    return release.tag_name

def download_ldn_mitm(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)
    
    common.delete_path(bundle_path)
    if kosmos_build:
        common.delete_path(os.path.join(temp_directory, 'atmosphere', 'contents', '4200000000000010', 'flags', 'boot2.flag'))
    common.copy_module_file('ldn_mitm', 'toolbox.json', os.path.join(temp_directory, 'atmosphere', 'contents', '4200000000000010', 'toolbox.json'))

    return release.tag_name

def download_lockpick(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'Lockpick'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'Lockpick', 'Lockpick.nro'))

    return release.tag_name

def download_lockpick_rcm(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    payload_path = download_asset(module, release, 0)
    if payload_path is None:
        return None

    if kosmos_build:
        common.mkdir(os.path.join(temp_directory, 'bootloader', 'payloads'))
        shutil.move(payload_path, os.path.join(temp_directory, 'bootloader', 'payloads', 'Lockpick_RCM.bin'))
    else:
        shutil.move(payload_path, os.path.join(temp_directory, 'Lockpick_RCM.bin'))

    return release.tag_name

def download_nxdumptool(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'NXDumpTool'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'NXDumpTool', 'NXDumpTool.nro'))

    return release.tag_name

def download_nx_ovlloader(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)
    
    common.delete_path(bundle_path)

    return release.tag_name

def download_ovl_sysmodules(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', '.overlays'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', '.overlays', 'ovlSysmodules.ovl'))

    return release.tag_name

def download_status_monitor_overlay(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', '.overlays'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', '.overlays', 'Status-Monitor-Overlay.ovl'))

    return release.tag_name

def download_sys_clk(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)

    common.delete_path(bundle_path)
    common.mkdir(os.path.join(temp_directory, 'atmosphere', 'contents'))
    shutil.move(os.path.join(temp_directory, 'atmosphere', 'titles', '00FF0000636C6BFF'), os.path.join(temp_directory, 'atmosphere', 'contents', '00FF0000636C6BFF'))
    common.delete_path(os.path.join(temp_directory, 'atmosphere', 'titles'))
    if kosmos_build:
        common.delete_path(os.path.join(temp_directory, 'atmosphere', 'contents', '00FF0000636C6BFF', 'flags', 'boot2.flag'))
    common.delete_path(os.path.join(temp_directory, 'README.md'))
    common.copy_module_file('sys-clk', 'toolbox.json', os.path.join(temp_directory, 'atmosphere', 'contents', '00FF0000636C6BFF', 'toolbox.json'))

    return release.tag_name

def download_sys_con(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)

    common.delete_path(bundle_path)
    if kosmos_build:
        common.delete_path(os.path.join(temp_directory, 'atmosphere', 'contents', '690000000000000D', 'flags', 'boot2.flag'))

    return release.tag_name

def download_sys_ftpd_light(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)

    common.delete_path(bundle_path)
    if kosmos_build:
        common.delete_path(os.path.join(temp_directory, 'atmosphere', 'contents', '420000000000000E', 'flags', 'boot2.flag'))

    return release.tag_name

def download_tesla_menu(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)
    
    common.delete_path(bundle_path)

    return release.tag_name

def build(temp_directory, kosmos_version, kosmos_build, auto_build):
    results = []

    # Open up modules.json
    with open('modules.json') as json_file:
        # Parse JSON
        data = json.load(json_file)

        # Loop through modules
        for module in data:
            sdsetup_opts = module['sdsetup']

            # Running a Kosmos Build
            if kosmos_build:
                # Download the module.
                print(f'Downloading {module["name"]}...')
                download = globals()[module['download_function_name']]
                version = download(module, temp_directory, kosmos_version, kosmos_build)
                if version is None:
                    return None
                results.append(f'  {module["name"]} - {version}')

            # Running a SDSetup Build
            elif not kosmos_build and sdsetup_opts['included']:
                # Only show prompts when it's not an auto build.
                if not auto_build:
                    print(f'Downloading {module["name"]}...')

                # Make sure module directory is created.
                module_directory = os.path.join(temp_directory, sdsetup_opts['name'])
                common.mkdir(module_directory)

                # Download the module.
                download = globals()[module['download_function_name']]
                version = download(module, module_directory, kosmos_version, kosmos_build)
                if version is None:
                    return None

                # Auto builds have a different prompt at the end for parsing.
                if auto_build:
                    results.append(f'{sdsetup_opts["name"]}:{version}')
                else:
                    results.append(f'  {module["name"]} - {version}')
    
    return results
