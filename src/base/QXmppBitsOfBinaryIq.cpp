/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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

#include "QXmppBitsOfBinaryIq.h"

#include "QXmppConstants_p.h"

#include <QDomElement>
#include <QSharedData>

///
/// \class QXmppBitsOfBinaryIq
///
/// QXmppBitsOfBinaryIq represents a \xep{0231, Bits of Binary} IQ to request
/// and transmit Bits of Binary data elements.
///
/// \since QXmpp 1.2
///

QXmppBitsOfBinaryIq::QXmppBitsOfBinaryIq() = default;

QXmppBitsOfBinaryIq::~QXmppBitsOfBinaryIq() = default;

///
/// Returns true, if \c element is a \xep{0231, Bits of Binary} IQ
///
/// \note This may also return true, if the IQ is not a Bits of Binary IQ in
/// first place, but only contains a Bits of Binary data element.
///
bool QXmppBitsOfBinaryIq::isBitsOfBinaryIq(const QDomElement &element)
{
    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        if (QXmppBitsOfBinaryData::isBitsOfBinaryData(child))
            return true;
        child = child.nextSiblingElement();
    }
    return false;
}

/// \cond
void QXmppBitsOfBinaryIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        if (QXmppBitsOfBinaryData::isBitsOfBinaryData(child)) {
            QXmppBitsOfBinaryData::parseElementFromChild(child);
            break;
        }
        child = child.nextSiblingElement();
    }
}

void QXmppBitsOfBinaryIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    QXmppBitsOfBinaryData::toXmlElementFromChild(writer);
}
/// \endcond
