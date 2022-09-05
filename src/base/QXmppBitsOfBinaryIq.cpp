// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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
        if (QXmppBitsOfBinaryData::isBitsOfBinaryData(child)) {
            return true;
        }
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
