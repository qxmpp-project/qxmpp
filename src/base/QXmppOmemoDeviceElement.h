/*
 * Copyright (C) 2008-2021 The QXmpp developers
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

#ifndef QXMPPOMEMODEVICEELEMENT_H
#define QXMPPOMEMODEVICEELEMENT_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QXmppOmemoDeviceElementPrivate;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppOmemoDeviceElement
{
public:
    QXmppOmemoDeviceElement();
    QXmppOmemoDeviceElement(const QXmppOmemoDeviceElement &other);
    ~QXmppOmemoDeviceElement();

    QXmppOmemoDeviceElement &operator=(const QXmppOmemoDeviceElement &other);

    bool operator==(const QXmppOmemoDeviceElement &other) const;

    uint32_t id() const;
    void setId(uint32_t id);

    QString label() const;
    void setLabel(const QString &label);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoDeviceElement(const QDomElement &element);

private:
    QSharedDataPointer<QXmppOmemoDeviceElementPrivate> d;
};

Q_DECLARE_TYPEINFO(QXmppOmemoDeviceElement, Q_MOVABLE_TYPE);

#endif  // QXMPPOMEMODEVICEELEMENT_H
