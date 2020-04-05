# QXmpp [![XMPP:2020 Client IM Mobile+ Compliance Badge](https://img.shields.io/badge/XMPP%3A2020%20Client-Core%20IM%20Mobile%2B-green)][xmpp-compliance] [![Build Status](https://github.com/qxmpp-project/qxmpp/workflows/tests/badge.svg)](https://github.com/qxmpp-project/qxmpp/actions) [![Code Coverage](https://img.shields.io/codecov/c/github/qxmpp-project/qxmpp.svg)](https://codecov.io/gh/qxmpp-project/qxmpp) [![Latest release](https://img.shields.io/github/v/release/qxmpp-project/qxmpp)](https://github.com/qxmpp-project/qxmpp/releases/latest) [![Website](https://img.shields.io/website?down_message=offline&label=documentation&up_message=online&url=https%3A%2F%2Fdoc.qxmpp.org%2F)][qxmpp-doc]

QXmpp is a cross-platform C++ XMPP client and server library. It is written
in C++ and uses Qt framework.

QXmpp strives to be as easy to use as possible, the underlying TCP socket, the
core XMPP RFCs (RFC6120 and RFC6121) and XMPP extensions have been nicely
encapsulated into classes. QXmpp is ready to build XMPP clients complying with
the [XMPP Compliance Suites 2020][xmpp-compliance] for IM and Advanced Mobile.
It comes with full API documentation, automatic tests and some examples.

QXmpp uses Qt extensively, and as such users need to a have working knowledge of
C++ and Qt basics (Signals and Slots and Qt data types).

Qt is the only third party library which is required to build QXmpp, but
libraries such as speex and theora enable additional features.

QXmpp is released under the terms of the GNU Lesser General Public License,
version 2.1 or later.

Building QXmpp
==============

QXmpp requires Qt 5.7 or higher with SSL enabled.
It uses CMake as build system.

Build from command line:

    mkdir build
    cd build
    cmake ..
    cmake --build .

You can pass the following arguments to CMake:

    BUILD_SHARED                  to build with shared type library, otherwise static (default: true)
    BUILD_DOCUMENTATION           to build the documentation (default: false)
    BUILD_EXAMPLES                to build the examples (default: true)
    BUILD_TESTS                   to build the unit tests (default: true)
    WITH_GSTREAMER                to enable audio/video over jingle (default: false)

Installing QXmpp
================

After building QXmpp, you can install the Headers, Libraries
and Documentation using the following command:

Install from command line:

    cmake --build . --target install

Examples
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

Documentation
=============

You can find the API documentation for the latest stable QXmpp version here:

https://doc.qxmpp.org/

The API documentation of the master branch is also available:

https://doc.qxmpp.org/qxmpp-dev/

Supported Platforms
===================

It should work on all the platforms supported by Qt. For a complete list of
platforms support by Qt, see:

https://doc.qt.io/qt-5/supported-platforms.html

How to report a bug
===================

If you think you have found a bug in QXmpp, we would like to hear about
it so that we can fix it. Before reporting a bug, please check if the issue
is already know at:

https://github.com/qxmpp-project/qxmpp/issues

[xmpp-compliance]: https://xmpp.org/extensions/xep-0423.html
[qxmpp-doc]: https://doc.qxmpp.org/
