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
import xmltodict
import zipfile

gh = Github(config.github_username, config.github_password)
gl = Gitlab('https://gitlab.com', private_token=config.gitlab_private_access_token)
gl.auth()

def get_latest_release(module, include_prereleases = True):
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

        if include_prereleases:
            return releases[0]

        for release in releases:
            if not release.prerelease:
                return release
        
        return None
    elif common.GitService(module['git']['service']) == common.GitService.GitLab:
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
    else:
        releases = None
        with urllib.request.urlopen(f'https://sourceforge.net/projects/{module["git"]["repo_name"]}/rss?path=/') as fd:
            releases = xmltodict.parse(fd.read().decode('utf-8'))

        return releases

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
    elif common.GitService(module['git']['service']) == common.GitService.GitLab:
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
    else:
        matched_item = None
        for item in release['rss']['channel']['item']:
            if re.search(pattern, item['title']):
                matched_item = item
                break

        if matched_item is None:
            print(f'[Error] Unable to find asset that match pattern: "{pattern}"')
            return None

        download_path = common.generate_temp_path()
        urllib.request.urlretrieve(matched_item['link'], download_path)

        return download_path

def find_asset(release, pattern):
    for asset in release.get_assets():
        if re.search(pattern, asset.name):
            return asset

    return None

def get_version(module, release, index):
    if common.GitService(module['git']['service']) == common.GitService.GitHub:
        return release.tag_name
    elif common.GitService(module['git']['service']) == common.GitService.GitLab:
        return release.name
    else:
        matched_item = None
        for item in release['rss']['channel']['item']:
            if re.search(module['git']['asset_patterns'][index], item['title']):
                matched_item = item
                break

        if matched_item is None:
            return "Latest"

        match = re.search(module['git']['version_pattern'], matched_item['title'])
        if match is None:
            return "Latest"

        groups = match.groups()
        if len(groups) == 0:
            return "Latest"

        return groups[0]

def download_atmosphere(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None

    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)
    
    common.delete_path(bundle_path)
    common.delete_path(os.path.join(temp_directory, 'switch', 'reboot_to_payload.nro'))
    common.delete_path(os.path.join(temp_directory, 'switch'))
    common.delete_path(os.path.join(temp_directory, 'atmosphere', 'reboot_payload.bin'))
    
    payload_path = download_asset(module, release, 1)
    if payload_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'bootloader', 'payloads'))
    shutil.move(payload_path, os.path.join(temp_directory, 'bootloader', 'payloads', 'fusee-primary.bin'))

    common.copy_module_file('atmosphere', 'system_settings.ini', os.path.join(temp_directory, 'atmosphere', 'config', 'system_settings.ini'))

    if not kosmos_build:
        common.delete_path(os.path.join(temp_directory, 'hbmenu.nro'))

    return get_version(module, release, 0)

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

    common.delete_path(os.path.join(temp_directory, 'nyx_usb_max_rate (run once per windows pc).reg'))
    
    if not kosmos_build:
        common.mkdir(os.path.join(temp_directory, '..', 'must_have'))
        common.move_contents_of_folder(os.path.join(temp_directory, 'bootloader'), os.path.join(temp_directory, '..', 'must_have', 'bootloader'))
        shutil.move(os.path.join(temp_directory, 'atmosphere', 'reboot_payload.bin'), os.path.join(temp_directory, '..', 'must_have', 'atmosphere', 'reboot_payload.bin'))
        common.delete_path(os.path.join(temp_directory, 'atmosphere'))

    return get_version(module, release, 0)

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

    return get_version(module, release, 0)

def download_edizon(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'EdiZon'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'EdiZon', 'EdiZon.nro'))

    return get_version(module, release, 0)

def download_emuiibo(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)

    common.delete_path(bundle_path)
    common.mkdir(os.path.join(temp_directory, 'atmosphere', 'contents'))
    shutil.move(os.path.join(temp_directory, 'SdOut', 'atmosphere', 'contents', '0100000000000352'), os.path.join(temp_directory, 'atmosphere', 'contents', '0100000000000352'))
    common.mkdir(os.path.join(temp_directory, 'switch', '.overlays'))
    shutil.move(os.path.join(temp_directory, 'SdOut', 'switch', '.overlays', 'emuiibo.ovl'), os.path.join(temp_directory, 'switch', '.overlays', 'emuiibo.ovl'))  
    common.delete_path(os.path.join(temp_directory, 'SdOut'))
    if kosmos_build:
        common.delete_path(os.path.join(temp_directory, 'atmosphere', 'contents', '0100000000000352', 'flags', 'boot2.flag'))
    common.copy_module_file('emuiibo', 'toolbox.json', os.path.join(temp_directory, 'atmosphere', 'contents', '0100000000000352', 'toolbox.json'))

    return get_version(module, release, 0)

