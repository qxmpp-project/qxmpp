// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "QXmppComponentExtension.h"

QXmppComponentExtension::QXmppComponentExtension()
    : m_component(nullptr)
{
}

QXmppComponentExtension::~QXmppComponentExtension()
{
}

QXmppComponent *QXmppComponentExtension::component()
{
    return m_component;
}

void QXmppComponentExtension::setComponent(QXmppComponent *component)
{
    m_component = component;
}
