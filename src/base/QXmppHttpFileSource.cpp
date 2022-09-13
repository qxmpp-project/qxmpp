// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppHttpFileSource.h"

#include "QXmppConstants_p.h"

#include <QDomElement>
#include <QXmlStreamWriter>

///
/// \class QXmppHttpFileSource
///
/// Represents an HTTP File source for file sharing.
///
/// \since QXmpp 1.5
///

/// Default constructor.
QXmppHttpFileSource::QXmppHttpFileSource()
{
}

/// Default constructor.
QXmppHttpFileSource::QXmppHttpFileSource(QUrl url)
    : m_url(std::move(url))
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppHttpFileSource)

///
/// Returns the HTTP url.
///
const QUrl &QXmppHttpFileSource::url() const
{
    return m_url;
}

///
/// Sets the HTTP url.
///
void QXmppHttpFileSource::setUrl(QUrl url)
{
    m_url = std::move(url);
}

/// \cond
bool QXmppHttpFileSource::parse(const QDomElement &el)
{
    if (el.tagName() == "url-data" && el.namespaceURI() == ns_url_data) {
        m_url = QUrl(el.attribute("target"));
        return true;
    }
    return false;
}

void QXmppHttpFileSource::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("url-data");
    writer->writeDefaultNamespace(ns_url_data);
    writer->writeAttribute("target", m_url.toString());
    writer->writeEndElement();
}
/// \endcond
