/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Germán Márquez Mejía
 *  Melvin Keskin
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifndef QXMPPOMEMODEVICELIST_H
#define QXMPPOMEMODEVICELIST_H

#include "QXmppGlobal.h"

#include "QList"

class QDomElement;
class QXmlStreamWriter;
class QXmppOmemoDeviceElement;

class QXMPP_EXPORT QXmppOmemoDeviceList : public QList<QXmppOmemoDeviceElement>
{
public:
    QXmppOmemoDeviceList();
    QXmppOmemoDeviceList(const QXmppOmemoDeviceList &other);
    ~QXmppOmemoDeviceList();

    QXmppOmemoDeviceList &operator=(const QXmppOmemoDeviceList &other);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoDeviceList(const QDomElement &element);
};

Q_DECLARE_TYPEINFO(QXmppOmemoDeviceList, Q_MOVABLE_TYPE);

#endif  // QXMPPOMEMODEVICELIST_H
