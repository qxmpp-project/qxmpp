<!--
SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>

SPDX-License-Identifier: CC0-1.0
-->
<h1 align="center">
<picture>
    <img alt="QXmpp logo" src="./logo.svg" height="150">
</picture>
</h1>
<p align="center">
    <a href="https://xmpp.org/extensions/xep-0459.html">
        <img alt="XMPP:2022 Client IM Mobile+ Compliance Badge" src="https://img.shields.io/badge/XMPP%3A2022%20Client-Core%20IM%20Mobile%2B-green">
    </a>
</p>
<p align="center">
    <a href="https://github.com/qxmpp-project/qxmpp/actions">
        <img alt="Build Status" src="https://github.com/qxmpp-project/qxmpp/workflows/tests/badge.svg">
    </a>
    <a href="https://codecov.io/gh/qxmpp-project/qxmpp">
        <img alt="Code Coverage" src="https://img.shields.io/codecov/c/github/qxmpp-project/qxmpp.svg">
    </a>
    <a href="https://github.com/qxmpp-project/qxmpp/releases/latest">
        <img alt="Latest release" src="https://img.shields.io/github/v/release/qxmpp-project/qxmpp">
    </a>
    <a href="https://doc.qxmpp.org/">
        <img alt="Documentation" src="https://img.shields.io/website?down_message=offline&label=documentation&up_message=online&url=https%3A%2F%2Fdoc.qxmpp.org%2F">
    </a>
    <a href="xmpp:qxmpp@muc.kaidan.im?join">
        <img alt="Development Chat" src="https://search.jabbercat.org/api/1.0/badge?address=qxmpp@muc.kaidan.im">
    </a>
    <a href="https://liberapay.com/QXmpp/donate">
        <img alt="Donate using Liberapay" src="https://img.shields.io/liberapay/patrons/QXmpp.svg?logo=liberapay">
    </a>
</p>

QXmpp is a cross-platform C++ XMPP client and server library. It is written
in C++ and uses Qt framework.

QXmpp strives to be as easy to use as possible, the underlying TCP socket, the
core XMPP RFCs (RFC6120 and RFC6121) and XMPP extensions have been nicely
encapsulated into classes. QXmpp is ready to build XMPP clients complying with
the [XMPP Compliance Suites 2022][xmpp-compliance] for IM and Advanced Mobile.
It comes with full API documentation, automatic tests and some examples.

QXmpp uses Qt extensively, and as such users need to a have working knowledge of
C++ and Qt basics (Signals and Slots and Qt data types).

Qt is the only third party library which is required to build QXmpp, but
libraries such as GStreamer enable additional features.

QXmpp is released under the terms of the GNU Lesser General Public License,
version 2.1 or later.

Building QXmpp
==============

QXmpp requires Qt 5.15 or Qt 6.0 or higher with SSL enabled.
It uses CMake as build system.

Build from command line:

    mkdir build
    cd build
    cmake ..
    cmake --build .

You can pass the following arguments to CMake:

    BUILD_SHARED                  to build with shared type library, otherwise static (default: true)
    BUILD_DOCUMENTATION           to build the documentation, requires Doxygen (default: false)
    BUILD_EXAMPLES                to build the examples (default: true)
    BUILD_TESTS                   to build the unit tests (default: true)
    BUILD_INTERNAL_TESTS          to build the unit tests testing private parts of the API (default: false)
    BUILD_OMEMO                   to build the OMEMO module (default: false)
    WITH_GSTREAMER                to enable audio/video over jingle (default: false)
    QT_VERSION_MAJOR=5/6          to build with a specific Qt major version (default behaviour: prefer 6)

For building the OMEMO module [additional dependencies](src/omemo/README.md)
are required.

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

[xmpp-compliance]: https://xmpp.org/extensions/xep-0459.html
