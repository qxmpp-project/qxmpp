# SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)

add_executable(9_vCard example_9_vCard.cpp)
target_link_libraries(9_vCard Qt${QT_VERSION_MAJOR}::Gui ${QXMPP_TARGET})
