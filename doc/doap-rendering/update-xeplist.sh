#!/bin/bash

# SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
#
# SPDX-License-Identifier: CC0-1.0

#
# This script updates the list that contains XEP metadata used by "doap.xsl".
#
# It should be run before generating the Doxygen documentation.
#

SCRIPT_DIR=$(dirname "$(readlink -f "${0}")")

# The list is generated by "https://github.com/xsf/xeps/blob/master/tools/extract-metadata.py".
curl -L https://xmpp.org/extensions/xeplist.xml > "${SCRIPT_DIR}/xeplist.xml"
