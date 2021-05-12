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

#ifndef QXMPPOMEMODEVICELISTELEMENT_H
#define QXMPPOMEMODEVICELISTELEMENT_H

#include <QSharedDataPointer>

#include "QXmppElement.h"

class QXmppOmemoDeviceListElementPrivate;

class QXMPP_EXPORT QXmppOmemoDeviceListElement
{
public:
    QXmppOmemoDeviceListElement();
    QXmppOmemoDeviceListElement(const QXmppOmemoDeviceListElement &other);
    ~QXmppOmemoDeviceListElement();

    QXmppOmemoDeviceListElement& operator=(const QXmppOmemoDeviceListElement &other);

    int id() const;
    void setId(const int id);

    QString label() const;
    void setLabel(const QString &label);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoDeviceListElement(const QDomElement &element);

private:
    QSharedDataPointer<QXmppOmemoDeviceListElementPrivate> d;
};

inline bool operator==(const QXmppOmemoDeviceListElement &e1, const QXmppOmemoDeviceListElement &e2)
{
    return e1.id() == e2.id();
}

Q_DECLARE_TYPEINFO(QXmppOmemoDeviceListElement, Q_MOVABLE_TYPE);

#endif // QXMPPOMEMODEVICELISTELEMENT_H
