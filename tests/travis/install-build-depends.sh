#!/bin/sh

sudo apt-get update -qq

if [ "$QT_SELECT" = "qt4" ]; then
    sudo apt-get install -qq libqt4-dev
else
    sudo add-apt-repository -y ppa:ubuntu-sdk-team/ppa
    sudo apt-get update -qq
    sudo apt-get install qtbase5-dev
fi

case "$CONFIG" in
full*)
    sudo apt-get install -qq  libspeex-dev libtheora-dev libvpx-dev
    ;;
esac
