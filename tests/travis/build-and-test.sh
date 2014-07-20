#!/bin/sh
set -e

BUILD_FLAGS=""
case "$CONFIG" in
full*)
    BUILD_FLAGS="$BUILD_FLAGS QXMPP_USE_SPEEX=1 QXMPP_USE_THEORA=1"
    ;;
esac

case "$CONFIG" in
*static)
    BUILD_FLAGS="$BUILD_FLAGS QXMPP_LIBRARY_TYPE=staticlib"
    ;;
esac

qmake $BUILD_FLAGS
make VERBOSE=1

tests/run.py
