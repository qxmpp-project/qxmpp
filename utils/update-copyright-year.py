#!/usr/bin/env python3
#
# Copyright (C) 2008-2022 The QXmpp developers
#
# Author:
#  Melvin Keskin <melvo@olomono.de>
#
# Source:
#  https://github.com/qxmpp-project/qxmpp
#
# This file is a part of QXmpp library.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
#/

"""
This script updates the year in the copyright headers of files.

It should be run every new year but not before because it uses the current year as the new one.
"""

import logging
from datetime import datetime
from pathlib import Path
import re

# Use "logging.DEBUG" for debug output.
LOG_LEVEL = logging.DEBUG

# relative path from this script's directory to the root directory of SEARCHED_DIRECTORIES
RELATIVE_ROOT_DIRECTORY_PATH = ".."
# relative paths from RELATIVE_ROOT_DIRECTORY_PATH to the uppermost directories of the checked and maybe updated files
SEARCHED_DIRECTORIES = ["src", "tests", "utils"]

# template which us used to create the new copyright string by replacing "%s" with the current year
COPYRIGHT_STRING = "Copyright (C) 2008-%s The QXmpp developers"
COPYRIGHT_REGULAR_EXPRESSION = "Copyright \(C\) (\d{4}-)?\d{4} The QXmpp developers"

logging.basicConfig(level=LOG_LEVEL)

script_directory_path = Path(__file__).resolve().parent
logging.debug("Determined path of this script's directory: %s" % script_directory_path)
root_directory_path = Path(script_directory_path, RELATIVE_ROOT_DIRECTORY_PATH).resolve()
logging.debug("Determined path of searched directories' root: %s" % root_directory_path)

current_year = datetime.now().year
logging.debug("Set new copyright year to %s" % current_year)
new_copyright_string = COPYRIGHT_STRING % current_year
logging.debug("Generated copyright string: %s" % new_copyright_string)

for searched_directory in SEARCHED_DIRECTORIES:
	for node in sorted(root_directory_path.glob("%s/**/*" % searched_directory)):
		logging.debug("Checking %s" % node)

		if node.is_file():
			try:
				with open(node) as file:
					old_content = file.read()
					new_content = re.sub(COPYRIGHT_REGULAR_EXPRESSION, new_copyright_string, old_content)
			except UnicodeDecodeError:
				# Skip binary files.
				pass

			if new_content != old_content:
				with open(node, "w") as file:
					file.write(new_content)
					logging.info("Updated %s" % node)
