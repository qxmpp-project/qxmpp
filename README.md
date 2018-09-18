[![Build Status](https://img.shields.io/travis/qxmpp-project/qxmpp.svg)](https://travis-ci.org/qxmpp-project/qxmpp)
[![Code Coverage](https://img.shields.io/codecov/c/github/qxmpp-project/qxmpp.svg)](https://codecov.io/gh/qxmpp-project/qxmpp)

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

QXmpp requires Qt 5.0 or higher with SSL enabled.
It uses CMake as build system.

Build from command line:

    mkdir build
    cd build
    cmake ..
    cmake --build .

You can pass the following arguments to CMake:

    BUILD_DOCUMENTATION           to build the documentation
    BUILD_EXAMPLES                to build the examples
    BUILD_TESTS                   to build the unit tests
    WITH_OPUS                     to enable opus audio codec
    WITH_SPEEX                    to enable speex audio codec
    WITH_THEORA                   to enable theora video codec
    WITH_VPX                      to enable vpx video codec

INSTALLING QXMPP
================

After building QXmpp, you can install the Headers, Libraries
and Documentation using the following command:

Install from command line:

    cmake --build . --target install

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

http://doc.qxmpp.org/

SUPPORTED PLATFORMS
===================

It should work on all the plaforms supported by Qt. For a complete list of
platforms support by Qt, see:

https://doc.qt.io/qt-5/supported-platforms.html

HOW TO REPORT A BUG
===================

If you think you have found a bug in QXmpp, we would like to hear about
it so that we can fix it. Before reporting a bug, please check if the issue
is already know at:

https://github.com/qxmpp-project/qxmpp/issues

DISCUSSION GROUP
================

Join QXmpp Discussion Group for queries, discussions and updates.

https://groups.google.com/forum/#!forum/qxmpp
