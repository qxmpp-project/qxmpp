<!--
SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>

SPDX-License-Identifier: CC0-1.0
-->

# DOAP rendering

Run `doc/doap-rendering/update-xeplist.sh` to get the latest XEP metadata before building the documentation.

You maybe need to open a local web server to display the list of supported XEPs in case your web browser prohibits the request to `doc/doap-rendering/doap.xsl`.
Here is an example with Python's built-in web server and Firefox:
```
python3 -m http.server
firefox http://0.0.0.0:8000/xep.html
```
