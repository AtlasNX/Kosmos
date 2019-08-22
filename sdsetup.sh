#!/bin/bash
#
# Kosmos
# Copyright (C) 2019 Steven Mattera
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

func_result=""
auto=0

# Prompts for GitHub Username and Password
# Returns:
#   The username and password on ${func_result}.
prompt_login () {
    echo "It is recommended that you login to GitHub to use this tool. However you"
    echo "can continue without logging in, but you may run into rate limits. If you"
    echo "use two-factor authentication then you will need to generate a personal"
    echo "access token and use it as your password. https://github.com/settings/tokens"
    echo ""
    read -p "Username: (Leave blank to continue without logging in) " username

    if [ ! -z "${username}" ]
    then
        read -sp "Password or personal access token:" password
        func_result="${username}:${password}"
    else
        func_result=""
    fi
}

# =============================================================================
# Main Script
# =============================================================================

if [ $# -le 0 ]
then
    echo "Usage: ./sdsetup.sh <output_directory> [github username] [github password/access token] [\"auto\"]"
    exit 1
fi

if [ $# -gt 3 ]
then
    if [ "${4}" == "auto" ]
    then
        auto=1
    fi
fi

authenticated=0
username_password=""

if [ $# -gt 2 ]
then
    username_password=${2}:${3}
    authenticated=$(./common.sh test_login "${username_password}")
    if [ $authenticated -ne 1 ]
    then
        echo "Authentication Failed"
        exit 1
    fi
else
    while [ $authenticated -ne 1 ]; do
        prompt_login
        username_password=${func_result}

        if [ ! -z "${username_password}" ]
        then
            authenticated=$(./common.sh test_login "${username_password}")
        else
            authenticated=1
        fi

        echo ""
    done
fi



# Get version number
version_number=$(head -1 version.txt)

# Build temp directory
temp_directory="/tmp/$(uuidgen)"
mkdir -p "${temp_directory}"

# Start building!
atmosphere_version=$(./modules.sh download_atmosphere "${temp_directory}/must_have" "${username_password}")
hekate_version=$(./modules.sh download_hekate "${temp_directory}/must_have" "${version_number}" "${username_password}")
emuiibo_version=$(./modules.sh download_emuiibo "${temp_directory}/emuiibo" "${username_password}")
hid_mitm_version=$(./modules.sh download_hid_mitm "${temp_directory}/hid_mitm" "${username_password}")
kosmos_toolbox_version=$(./modules.sh download_kosmos_toolbox "${temp_directory}/kosmos_toolbox" "${username_password}")
kosmos_updater_version=$(./modules.sh download_kosmos_updater "${temp_directory}/kosmos_updater" "${version_number}" "${username_password}")
ldn_mitm_version=$(./modules.sh download_ldn_mitm "${temp_directory}/ldn_mitm" "${username_password}")
lockpick_rcm_version=$(./modules.sh download_lockpick_rcm "${temp_directory}/lockpick_rcm" "${username_password}")
sys_clk_version=$(./modules.sh download_sys_clk "${temp_directory}/sys_clk" "${username_password}")
sys_ftpd_version=$(./modules.sh download_sys_ftpd "${temp_directory}/sys_ftpd" "${username_password}")

if [ "${auto}" != "1" ]
then
    appstore_version=$(./modules.sh download_appstore "${temp_directory}/appstore" "${username_password}")
    edizon_version=$(./modules.sh download_edizon "${temp_directory}/edizon" "${username_password}")
    goldleaf_version=$(./modules.sh download_goldleaf "${temp_directory}/goldleaf" "${username_password}")
    lockpick_version=$(./modules.sh download_lockpick "${temp_directory}/lockpick" "${username_password}")
fi


# Delete some files we don't want in the output
rm -f ${temp_directory}/must_have/hbmenu.nro
rm -f ${temp_directory}/must_have/hekate*

# Delete the output directory if it already exists.
dest=""

if [ "${auto}" != "1" ]
then
    dest=$(realpath -s ${1})/${version_number}
else
    dest=$(realpath -s ${1})
fi

rm -rf ${dest}

# Move temp folder files to output directory
mv -f ${temp_directory} ${dest}

# Output some useful information.

if [ "${auto}" != "1" ]
then
    echo "SDSetup Modules built with:"
    echo "  Atmosphere - ${atmosphere_version}"
    echo "  Hekate - ${hekate_version}"
    echo "  EdiZon - ${edizon_version}"
    echo "  Emuiibo - ${emuiibo_version}"
    echo "  Goldleaf - ${goldleaf_version}"
    echo "  hid-mitm - ${hid_mitm_version}"
    echo "  Homebrew App Store - ${appstore_version}"
    echo "  Kosmos Toolbox - ${kosmos_toolbox_version}"
    echo "  Kosmos Updater - ${kosmos_updater_version}"
    echo "  ldn_mitm - ${ldn_mitm_version}"
    echo "  Lockpick - ${lockpick_version}"
    echo "  Lockpick RCM - ${lockpick_rcm_version}"
    echo "  sys-clk - ${sys_clk_version}"
    echo "  sys-ftpd - ${sys_ftpd_version}"
else
    echo "atmosphere:${atmosphere_version}"
    echo "hekate:${hekate_version}"
    echo "emuiibo:${emuiibo_version}"
    echo "hid-mitm:${hid_mitm_version}"
    echo "kosmos-toolbox:${kosmos_toolbox_version}"
    echo "kosmos-updater:${kosmos_updater_version}"
    echo "ldn_mitm:${ldn_mitm_version}"
    echo "lockpick_rcm:${lockpick_rcm_version}"
    echo "sys-clk:${sys_clk_version}"
    echo "sys-ftpd:${sys_ftpd_version}"
fi