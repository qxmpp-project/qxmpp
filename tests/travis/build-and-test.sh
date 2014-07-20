#!/bin/sh
set -e

BUILD_FLAGS="QMAKE_CXXFLAGS += '-O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2'" \
            "QMAKE_LFLAGS += '-Wl,-z,relro -Wl,--as-needed'"

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