def download_goldleaf(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'Goldleaf'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'Goldleaf', 'Goldleaf.nro'))

    return get_version(module, release, 0)

def download_kosmos_cleaner(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)

    return get_version(module, release, 0)

def download_kosmos_toolbox(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'KosmosToolbox'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'KosmosToolbox', 'KosmosToolbox.nro'))
    common.copy_module_file('kosmos-toolbox', 'config.json', os.path.join(temp_directory, 'switch', 'KosmosToolbox', 'config.json'))

    return get_version(module, release, 0)

def download_kosmos_updater(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'KosmosUpdater'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'KosmosUpdater', 'KosmosUpdater.nro'))
    common.copy_module_file('kosmos-updater', 'internal.db', os.path.join(temp_directory, 'switch', 'KosmosUpdater', 'internal.db'))
    common.sed('KOSMOS_VERSION', kosmos_version, os.path.join(temp_directory, 'switch', 'KosmosUpdater', 'internal.db'))

    return get_version(module, release, 0)

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

    return get_version(module, release, 0)

def download_lockpick(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'Lockpick'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'Lockpick', 'Lockpick.nro'))

    return get_version(module, release, 0)

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

    return get_version(module, release, 0)

def download_nxdumptool(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', 'NXDumpTool'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', 'NXDumpTool', 'NXDumpTool.nro'))

    return get_version(module, release, 0)

def download_nx_ovlloader(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)
    
    common.delete_path(bundle_path)

    return get_version(module, release, 0)

def download_ovl_sysmodules(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', '.overlays'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', '.overlays', 'ovlSysmodules.ovl'))

    return get_version(module, release, 0)

def download_status_monitor_overlay(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    app_path = download_asset(module, release, 0)
    if app_path is None:
        return None

    common.mkdir(os.path.join(temp_directory, 'switch', '.overlays'))
    shutil.move(app_path, os.path.join(temp_directory, 'switch', '.overlays', 'Status-Monitor-Overlay.ovl'))

    return get_version(module, release, 0)

def download_sys_clk(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)

    common.delete_path(bundle_path)
    if kosmos_build:
        common.delete_path(os.path.join(temp_directory, 'atmosphere', 'contents', '00FF0000636C6BFF', 'flags', 'boot2.flag'))
    common.delete_path(os.path.join(temp_directory, 'README.md'))
    common.copy_module_file('sys-clk', 'toolbox.json', os.path.join(temp_directory, 'atmosphere', 'contents', '00FF0000636C6BFF', 'toolbox.json'))

    return get_version(module, release, 0)

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

    return get_version(module, release, 0)

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

    return get_version(module, release, 0)

def download_tesla_menu(module, temp_directory, kosmos_version, kosmos_build):
    release = get_latest_release(module)
    bundle_path = download_asset(module, release, 0)
    if bundle_path is None:
        return None
    
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)
    
    common.delete_path(bundle_path)

    return get_version(module, release, 0)

def build(temp_directory, kosmos_version, command, auto_build):
    results = []

    modules_filename = 'kosmos.json'
    if command == common.Command.KosmosMinimal:
        modules_filename = 'kosmos-minimal.json'
    elif command == common.Command.SDSetup:
        modules_filename = 'sdsetup.json'

    # Open up modules.json
    with open(modules_filename) as json_file:
        # Parse JSON
        data = json.load(json_file)

        # Loop through modules
        for module in data:
            # Running a SDSetup Build
            if command == common.Command.SDSetup:
                # Only show prompts when it's not an auto build.
                if not auto_build:
                    print(f'Downloading {module["name"]}...')

                # Make sure module directory is created.
                module_directory = os.path.join(temp_directory, module['sdsetup_module_name'])
                common.mkdir(module_directory)

                # Download the module.
                download = globals()[module['download_function_name']]
                version = download(module, module_directory, kosmos_version, False)
                if version is None:
                    return None

                # Auto builds have a different prompt at the end for parsing.
                if auto_build:
                    results.append(f'{module["sdsetup_module_name"]}:{version}')
                else:
                    results.append(f'  {module["name"]} - {version}')
    
            # Running a Kosmos Build
            else:
                # Download the module.
                print(f'Downloading {module["name"]}...')
                download = globals()[module['download_function_name']]
                version = download(module, temp_directory, kosmos_version, True)
                if version is None:
                    return None
                results.append(f'  {module["name"]} - {version}')

    return results
