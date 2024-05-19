// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppEntityTimeIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

///
/// Returns the timezone offset in seconds.
///
int QXmppEntityTimeIq::tzo() const
{
    return m_tzo;
}

///
/// Sets the timezone offset in seconds.
///
/// \param tzo
///
void QXmppEntityTimeIq::setTzo(int tzo)
{
    m_tzo = tzo;
}

///
/// Returns the date/time in Coordinated Universal Time (UTC).
///
QDateTime QXmppEntityTimeIq::utc() const
{
    return m_utc;
}

///
/// Sets the date/time in Coordinated Universal Time (UTC).
///
/// \param utc
///
void QXmppEntityTimeIq::setUtc(const QDateTime &utc)
{
    m_utc = utc;
}

///
/// Returns true, if the element is a valid entity time IQ.
///
bool QXmppEntityTimeIq::isEntityTimeIq(const QDomElement &element)
{
    return isIqType(element, u"time", ns_entity_time);
}

/// \cond
bool QXmppEntityTimeIq::checkIqType(const QString &tagName, const QString &xmlns)
{
    return tagName == u"time" && xmlns == ns_entity_time;
}

void QXmppEntityTimeIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement timeElement = firstChildElement(element, u"time");
    m_tzo = QXmppUtils::timezoneOffsetFromString(firstChildElement(timeElement, u"tzo").text());
    m_utc = QXmppUtils::datetimeFromString(firstChildElement(timeElement, u"utc").text());
}

void QXmppEntityTimeIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("time"));
    writer->writeDefaultNamespace(toString65(ns_entity_time));

    if (m_utc.isValid()) {
        writeXmlTextElement(writer, u"tzo", QXmppUtils::timezoneOffsetToString(m_tzo));
        writeXmlTextElement(writer, u"utc", QXmppUtils::datetimeToString(m_utc));
    }
    writer->writeEndElement();
}
/// \endcond
