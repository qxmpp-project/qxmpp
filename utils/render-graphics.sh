#!/bin/bash

# SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

QXMPP_SOURCES=$(dirname "$(readlink -f "${0}")")/..

echo "*****************************************"
echo "Rendering graphics"
echo "*****************************************"

render_support_chat_avatar() {
    render_svg_with_margin $QXMPP_SOURCES/logo.svg $QXMPP_SOURCES/support-chat-avatar.png 300 300 18
}

# $1 - $4: see render_svg()
# $5: margin
render_svg_with_margin() {
    output_directory=$(dirname $2)
    mkdir -p $output_directory
    tmp_file=$output_directory/rendered_tmp.svg
    inkscape -o $tmp_file --export-margin=$5 $1
    render_svg $tmp_file $2 $3 $4
    rm $tmp_file
}

# $1: input file
# $2: output file
# $3: width
# $4: height
render_svg() {
    inkscape -o $2 -w $3 -h $4 $1 >/dev/null
    optipng -quiet -o7 $2 >/dev/null
    advpng -z4 $2 >/dev/null
    echo "Created "$2
}

render_support_chat_avatar
