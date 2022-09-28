// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppOutOfBandUrl.h"

#include "QXmppConstants_p.h"

#include <QDomElement>
#include <QXmlStreamWriter>

class QXmppOutOfBandUrlPrivate : public QSharedData
{
public:
    QString url;
    std::optional<QString> description;
};

///
/// \class QXmppOutOfBandUrl
///
/// A URL and a description of its contents, from \xep{0066}: Out of Band Data
///
/// \since QXmpp 1.5
///

QXmppOutOfBandUrl::QXmppOutOfBandUrl()
    : d(new QXmppOutOfBandUrlPrivate())
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppOutOfBandUrl);

///
/// Returns the attached URL
///
const QString &QXmppOutOfBandUrl::url() const
{
    return d->url;
}

///
/// Sets the attached URL
///
void QXmppOutOfBandUrl::setUrl(const QString &url)
{
    d->url = url;
}

///
/// Returns the description of the URL
///
const std::optional<QString> &QXmppOutOfBandUrl::description() const
{
    return d->description;
}

///
/// Set the description of the URL
///
void QXmppOutOfBandUrl::setDescription(const std::optional<QString> &description)
{
    d->description = description;
}

/// \cond
bool QXmppOutOfBandUrl::parse(const QDomElement &el)
{
    d->url = el.firstChildElement(QStringLiteral("url")).text();
    auto childEl = el.firstChildElement(QStringLiteral("desc"));
    if (!childEl.isNull()) {
        d->description = childEl.text();
    }

    return true;
}

void QXmppOutOfBandUrl::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("x"));
    writer->writeDefaultNamespace(ns_oob);
    writer->writeTextElement(QStringLiteral("url"), d->url);
    if (d->description) {
        writer->writeTextElement(QStringLiteral("desc"), *d->description);
    }
    writer->writeEndElement();
}
/// \endcond
