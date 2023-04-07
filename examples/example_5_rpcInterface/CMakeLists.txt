# SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
#
# SPDX-License-Identifier: CC0-1.0

add_executable(5_rpcInterface main.cpp remoteinterface.cpp)
target_link_libraries(5_rpcInterface ${QXMPP_TARGET})
