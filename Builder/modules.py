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
import json
import os
import re
import shutil
import urllib.request
import uuid
import zipfile

git = Github(config.github_username, config.github_password)

def get_latest_release(module):
    repo = git.get_repo(f'{module["git"]["org_name"]}/{module["git"]["repo_name"]}')
    return repo.get_latest_release()

def download_asset(release, pattern):
    matched_asset = None
    for asset in release.get_assets():
        if re.search(pattern, asset.name):
            matched_asset = asset
            break

    if matched_asset is None:
        return None

    download_path = common.generate_temp_path()
    urllib.request.urlretrieve(matched_asset.browser_download_url, download_path)

    return download_path

def find_asset(release, pattern):
    for asset in release.get_assets():
        if re.search(pattern, asset.name):
            return asset

    return None

def download_atmosphere(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    bundle_path = download_asset(release, '.*atmosphere-.*\\.zip')
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)
    
    common.delete_path(bundle_path)
    common.delete_path(os.path.join(temp_directory, 'switch', 'reboot_to_payload.nro'))
    common.delete_path(os.path.join(temp_directory, 'atmosphere', 'reboot_payload.bin'))
    
    payload_path = download_asset(release, '.*fusee-primary\\.bin')

    os.makedirs(os.path.join(temp_directory, 'bootloader', 'payloads'))
    shutil.move(payload_path, os.path.join(temp_directory, 'bootloader', 'payloads','fusee-primary.bin'))
    common.copy_module_file('atmosphere', 'system_settings.ini', os.path.join(temp_directory, 'atmosphere', 'config', 'system_settings.ini'))

    return release.tag_name

def download_hekate(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    bundle_path = download_asset(release, '.*hekate_ctcaer_.*\\.zip')
    with zipfile.ZipFile(bundle_path, 'r') as zip_ref:
        zip_ref.extractall(temp_directory)

    common.delete_path(bundle_path)
    
    common.copy_module_file('hekate', 'hekate_ipl.ini', os.path.join(temp_directory, 'bootloader', 'hekate_ipl.ini'))
    common.sed('KOSMOS_VERSION', kosmos_version, os.path.join(temp_directory, 'bootloader', 'hekate_ipl.ini'))

    payload = common.find_file(os.path.join(temp_directory, 'hekate_ctcaer_*.bin'))
    if len(payload) != 0:
        shutil.copyfile(payload[0], os.path.join(temp_directory, 'bootloader', 'update.bin'))
        shutil.copyfile(payload[0], os.path.join(temp_directory, 'atmosphere', 'reboot_payload.bin'))

    return release.tag_name

def download_appstore(module, temp_directory, kosmos_version):
    # TODO: GitLab.
    return 'v1.0.0'

def download_edizon(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_emuiibo(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_goldleaf(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_kosmos_toolbox(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_kosmos_updater(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_ldn_mitm(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_lockpick(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_lockpick_rcm(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_nxdumptool(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_nx_ovlloader(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_ovl_sysmodules(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_sys_clk(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_sys_con(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_sys_ftpd_light(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
    return release.tag_name

def download_tesla_menu(module, temp_directory, kosmos_version):
    release = get_latest_release(module)
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
                version = download(module, temp_directory, kosmos_version)
                results.append(f'  {module["name"]} - {version}')

            # Running a SDSetup Build
            elif not kosmos_build and sdsetup_opts['included']:
                # Only show prompts when it's not an auto build.
                if not auto_build:
                    print(f'Downloading {module["name"]}...')

                # Download the module.
                version = ''
                if not auto_build or (
                    auto_build and sdsetup_opts['included_with_auto_builds']):
                    download = globals()[module['download_function_name']]
                    version = download(module, temp_directory, kosmos_version)
                else:
                    continue

                # Auto builds have a different prompt at the end for parsing.
                if auto_build and sdsetup_opts['included_with_auto_builds']:
                    results.append(f'{sdsetup_opts["auto_build_name"]}:{version}')
                else:
                    results.append(f'  {module["name"]} - {version}')
    
    return results
