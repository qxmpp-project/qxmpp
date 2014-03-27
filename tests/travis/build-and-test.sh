#!/bin/sh

BUILD_FLAGS="QMAKE_CXXFLAGS += '-O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2'" \
            "QMAKE_LFLAGS += '-Wl,-z,relro -Wl,--as-needed'"

if [ "$CONFIG" = "full" ]; then
    qmake $BUILD_FLAGS QXMPP_USE_SPEEX=1 QXMPP_USE_THEORA=1
else
    qmake $BUILD_FLAGS QXMPP_USE_SPEEX="" QXMPP_USE_THEORA=""
fi

make VERBOSE=1

tests/run.py
