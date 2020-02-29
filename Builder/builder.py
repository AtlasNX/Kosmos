#!/usr/bin/env python
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

import argparse
import config
import enum
import json
import modules
import os
import uuid
import shutil


class Command(enum.Enum):
    Kosmos = 0
    SDSetup = 1

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-v', '--version',
        default=None,
        type=str,
        help='Overides the Kosmos Version from the config file.',
        metavar='KosmosVersion')
    subparsers = parser.add_subparsers()

    # Kosmos subcommands
    parser_kosmos = subparsers.add_parser(
        'kosmos',
        help='Create a release build of Kosmos.')
    parser_kosmos.add_argument(
        'output',
        help='Zip file to create.')
    parser_kosmos.set_defaults(command=Command.Kosmos)

    # SDSetup subcommands
    parser_sdsetup = subparsers.add_parser(
        'sdsetup',
        help='Create a Kosmos modules for SDSetup.')
    parser_sdsetup.add_argument(
        'output',
        help='Directory to output modules to.')
    parser_sdsetup.add_argument(
        '-a', '--auto',
        action='store_true',
        default=False,
        help='Perform an auto build.')
    parser_sdsetup.set_defaults(command=Command.SDSetup)

    # Parse arguments
    return parser.parse_args()

def create_temp_directory():
    directory = f'{os.getcwd()}/tmp/{str(uuid.uuid4())}'
    os.makedirs(directory)
    return directory

def get_kosmos_version(args):
    if args.version is not None:
        return args.version
    return config.version

def init_version_messages(args, kosmos_version):
    if args.command == Command.Kosmos:
        return [ f'Kosmos {kosmos_version} built with:' ]
    elif args.command == Command.SDSetup and not args.auto:
        return [ 'SDSetup Modules built with:' ]
    return []

def build(args, temp_directory, kosmos_version):
    results = []

    # Open up modules.json
    with open('modules.json') as json_file:
        # Parse JSON
        data = json.load(json_file)

        # Loop through modules
        for module in data:
            # Running a Kosmos Build
            if args.command == Command.Kosmos:
                # Download the module.
                print(f'Downloading {module["name"]}...')
                download = getattr(modules,
                    module['download_function_name'])
                version = download(temp_directory, kosmos_version)
                results.append(f'  {module["name"]} - {version}')

            # Running a SDSetup Build
            elif args.command == Command.SDSetup and module['include_in_sdsetup']:
                # Only show prompts when it's not an auto build.
                if not args.auto:
                    print(f'Downloading {module["name"]}...')

                # Download the module.
                version = ''
                if not args.auto or (
                    args.auto and module['include_in_sdsetup_auto_builds']):
                    download = getattr(modules,
                        module['download_function_name'])
                    version = download(temp_directory, kosmos_version)

                # Auto builds have a different prompt at the end for parsing.
                if args.auto and module['include_in_sdsetup_auto_builds']:
                    results.append(f'{module["sdsetup_auto_name"]}:{version}')
                else:
                    results.append(f'  {module["name"]} - {version}')
    return results

def delete_file_or_folder(path):
    if os.path.exists(path):
        if os.path.isfile(path):
            os.remove(path)
        else:
            shutil.rmtree(path)

if __name__ == '__main__':
    args = parse_args()
    temp_directory = create_temp_directory()
    kosmos_version = get_kosmos_version(args)

    version_messages = init_version_messages(args, kosmos_version)
    version_messages += build(args, temp_directory, kosmos_version)

    delete_file_or_folder(args.output)

    if args.command == Command.Kosmos:
        shutil.make_archive(
            os.path.splitext(args.output)[0],
            'zip',
            temp_directory)
    elif args.command == Command.SDSetup:
        shutil.move(temp_directory, args.output)

    delete_file_or_folder(f'{os.getcwd()}/tmp')

    for message in version_messages:
        print(message)
