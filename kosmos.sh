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
    echo "Usage: ./kosmos.sh [output_directory]"
    exit 1
fi

authenticated=0
username_password=""
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

# Get version number
version_number=$(head -1 version.txt)

# Build temp directory
temp_directory="/tmp/$(uuidgen)"
mkdir -p "${temp_directory}"

# Start building!
atmosphere_version=$(./modules.sh download_atmosphere "${temp_directory}" "${username_password}")
hekate_version=$(./modules.sh download_hekate "${temp_directory}" "${version_number}" "${username_password}")
appstore_version=$(./modules.sh download_appstore "${temp_directory}" "${username_password}")
edizon_version=$(./modules.sh download_edizon "${temp_directory}" "${username_password}")
emuiibo_version=$(./modules.sh download_emuiibo "${temp_directory}" "${username_password}")
goldleaf_version=$(./modules.sh download_goldleaf "${temp_directory}" "${username_password}")
kosmos_toolbox_version=$(./modules.sh download_kosmos_toolbox "${temp_directory}" "${username_password}")
kosmos_updater_version=$(./modules.sh download_kosmos_updater "${temp_directory}" "${version_number}" "${username_password}")
ldn_mitm_version=$(./modules.sh download_ldn_mitm "${temp_directory}" "${username_password}")
lockpick_version=$(./modules.sh download_lockpick "${temp_directory}" "${username_password}")
lockpick_rcm_version=$(./modules.sh download_lockpick_rcm "${temp_directory}" "${username_password}")
sys_clk_version=$(./modules.sh download_sys_clk "${temp_directory}" "${username_password}")
sys_clk_editor_version=$(./modules.sh download_sys_clk_editor "${temp_directory}" "${username_password}")
sys_ftpd_version=$(./modules.sh download_sys_ftpd "${temp_directory}" "${username_password}")
nxdumptool_version=$(./modules.sh download_nxdumptool "${temp_directory}" "${username_password}")

# Delete the bundle if it already exists.
dest=$(realpath -s ${1})
rm -f "${dest}/Kosmos-${version_number}.zip"

# Bundle everything together.
current_directory=${PWD}
cd "${temp_directory}"
zip -q -r "${dest}/Kosmos-${version_number}.zip" .
cd "${current_directory}"

# Clean up.
rm -rf "${temp_directory}"

# Output some useful information.
echo "Kosmos ${version_number} built with:"
echo "  Atmosphere - ${atmosphere_version}"
echo "  Hekate - ${hekate_version}"
echo "  EdiZon - ${edizon_version}"
echo "  Emuiibo - ${emuiibo_version}"
echo "  Goldleaf - ${goldleaf_version}"
echo "  Homebrew App Store - ${appstore_version}"
echo "  Kosmos Toolbox - ${kosmos_toolbox_version}"
echo "  Kosmos Updater - ${kosmos_updater_version}"
echo "  ldn_mitm - ${ldn_mitm_version}"
echo "  Lockpick - ${lockpick_version}"
echo "  Lockpick RCM - ${lockpick_rcm_version}"
echo "  sys-clk - ${sys_clk_version}"
echo "  sys-clk-editor - ${sys_clk_editor_version}"
echo "  sys-ftpd - ${sys_ftpd_version}"
echo "  NXDumpTool - ${nxdumptool_version}"
