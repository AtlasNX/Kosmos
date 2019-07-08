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
user_agent="Kosmos/1.0.0"

# =============================================================================
# General Functions
# =============================================================================

# Downloads the latest release JSON.
# Params:
#   - GitHub Company
#   - GitHub Repo
# Returns:
#   The latest release JSON on ${func_result}.
get_latest_release () {
    releases=$(curl -H  "Accept: application/json" -H "Content-Type: application/json" -H "User-Agent: ${user_agent}" -s https://api.github.com/repos/${1}/${2}/releases)
    func_result=$(echo ${releases} | jq -r '.[0]')
}

# Gets the number of assets in a release.
# Params:
#   - The release JSON
# Returns:
#   The number of assets on ${?}.
get_number_of_assets () {
    return $(echo ${1} | jq -r '.assets | length')
}

# Finds a specific asset in a release.
# Params:
#   - The release JSON
#   - Start with blob pattern
#   - Ends with blob pattern
# Returns:
#   The asset JSON on ${func_result}.
find_asset () {
    get_number_of_assets "${1}"
    number_of_assets=${?}

    for (( i=0; i<${number_of_assets}; i++ ))
    do
        name=$(echo ${1} | jq -r ".assets[${i}].name" | tr '[:upper:]' '[:lower:]')
        asset=$(echo ${1} | jq -r ".assets[${i}]")

        if [[ ${#} -eq 2 && ${name} == ${2} ]]
        then
            func_result=${asset}
            break
        fi

        if [[ ${#} -eq 3 && ${name} == ${2} && ${name} == ${3} ]]
        then
            func_result=${asset}
            break
        fi
    done
}

# Gets the download URL from an asset.
# Params:
#   - The release asset JSON
# Returns:
#   The download URL on ${func_result}.
get_download_url () {
    func_result=$(echo ${1} | jq -r ".browser_download_url")
}

# Downloads a file.
# Params:
#   - The URL
# Returns:
#   The file path on ${func_result}.
download_file () {
    func_result="/tmp/$(uuidgen)"
    curl -L -H "User-Agent: ${user_agent}" -s ${1} >> ${func_result}
}

# Gets the version number from an asset.
# Params:
#   - The release asset JSON
# Returns:
#   The version number on ${func_result}.
get_version_number () {
    func_result=$(echo ${1} | jq -r ".tag_name")
}

# Find path matching a pattern
# Params:
#   - The pattern
# Returns:
#   The first file found on ${func_result}.
glob () {
    files=( ${1} )
    func_result=${files[0]}
}

# =============================================================================
# Atmosphere Functions
# =============================================================================

# Downloads the latest Atmosphere release and extracts it.
# Params:
#   - Directory to extract to
# Returns:
#   The version number on ${func_result}.
download_atmosphere () {
    get_latest_release "Atmosphere-NX" "Atmosphere"
    latest_release=${func_result}

    find_asset "${latest_release}" "atmosphere*" "*.zip"
    get_download_url "${func_result}"
    download_file "${func_result}"

    unzip -qq "${func_result}" -d "${1}"
    rm -f "${1}/switch/reboot_to_payload.nro"
    rm -f "${func_result}"

    find_asset "${latest_release}" "fusee*" "*.bin"
    get_download_url "${func_result}"
    download_file "${func_result}"

    mkdir -p "${1}/bootloader/payloads"
    mv ${func_result} "${1}/bootloader/payloads/fusee-primary.bin"

    get_version_number "${latest_release}"
}

# =============================================================================
# Hekate Functions
# =============================================================================

# Downloads the latest Hekate release and extracts it.
# Params:
#   - Directory to extract to
# Returns:
#   The version number on ${func_result}.
download_hekate () {
    get_latest_release "CTCaer" "hekate"
    latest_release=${func_result}

    find_asset "${latest_release}" "hekate*" "*.zip"
    get_download_url "${func_result}"
    download_file "${func_result}"

    unzip -qq "${func_result}" -d "${1}"
    rm -f "${func_result}"

    get_version_number "${latest_release}"
}

# Copy the payload to where it needs to be.
# Params:
#   - The temp directory
copy_payload () {
    glob "${1}/hekate*.bin"
    cp "${func_result}" "${1}/bootloader/update.bin"
    cp "${func_result}" "${1}/atmosphere/reboot_payload.bin"
}

# Builds the hekate files.
# Params:
#   - The temp directory
#   - The Kosmos version number
build_hekate_files () {
    cp "./Modules/hekate/bootlogo.bmp" "${1}/bootloader/bootlogo.bmp"
    sed "s/KOSMOS_VERSION/${2}/g" "./Modules/hekate/hekate_ipl.ini" >> "${1}/bootloader/hekate_ipl.ini"
}

# =============================================================================
# Homebrew Functions
# =============================================================================

download_appstore () {
    get_latest_release "vgmoose" "hb-appstore"
    latest_release=${func_result}

    find_asset "${latest_release}" "*.nro"
    get_download_url "${func_result}"
    download_file "${func_result}"

    mkdir -p "${1}/switch/appstore"
    mv ${func_result} "${1}/switch/appstore/appstore.nro"

    get_version_number "${latest_release}"
}

download_edizon () {
    get_latest_release "WerWolv" "EdiZon"
    latest_release=${func_result}

    find_asset "${latest_release}" "*.zip"
    get_download_url "${func_result}"
    download_file "${func_result}"

    unzip -qq "${func_result}" -d "${1}"
    rm -f "${func_result}"

    get_version_number "${latest_release}"
}

download_emuiibo () {
    get_latest_release "XorTroll" "emuiibo"
    latest_release=${func_result}

    find_asset "${latest_release}" "emuiibo*" "*.zip"
    get_download_url "${func_result}"
    download_file "${func_result}"

    unzip -qq "${func_result}" -d "${1}"
    rm -rf "${1}/ReiNX"
    rm -f "${1}/atmosphere/titles/0100000000000352/flags/boot2.flag"
    rm -f "${func_result}"

    get_version_number "${latest_release}"
}

download_goldleaf () {
    get_latest_release "XorTroll" "Goldleaf"
    latest_release=${func_result}

    find_asset "${latest_release}" "*.nro"
    get_download_url "${func_result}"
    download_file "${func_result}"

    mkdir -p "${1}/switch/Goldleaf"
    mv ${func_result} "${1}/switch/Goldleaf/Goldleaf.nro"

    get_version_number "${latest_release}"
}

download_hid_mitm () {
    get_latest_release "jakibaki" "hid-mitm"
    latest_release=${func_result}

    find_asset "${latest_release}" "hid*" "*.zip"
    get_download_url "${func_result}"
    download_file "${func_result}"

    unzip -qq "${func_result}" -d "${1}"
    rm -f "${1}/atmosphere/titles/0100000000000faf/flags/boot2.flag"
    rm -f "${func_result}"

    get_version_number "${latest_release}"
}

download_kosmos_toolbox () {
    get_latest_release "AtlasNX" "Kosmos-Toolbox"
    latest_release=${func_result}

    find_asset "${latest_release}" "*.nro"
    get_download_url "${func_result}"
    download_file "${func_result}"

    mkdir -p "${1}/switch/KosmosToolbox"
    mv "${func_result}" "${1}/switch/KosmosToolbox/KosmosToolbox.nro"
    cp "./Modules/kosmos-toolbox/config.json" "${1}/switch/KosmosToolbox/config.json"

    get_version_number "${latest_release}"
}

download_kosmos_updater () {
    get_latest_release "AtlasNX" "Kosmos-Updater"
    latest_release=${func_result}

    find_asset "${latest_release}" "*.nro"
    get_download_url "${func_result}"
    download_file "${func_result}"

    mkdir -p "${1}/switch/KosmosUpdater"
    mv ${func_result} "${1}/switch/KosmosUpdater/KosmosUpdater.nro"
    sed "s/KOSMOS_VERSION/${2}/g" "./Modules/kosmos-updater/internal.db" >> "${1}/switch/KosmosUpdater/internal.db"

    get_version_number "${latest_release}"
}

download_ldn_mitm () {
    get_latest_release "spacemeowx2" "ldn_mitm"
    latest_release=${func_result}

    find_asset "${latest_release}" "ldn_mitm*" "*.zip"
    get_download_url "${func_result}"
    download_file "${func_result}"

    unzip -qq "${func_result}" -d "${1}"
    rm -f "${1}/atmosphere/titles/4200000000000010/flags/boot2.flag"
    rm -f "${func_result}"

    get_version_number "${latest_release}"
}

download_lockpick () {
    get_latest_release "shchmue" "Lockpick"
    latest_release=${func_result}

    find_asset "${latest_release}" "*.nro"
    get_download_url "${func_result}"
    download_file "${func_result}"

    mkdir -p "${1}/switch/Lockpick"
    mv ${func_result} "${1}/switch/Lockpick/Lockpick.nro"

    get_version_number "${latest_release}"
}

download_lockpick_rcm () {
    get_latest_release "shchmue" "Lockpick_RCM"
    latest_release=${func_result}

    find_asset "${latest_release}" "*.bin"
    get_download_url "${func_result}"
    download_file "${func_result}"

    mv ${func_result} "${1}/bootloader/payloads/Lockpick_RCM.bin"

    get_version_number "${latest_release}"
}

download_sys_clk () {
    get_latest_release "retronx-team" "sys-clk"
    latest_release=${func_result}

    find_asset "${latest_release}" "sys-clk*" "*.zip"
    get_download_url "${func_result}"
    download_file "${func_result}"

    unzip -qq "${func_result}" -d "${1}"
    rm -f "${1}/atmosphere/titles/00FF0000636C6BFF/flags/boot2.flag"
    rm -f "${1}/README.html"
    rm -f "${func_result}"

    get_version_number "${latest_release}"
}

# download_sys_ftpd () {
    # Someone needs to create a release... =/
# }

# download_sys_netcheat () {
    # Someone needs to update their release to not be a kip... =/
# }

# =============================================================================
# Main Script
# =============================================================================

if [ $# -le 1 ]
then
    echo "Usage: ./kosmos.sh [version-number] [output]"
    exit 1
fi

# Build temp directory
temp_directory="/tmp/$(uuidgen)"
mkdir -p "${temp_directory}"

# Start building!

download_atmosphere "${temp_directory}"
atmosphere_version=${func_result}

download_hekate "${temp_directory}"
hekate_version=${func_result}
copy_payload "${temp_directory}"
build_hekate_files "${temp_directory}" "${1}"

download_appstore "${temp_directory}"
appstore_version=${func_result}

download_edizon "${temp_directory}"
edizon_version=${func_result}

download_emuiibo "${temp_directory}"
emuiibo_version=${func_result}

download_goldleaf "${temp_directory}"
goldleaf_version=${func_result}

download_hid_mitm "${temp_directory}"
hid_mitm_version=${func_result}

download_kosmos_toolbox "${temp_directory}"
kosmos_toolbox_version=${func_result}

download_kosmos_updater "${temp_directory}" "${1}"
kosmos_updater_version=${func_result}

download_ldn_mitm "${temp_directory}"
ldn_mitm_version=${func_result}

download_lockpick "${temp_directory}"
lockpick_version=${func_result}

download_lockpick_rcm "${temp_directory}"
lockpick_rcm_version=${func_result}

download_sys_clk "${temp_directory}"
sys_clk_version=${func_result}

# Bundle everything together.
current_directory=${PWD}
cd "${temp_directory}"
zip -q -r "${2}/Kosmos-${1}.zip" .
cd "${current_directory}"

# Clean up.
rm -rf "${temp_directory}"

# Output some useful information.
echo "Kosmos ${1} built with:"
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
