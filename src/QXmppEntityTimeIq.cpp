/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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


#include "QXmppEntityTimeIq.h"

#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppUtils.h"

/// Returns the timezone offset in seconds.
///

int QXmppEntityTimeIq::tzo() const
{
    return m_tzo;
}

/// Sets the timezone offset in seconds.
///
/// \param tzo

void QXmppEntityTimeIq::setTzo(int tzo)
{
    m_tzo = tzo;
}

/// Returns the date/time in Coordinated Universal Time (UTC).
///

QDateTime QXmppEntityTimeIq::utc() const
{
    return m_utc;
}

/// Sets the date/time in Coordinated Universal Time (UTC).
///
/// \param utc

void QXmppEntityTimeIq::setUtc(const QDateTime &utc)
{
    m_utc = utc;
}

bool QXmppEntityTimeIq::isEntityTimeIq(const QDomElement &element)
{
    QDomElement timeElement = element.firstChildElement("time");
    return timeElement.namespaceURI() == ns_entity_time;
}

void QXmppEntityTimeIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement timeElement = element.firstChildElement("time");
    m_tzo = timezoneOffsetFromString(timeElement.firstChildElement("tzo").text());
    m_utc = datetimeFromString(timeElement.firstChildElement("utc").text());
}

void QXmppEntityTimeIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("time");
    writer->writeAttribute("xmlns", ns_entity_time);

    if(m_utc.isValid())
    {
        helperToXmlAddTextElement(writer, "tzo", timezoneOffsetToString(m_tzo));
        helperToXmlAddTextElement(writer, "utc", datetimeToString(m_utc));
    }
    writer->writeEndElement();
}
