// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppIq.h"

#include "QXmppUtils.h"

#include <QDomElement>
#include <QXmlStreamWriter>

static const char *iq_types[] = {
    "error",
    "get",
    "set",
    "result"
};

class QXmppIqPrivate : public QSharedData
{
public:
    QXmppIq::Type type;
};

///
/// Constructs a QXmppIq with the specified \a type.
///
/// \param type
///
QXmppIq::QXmppIq(QXmppIq::Type type)
    : QXmppStanza(), d(new QXmppIqPrivate)
{
    d->type = type;
    generateAndSetNextId();
}

/// Constructs a copy of \a other.
QXmppIq::QXmppIq(const QXmppIq &other) = default;
/// Default move-constructor.
QXmppIq::QXmppIq(QXmppIq &&) = default;
QXmppIq::~QXmppIq() = default;

/// Assigns \a other to this IQ.
QXmppIq &QXmppIq::operator=(const QXmppIq &other) = default;
/// Move-assignment operator.
QXmppIq &QXmppIq::operator=(QXmppIq &&) = default;

///
/// Returns the IQ's type.
///
QXmppIq::Type QXmppIq::type() const
{
    return d->type;
}

///
/// Sets the IQ's type.
///
/// \param type
///
void QXmppIq::setType(QXmppIq::Type type)
{
    d->type = type;
}

///
/// Indicates if the QXmppStanza is a stanza in the XMPP sense (i. e. a message,
/// iq or presence)
///
/// \since QXmpp 1.0
///
bool QXmppIq::isXmppStanza() const
{
    return true;
}

/// \cond
void QXmppIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    const QString type = element.attribute("type");
    for (int i = Error; i <= Result; i++) {
        if (type == iq_types[i]) {
            d->type = static_cast<Type>(i);
            break;
        }
    }

    parseElementFromChild(element);
}

void QXmppIq::parseElementFromChild(const QDomElement &element)
{
    QXmppElementList extensions;

    for (auto itemElement = element.firstChildElement();
         !itemElement.isNull();
         itemElement = itemElement.nextSiblingElement()) {
        extensions.append(QXmppElement(itemElement));
    }
    setExtensions(extensions);
}

void QXmppIq::toXml(QXmlStreamWriter *xmlWriter) const
{
    xmlWriter->writeStartElement("iq");

    helperToXmlAddAttribute(xmlWriter, "id", id());
    helperToXmlAddAttribute(xmlWriter, "to", to());
    helperToXmlAddAttribute(xmlWriter, "from", from());
    helperToXmlAddAttribute(xmlWriter, "type", iq_types[d->type]);
    toXmlElementFromChild(xmlWriter);
    error().toXml(xmlWriter);
    xmlWriter->writeEndElement();
}

void QXmppIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    const auto exts = extensions();
    for (const QXmppElement &extension : exts) {
        extension.toXml(writer);
    }
}
/// \endcond
