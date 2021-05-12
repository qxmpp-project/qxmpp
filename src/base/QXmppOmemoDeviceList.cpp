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

#include "QXmppOmemoDeviceList.h"

#include <QDomElement>

#include "QXmppConstants_p.h"

///
/// \class QXmppOmemoDeviceList
///
/// \brief The QXmppOmemoDeviceList class represents an OMEMO device list
/// as defined by \xep{0384, OMEMO Encryption}.
///
/// \since QXmpp 1.5
///

class QXmppOmemoDeviceListPrivate : public QSharedData {};

///
/// Constructs an OMEMO device list.
///
QXmppOmemoDeviceList::QXmppOmemoDeviceList() : d(new QXmppOmemoDeviceListPrivate) {}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppOmemoDeviceList::QXmppOmemoDeviceList(const QXmppOmemoDeviceList &other) : d(other.d) {}

QXmppOmemoDeviceList::~QXmppOmemoDeviceList() = default;

///
/// Assigns \a other to this OMEMO device list.
///
/// \param other
///
QXmppOmemoDeviceList &QXmppOmemoDeviceList::operator=(const QXmppOmemoDeviceList &other)
{
    d = other.d;
    return *this;
}

/// \cond
void QXmppOmemoDeviceList::parse(const QDomElement &element)
{
    QDomElement device = element.firstChildElement("device");

    while (!device.isNull()) {
        QXmppOmemoDeviceListElement deviceListElement;
        deviceListElement.parse(device);
        append(deviceListElement);

        device = device.nextSiblingElement("device");
    }
}

void QXmppOmemoDeviceList::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("devices");
    writer->writeDefaultNamespace(ns_omemo_1);

    for (const auto &device : *this) {
        device.toXml(writer);
    }

    writer->writeEndElement();
}
/// \endcond

///
/// Determines whether the given DOM element is an OMEMO device list.
///
/// \param element DOM element being checked
///
/// \return true if element is an OMEMO device list, otherwise false
///
bool QXmppOmemoDeviceList::isOmemoDeviceList(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("devices") &&
        element.namespaceURI() == ns_omemo_1;
}
