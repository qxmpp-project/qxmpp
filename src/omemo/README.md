<!--
SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>

SPDX-License-Identifier: CC0-1.0
-->

QXmpp OMEMO module
==================

The QXmpp OMEMO module adds support for OMEMO (v0.8+) end-to-end encryption by
providing the `QXmppOmemoManager`.

Dependencies
------------

 * [QCA (Qt Cryptographic Architecture)](https://invent.kde.org/libraries/qca)
 * [libomemo-c](https://github.com/dino/libomemo-c) (built with `-DBUILD_SHARED_LIBS=ON`)

Building
--------

By default QXmpp is built without the OMEMO module. To enable it you need to
provide `-DBUILD_OMEMO=ON` to cmake:

    cmake <qxmpp folder> -DBUILD_OMEMO=ON

Usage
-----

QXmpp OMEMO is available as a CMake module. You can use it like this:

```cmake
find_package(QXmpp COMPONENTS Omemo)
target_link_libraries(${PROJECT} PRIVATE QXmpp::QXmpp QXmpp::Omemo)
```

How to use the OMEMO Manager is explained in the code documentation.

Licensing issues
----------------

QXmppOmemo itself is (like the core) licensed under LGPL-2.1-or-later. However,
as of today it is linking to `libomemo-c` which uses GPL-3.0. *This means that
the resulting binaries must be used under the terms of the **GPL-3.0**.*

We might move to a different, more permissive OMEMO library in the future and
in that case no relicensing would be necessary.

