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
    for (auto child = element.firstChildElement();
         !child.isNull();
         child = child.nextSiblingElement()) {
        if (QXmppBitsOfBinaryData::isBitsOfBinaryData(child)) {
            return true;
        }
    }
    return false;
}

/// \cond
void QXmppBitsOfBinaryIq::parseElementFromChild(const QDomElement &element)
{
    for (auto child = element.firstChildElement();
         !child.isNull();
         child = child.nextSiblingElement()) {
        if (QXmppBitsOfBinaryData::isBitsOfBinaryData(child)) {
            QXmppBitsOfBinaryData::parseElementFromChild(child);
            break;
        }
    }
}

void QXmppBitsOfBinaryIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    QXmppBitsOfBinaryData::toXmlElementFromChild(writer);
}
/// \endcond
