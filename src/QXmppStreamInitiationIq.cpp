/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppStreamInitiationIq.h"
#include "QXmppUtils.h"

QString QXmppStreamInitiationIq::mimeType() const
{
    return m_mimeType;
}

void QXmppStreamInitiationIq::setMimeType(const QString &mimeType)
{
    m_mimeType = mimeType;
}

QXmppStreamInitiationIq::Profile QXmppStreamInitiationIq::profile() const
{
    return m_profile;
}

void QXmppStreamInitiationIq::setProfile(QXmppStreamInitiationIq::Profile profile)
{
    m_profile = profile;
}

QXmppElementList QXmppStreamInitiationIq::siItems() const
{
    return m_siItems;
}

QString QXmppStreamInitiationIq::siId() const
{
    return m_siId;
}

void QXmppStreamInitiationIq::setSiId(const QString &id)
{
    m_siId = id;
}

void QXmppStreamInitiationIq::setSiItems(const QXmppElementList &items)
{
    m_siItems = items;
}

bool QXmppStreamInitiationIq::isStreamInitiationIq(const QDomElement &element)
{
    QDomElement siElement = element.firstChildElement("si");
    return (siElement.namespaceURI() == ns_stream_initiation);
}

void QXmppStreamInitiationIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement siElement = element.firstChildElement("si");
    m_siId = siElement.attribute("id");
    m_mimeType = siElement.attribute("mime-type");
    if (siElement.attribute("profile") == ns_stream_initiation_file_transfer)
        m_profile = FileTransfer;
    else
        m_profile = None;

    QDomElement itemElement = siElement.firstChildElement();
    while (!itemElement.isNull())
    {
        m_siItems.append(QXmppElement(itemElement));
        itemElement = itemElement.nextSiblingElement();
    }
}

void QXmppStreamInitiationIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("si");
    writer->writeAttribute("xmlns", ns_stream_initiation);
    helperToXmlAddAttribute(writer, "id", m_siId);
    helperToXmlAddAttribute(writer, "mime-type", m_mimeType);
    if (m_profile == FileTransfer)
        helperToXmlAddAttribute(writer, "profile", ns_stream_initiation_file_transfer);
    foreach (const QXmppElement &item, m_siItems)
        item.toXml(writer);
    writer->writeEndElement();
}

