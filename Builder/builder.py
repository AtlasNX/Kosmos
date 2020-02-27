#!/usr/bin/env python
"""Kosmos Builder v3.0.0

Usage:
    builder.py kosmos <output> [--version=<version>]
    builder.py sdsetup <output> [--version=<version>] [-a | --auto]
    builder.py (-h | --help)

Options:
    -h --help               Show this screen.
    --version=<version>     Overides the Kosmos Version from the config file.
    -a --auto               Perform an auto build.

"""

from docopt import docopt
from modules import Modules
from zipfile import ZipFile 


if __name__ == '__main__':
    arguments = docopt(__doc__)
