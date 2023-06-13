// SPDX-FileCopyrightText: 2023 Matthieu Volat <mazhe@alkumuna.eu>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMucHistory.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>

QXmppMucHistory::QXmppMucHistory()
    : m_maxchars(-1),
      m_maxstanzas(-1),
      m_seconds(-1)
{
}

/// Returns true if the history is not configured (is null).

bool QXmppMucHistory::isNull() const
{
    return m_maxchars < 0 &&
        m_maxstanzas < 0 &&
        m_seconds < 0 &&
        !m_since.isValid();
}

/// Returns the character limit of the room history.

int QXmppMucHistory::maxchars() const
{
    return m_maxchars;
}

/// Sets the character limit of the room history.

void QXmppMucHistory::setMaxchars(int maxchars)
{
    m_maxchars = maxchars;
}

/// Returns the stanza limit of the room history.

int QXmppMucHistory::maxstanzas() const
{
    return m_maxstanzas;
}

/// Sets the stanza limit of the room history.

void QXmppMucHistory::setMaxstanzas(int maxstanzas)
{
    m_maxstanzas = maxstanzas;
}

/// Returns the seconds limit of the room history.

int QXmppMucHistory::seconds() const
{
    return m_seconds;
}

/// Sets the seconds limit of the room history.

void QXmppMucHistory::setSeconds(int seconds)
{
    m_seconds = seconds;
}

/// Returns the datetime limit of the room history.

QDateTime QXmppMucHistory::since() const
{
    return m_since;
}

/// Sets the datetime limit of the room history.

void QXmppMucHistory::setSince(QDateTime &since)
{
    m_since = since;
}

/// \cond
void QXmppMucHistory::parse(const QDomElement &element)
{
    m_maxchars = element.attribute(QStringLiteral("maxchars")).toUInt();
    m_maxstanzas = element.attribute(QStringLiteral("maxstanzas")).toUInt();
    m_seconds = element.attribute(QStringLiteral("seconds")).toUInt();
    m_since = QXmppUtils::datetimeFromString(element.attribute(QStringLiteral("since")));
}

void QXmppMucHistory::toXml(QXmlStreamWriter *writer) const
{
    if (isNull()) {
        return;
    }
    writer->writeStartElement(QStringLiteral("history"));
    if (m_maxchars >= 0) {
        helperToXmlAddAttribute(writer, QStringLiteral("maxchars"), QString::number(m_maxchars));
    }
    if (m_maxstanzas >= 0) {
        helperToXmlAddAttribute(writer, QStringLiteral("maxstanzas"), QString::number(m_maxstanzas));
    }
    if (m_seconds >= 0) {
        helperToXmlAddAttribute(writer, QStringLiteral("seconds"), QString::number(m_seconds));
    }
    if (m_since.isValid()) {
        helperToXmlAddAttribute(writer, QStringLiteral("since"), QXmppUtils::datetimeToString(m_since));
    }
    writer->writeEndElement();
}
/// \endcond
