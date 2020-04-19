#!/usr/bin/env python3
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
import common
import config
import modules
import os
import shutil
import sys

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
    parser_kosmos = subparsers.add_parser('kosmos', help='Create a release build of Kosmos.')
    parser_kosmos.add_argument('output', help='Zip file to create.')
    parser_kosmos.set_defaults(command=common.Command.Kosmos)

    # SDSetup subcommands
    parser_sdsetup = subparsers.add_parser('sdsetup', help='Create a Kosmos modules for SDSetup.')
    parser_sdsetup.add_argument('output', help='Directory to output modules to.')
    parser_sdsetup.add_argument(
        '-a', '--auto',
        action='store_true',
        default=False,
        help='Perform an auto build.')
    parser_sdsetup.set_defaults(command=common.Command.SDSetup)

    # Parse arguments
    args = parser.parse_args()

    if not hasattr(args, 'command'):
        parser.print_help()
        sys.exit()

    return args

def get_kosmos_version(args):
    if args.version is not None:
        return args.version
    return config.version

def init_version_messages(args, kosmos_version):
    if args.command == common.Command.Kosmos:
        return [ f'Kosmos {kosmos_version} built with:' ]
    elif args.command == common.Command.SDSetup and not args.auto:
        return [ 'SDSetup Modules built with:' ]
    return []

if __name__ == '__main__':
    args = parse_args()

    temp_directory = common.generate_temp_path()
    os.makedirs(temp_directory)
    kosmos_version = get_kosmos_version(args)

    auto_build = False
    if hasattr(args, 'auto'):
        auto_build = args.auto

    version_messages = init_version_messages(args, kosmos_version)

    is_kosmos_build = args.command == common.Command.Kosmos
    build_messages = modules.build(temp_directory, kosmos_version, is_kosmos_build, auto_build)

    common.delete_path(args.output)

    if build_messages is not None:
        version_messages += build_messages
        
        if is_kosmos_build:
            shutil.make_archive(
                os.path.splitext(args.output)[0],
                'zip',
                temp_directory)
        else:
            shutil.move(temp_directory, args.output)

        common.delete_path(os.path.join(os.getcwd(), 'tmp'))

        for message in version_messages:
            print(message)
    else:
        common.delete_path(os.path.join(os.getcwd(), 'tmp'))
