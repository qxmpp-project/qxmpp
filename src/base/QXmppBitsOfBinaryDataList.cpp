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

#include "QXmppBitsOfBinaryDataList.h"

#include "QXmppBitsOfBinaryData.h"
#include "QXmppConstants_p.h"

#include <QDomElement>

QXmppBitsOfBinaryDataList::QXmppBitsOfBinaryDataList() = default;

QXmppBitsOfBinaryDataList::~QXmppBitsOfBinaryDataList() = default;

/// \cond
void QXmppBitsOfBinaryDataList::parse(const QDomElement &element)
{
    // clear previous data elements
    clear();

    // parse all <data/> elements
    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        if (QXmppBitsOfBinaryData::isBitsOfBinaryData(child)) {
            QXmppBitsOfBinaryData data;
            data.parseElementFromChild(child);
            append(data);
        }
        child = child.nextSiblingElement();
    }
}

void QXmppBitsOfBinaryDataList::toXml(QXmlStreamWriter *writer) const
{
    for (const auto &bitsOfBinaryData : *this)
        bitsOfBinaryData.toXmlElementFromChild(writer);
}
/// \endcond
