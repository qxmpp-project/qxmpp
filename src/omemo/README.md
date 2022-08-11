QXmpp OMEMO module
==================

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

