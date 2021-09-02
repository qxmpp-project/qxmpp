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

#include "QXmppConstants_p.h"
#include "QXmppOmemoDeviceElement.h"
#include "QXmppUtils.h"

#include <QDomElement>

///
/// \class QXmppOmemoDeviceElement
///
/// \brief The QXmppOmemoDeviceElement class represents an element of the
/// OMEMO device list as defined by \xep{0384, OMEMO Encryption}.
///
/// \since QXmpp 1.5
///

class QXmppOmemoDeviceElementPrivate : public QSharedData
{
public:
    uint32_t id = 0;
    QString label;
};

///
/// Constructs an OMEMO device element.
///
QXmppOmemoDeviceElement::QXmppOmemoDeviceElement()
    : d(new QXmppOmemoDeviceElementPrivate)
{
}

bool QXmppOmemoDeviceElement::operator==(const QXmppOmemoDeviceElement &other) const
{
    return d->id == other.id();
}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppOmemoDeviceElement::QXmppOmemoDeviceElement(const QXmppOmemoDeviceElement &other) = default;

QXmppOmemoDeviceElement::~QXmppOmemoDeviceElement() = default;

///
/// Assigns \a other to this OMEMO device element.
///
/// \param other
///
QXmppOmemoDeviceElement &QXmppOmemoDeviceElement::operator=(const QXmppOmemoDeviceElement &other) = default;

///
/// Returns the ID of this device element.
///
/// The ID is used to identify a device and fetch its bundle.
/// The ID is 0 if it is unset.
///
/// \see QXmppOmemoDeviceBundle
///
/// \return this device element's ID
///
uint32_t QXmppOmemoDeviceElement::id() const
{
    return d->id;
}

///
/// Sets the ID of this device element.
///
/// A valid ID must be at least 1 and at most 2^32-1.
///
/// \param id this device element's ID
///
void QXmppOmemoDeviceElement::setId(const uint32_t id)
{
    d->id = id;
}

///
/// Returns the label of this device element.
///
/// The label is a human-readable string used to identify the device by users.
/// If no label is set, a default-constructed QString is returned.
///
/// \return this device element's label
///
QString QXmppOmemoDeviceElement::label() const
{
    return d->label;
}

///
/// Sets the optional label of this device element.
///
/// The label should not contain more than 53 characters.
///
/// \param label this device element's label
///
void QXmppOmemoDeviceElement::setLabel(const QString &label)
{
    d->label = label;
}

/// \cond
void QXmppOmemoDeviceElement::parse(const QDomElement &element)
{
    d->id = element.attribute("id").toInt();
    d->label = element.attribute("label");
}

void QXmppOmemoDeviceElement::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("device");

    writer->writeAttribute("id", QString::number(d->id));
    if (!d->label.isEmpty()) {
        writer->writeAttribute("label", d->label);
    }

    writer->writeEndElement();  // device
}
/// \endcond

///
/// Determines whether the given DOM element is an OMEMO device element.
///
/// \param element DOM element being checked
///
/// \return true if element is an OMEMO device element, otherwise false
///
bool QXmppOmemoDeviceElement::isOmemoDeviceElement(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("device") &&
        element.namespaceURI() == ns_omemo_1;
}
