[![Build Status](https://travis-ci.org/qxmpp-project/qxmpp.png)](https://travis-ci.org/qxmpp-project/qxmpp)

ABOUT QXMPP
===========

QXmpp is a cross-platform C++ XMPP client and server library. It is written
in C++ and uses Qt framework.

QXmpp strives to be as easy to use as possible, the underlying TCP socket,
the core XMPP RFCs (RFC3920 and RFC3921) and XMPP extensions have been
nicely encapsulated into classes. QXmpp comes with full API
documentation, automatic tests and many examples.

QXmpp uses Qt extensively, and as such users need to a have working knowledge
of C++ and Qt basics (Signals and Slots and Qt data types).

Qt is the only third party library which is required to build QXmpp, but
libraries such as speex and theora enable additional features.

QXmpp is released under the terms of the GNU Lesser General Public License,
version 2.1 or later.

BUILDING QXMPP
==============

QXmpp requires Qt 4.5 or higher (with SSL enabled) and it uses
the standard qmake build system of Qt.

Build from command line:

    cd <where qxmpp.pro is located>
    qmake <arguments>
    <respective-make-cmd = gmake, make, mingw32-make, nmake>

You can pass the following arguments to qmake:

    PREFIX=<prefix>               to change the install prefix
                                  default:
                                      unix:  /usr/local on unix
                                      other: $$[QT_INSTALL_PREFIX]
    QXMPP_AUTOTEST_INTERNAL=1     to enabled internal autotests
    QXMPP_LIBRARY_TYPE=staticlib  to build a static version of QXmpp
    QXMPP_USE_DOXYGEN=1           to build the HTML documentation
    QXMPP_USE_OPUS=1              to enable opus audio codec
    QXMPP_USE_SPEEX=1             to enable speex audio codec
    QXMPP_USE_THEORA=1            to enable theora video codec
    QXMPP_USE_VPX=1               to enable vpx video codec

Note: by default QXmpp is built as a shared library. If you decide to build
a static library instead, you will need to pass -DQXMPP_STATIC when building
your programs against QXmpp.

Build using Qt Creator:

Open the qxmpp.pro file in Qt Creator and hit "Build All" to build all
the examples and library.

INSTALLING QXMPP
================

After building QXmpp, you can install the Headers, Libraries
and Documentation using the following command:

Install from command line:

    <respective-make-cmd = gmake, make, mingw32-make, nmake> install

Path of installations:

    Headers:            PREFIX/include/qxmpp
    Library:            PREFIX/lib
    API Documentation:  PREFIX/share/doc/qxmpp

To link against the shared version of QXmpp, you need to add -DQXMPP_SHARED
to your C++ flags.

EXAMPLES
========

Look at the example directory for various examples. Here is a description of
a few.

* *example_0_connected*
This example just connects to the xmpp server and start receiving presences
(updates) from the server. After running this example, you can see this user
online, if it's added in your roster (friends list).

* *example_1_echoClient*
This is a very simple bot which echoes the message sent to it. Run this
example, send it a message from a friend of this bot and you will
receive the message back. This example shows how to receive and send messages.

* *GuiClient*
This is a full fledged Graphical XMPP client. This example will uses most of
the part of this library.

DOCUMENTATION
=============

You can find the API documentation for the latest QXmpp version here:

http://doc.qxmpp.org/qxmpp-snapshot/

SUPPORTED PLATFORMS
===================

It should work on all the plaforms supported by Qt. For a complete list of
platforms support by Qt, see:

http://qt-project.org/doc/supported-platforms.html

In past, we have tested on variety of platforms:

    win32-g++        (Qt SDK)
    win32-msvc2008   (Qt MSVC-2008)
    win64-msvc2008   (Qt MSVC-2008)
    symbian-gcce     (Nokia Qt SDK)
    linux-g++        (32-bit and 64-bit)
    macos-g++        (32-bit and 64-bit)

Please note that on Symbian, you will need to make sure your add the
"NetworkServices" to your application to enable it to access the network.
You can do this by adding the following to your .pro file:

    TARGET.CAPABILITY = "NetworkServices"

HOW TO REPORT A BUG
===================

If you think you have found a bug in QXmpp, we would like to hear about
it so that we can fix it. Before reporting a bug, please check if the issue
is already know at:

https://github.com/qxmpp-project/qxmpp/issues

DISCUSSION GROUP
================

Join QXmpp Discussion Group for queries, discussions and updates.

http://groups.google.com/group/qxmpp
