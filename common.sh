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

# =============================================================================
# General Functions
# =============================================================================

# Test username and password against GitHub's API
# Params:
#   - GitHub Login
# Returns:
#   Whether it worked or not.
test_login () {
    response=$(curl -u ${1} -H  "Accept: application/json" -H "Content-Type: application/json" -H "User-Agent: Kosmos/1.0.0" -s https://api.github.com/)
    message_index=$(echo ${response} | jq 'keys | index("message")')
    if [ "${message_index}" == "null" ]
    then
        echo "1"
    else
        echo "0"
    fi
}

# Downloads the latest release JSON.
# Params:
#   - GitHub Login
#   - GitHub Company
#   - GitHub Repo
# Returns:
#   The latest release JSON.
get_latest_release () {
    if [ -z "${1}" ] 
    then
        echo $(curl -H "Accept: application/json" -H "Content-Type: application/json" -H "User-Agent: Kosmos/1.0.0" -s https://api.github.com/repos/${2}/${3}/releases | jq -r '.[0]')
    else
        echo $(curl -u ${1} -H "Accept: application/json" -H "Content-Type: application/json" -H "User-Agent: Kosmos/1.0.0" -s https://api.github.com/repos/${2}/${3}/releases | jq -r '.[0]')
    fi
}

# Gets the number of assets in a release.
# Params:
#   - The release JSON
# Returns:
#   The number of assets.
_get_number_of_assets () {
    return $(echo ${1} | jq -r '.assets | length')
}

# Finds a specific asset in a release.
# Params:
#   - The release JSON
#   - Start with blob pattern
#   - Ends with blob pattern
# Returns:
#   The asset JSON.
find_asset () {
    _get_number_of_assets "${1}"
    number_of_assets=${?}

    for (( i=0; i<${number_of_assets}; i++ ))
    do
        name=$(echo ${1} | jq -r ".assets[${i}].name" | tr '[:upper:]' '[:lower:]')
        asset=$(echo ${1} | jq -r ".assets[${i}]")

        if [[ ${#} -eq 2 && ${name} == ${2} ]]
        then
            echo ${asset}
            break
        fi

        if [[ ${#} -eq 3 && ${name} == ${2} && ${name} == ${3} ]]
        then
            echo ${asset}
            break
        fi
    done
}

# Downloads a file.
# Params:
#   - The release asset JSON
# Returns:
#   The file path.
download_file () {
    url=$(echo ${1} | jq -r ".browser_download_url")
    echo $(download_file_url "${url}")
}

# Downloads a file from URL.
# Params:
#   - URL
# Returns:
#   The file path.
download_file_url () {
    file="/tmp/$(uuidgen)"
    curl -L -H "User-Agent: ${user_agent}" -s ${1} >> ${file}
    echo ${file}
}

# Gets the version number from an asset.
# Params:
#   - The release asset JSON
# Returns:
#   The version number.
get_version_number () {
    echo $(echo ${1} | jq -r ".tag_name")
}

# Find path matching a pattern
# Params:
#   - The pattern
# Returns:
#   The first file found.
glob () {
    files=( ${1} )
    echo ${files[0]}
}

# =============================================================================
# Main Script
# =============================================================================

if [ $# -le 1 ]
then
    echo "This is not meant to be called by end users and is used by the kosmos.sh and sdsetup.sh scripts."
    exit 1
fi

# Check if the function exists (bash specific)
if declare -f "$1" > /dev/null
then
  # call arguments verbatim
  "$@"
else
  # Show a helpful error
  echo "'$1' is not a known function name" >&2
  exit 1
fi
