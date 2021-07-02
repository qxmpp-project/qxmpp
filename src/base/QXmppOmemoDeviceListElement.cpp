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

#include "QXmppOmemoDeviceListElement.h"

#include <QDomElement>

#include "QXmppConstants_p.h"

///
/// \class QXmppOmemoDeviceListElement
///
/// \brief The QXmppOmemoDeviceListElement class represents an element of the
/// OMEMO device list as defined by \xep{0384, OMEMO Encryption}.
///
/// \since QXmpp 1.5
///

class QXmppOmemoDeviceListElementPrivate : public QSharedData
{
public:
    int id = 0;
    QString label;
};

///
/// Constructs an OMEMO device list element.
///
QXmppOmemoDeviceListElement::QXmppOmemoDeviceListElement() : d(new QXmppOmemoDeviceListElementPrivate) {}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppOmemoDeviceListElement::QXmppOmemoDeviceListElement(const QXmppOmemoDeviceListElement &other) : d(other.d) {}

QXmppOmemoDeviceListElement::~QXmppOmemoDeviceListElement() = default;

///
/// Assigns \a other to this OMEMO device list element.
///
/// \param other
///
QXmppOmemoDeviceListElement &QXmppOmemoDeviceListElement::operator=(const QXmppOmemoDeviceListElement &other)
{
    d = other.d;
    return *this;
}

///
/// Returns the ID of this device list element.
///
/// The ID is used to identify a device and fetch its bundle.
/// The ID is 0 if it is unset.
///
/// \see QXmppOmemoDeviceBundle
///
/// \return this device list element's ID
///
int QXmppOmemoDeviceListElement::id() const
{
    return d->id;
}

///
/// Sets the ID of this device list element.
///
/// The ID must be a positive value, otherwise it is ignored.
///
/// \param id this device list element's ID
///
void QXmppOmemoDeviceListElement::setId(const int id)
{
    if (id > 0) {
        d->id = id;
    }
}

///
/// Returns the label of this device list element.
///
/// The label is a human-readable string used to identify the device by users.
/// If no label is set, a default-constructed QString is returned.
///
/// \return this device list element's label
///
QString QXmppOmemoDeviceListElement::label() const
{
    return d->label;
}

///
/// Sets the optional label of this device list element.
///
/// The label should not contain more than 53 characters.
///
/// \param label this device list element's label
///
void QXmppOmemoDeviceListElement::setLabel(const QString &label)
{
    d->label = label;
}

/// \cond
void QXmppOmemoDeviceListElement::parse(const QDomElement &element)
{
    d->id = element.attribute("id").toInt();
    d->label = element.attribute("label");
}

void QXmppOmemoDeviceListElement::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("device");

    writer->writeAttribute("id", QString::number(d->id));
    if (!d->label.isEmpty()) {
        writer->writeAttribute("label", d->label);
    }

    writer->writeEndElement(); // device
}
/// \endcond

///
/// Determines whether the given DOM element is an OMEMO device list element.
///
/// \param element DOM element being checked
///
/// \return true if element is an OMEMO device list element, otherwise false
///
bool QXmppOmemoDeviceListElement::isOmemoDeviceListElement(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("device") &&
        element.namespaceURI() == ns_omemo_1;
}
