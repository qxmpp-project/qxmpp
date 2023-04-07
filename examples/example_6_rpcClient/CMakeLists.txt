# SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
#
# SPDX-License-Identifier: CC0-1.0

add_executable(6_rpcClient main.cpp rpcClient.cpp)
target_link_libraries(6_rpcClient ${QXMPP_TARGET})
