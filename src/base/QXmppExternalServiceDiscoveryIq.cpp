// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppExternalServiceDiscoveryIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDateTime>
#include <QDomElement>

using Action = QXmppExternalService::Action;
using Transport = QXmppExternalService::Transport;

QString actionToString(Action action)
{
    switch (action) {
    case Action::Add:
        return QStringLiteral("add");
    case Action::Delete:
        return QStringLiteral("delete");
    case Action::Modify:
        return QStringLiteral("modify");
    }
    return {};
}

std::optional<Action> actionFromString(const QString &string)
{
    if (string == QStringLiteral("add")) {
        return Action::Add;
    } else if (string == QStringLiteral("delete")) {
        return Action::Delete;
    } else if (string == QStringLiteral("modify")) {
        return Action::Modify;
    }

    return std::nullopt;
}

QString transportToString(Transport transport)
{
    switch (transport) {
    case Transport::Tcp:
        return QStringLiteral("tcp");
    case Transport::Udp:
        return QStringLiteral("udp");
    }

    return {};
}

std::optional<Transport> transportFromString(const QString &string)
{
    if (string == QStringLiteral("tcp")) {
        return Transport::Tcp;
    } else if (string == QStringLiteral("udp")) {
        return Transport::Udp;
    }

    return std::nullopt;
}

class QXmppExternalServicePrivate : public QSharedData
{
public:
    QString host;
    QString type;
    std::optional<Action> action;
    std::optional<QDateTime> expires;
    std::optional<QString> name;
    std::optional<QString> password;
    std::optional<int> port;  // recommended
    std::optional<bool> restricted;
    std::optional<Transport> transport;  // recommended
    std::optional<QString> username;
};

class QXmppExternalServiceDiscoveryIqPrivate : public QSharedData
{
public:
    QVector<QXmppExternalService> externalServices;
};

///
/// \class QXmppExternalService
///
/// QXmppExternalService represents a related XMPP entity that can be queried using \xep{0215,
/// External Service Discovery}.
///
/// \since QXmpp 1.6
///

QXmppExternalService::QXmppExternalService()
    : d(new QXmppExternalServicePrivate)
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppExternalService)

///
/// Returns the host of the external service.
///
QString QXmppExternalService::host() const
{
    return d->host;
}

///
/// Sets the host of the external service.
///
void QXmppExternalService::setHost(const QString &host)
{
    d->host = host;
}

///
/// Returns the type of the external service.
///
QString QXmppExternalService::type() const
{
    return d->type;
}

///
/// Sets the type of the external service.
///
void QXmppExternalService::setType(const QString &type)
{
    d->type = type;
}

///
/// Returns the action of the external service.
///
std::optional<Action> QXmppExternalService::action() const
{
    return d->action;
}

///
/// Sets the action of the external service.
///
void QXmppExternalService::setAction(std::optional<Action> action)
{
    d->action = action;
}

///
/// Returns the expiration date of the external service.
///
std::optional<QDateTime> QXmppExternalService::expires() const
{
    return d->expires;
}

///
/// Sets the expiration date of the external service.
///
void QXmppExternalService::setExpires(std::optional<QDateTime> expires)
{
    d->expires = std::move(expires);
}

///
/// Returns the name of the external service.
///
std::optional<QString> QXmppExternalService::name() const
{
    return d->name;
}

///
/// Sets the name of the external service.
///
void QXmppExternalService::setName(std::optional<QString> name)
{
    d->name = std::move(name);
}

///
/// Returns the password of the external service.
///
std::optional<QString> QXmppExternalService::password() const
{
    return d->password;
}

///
/// Sets the password of the external service.
///
void QXmppExternalService::setPassword(std::optional<QString> password)
{
    d->password = std::move(password);
}

///
/// Returns the port of the external service.
///
std::optional<int> QXmppExternalService::port() const
{
    return d->port;
}

///
/// Sets the port of the external service.
///
void QXmppExternalService::setPort(std::optional<int> port)
{
    d->port = port;
}

///
/// Returns the restricted mode of the external service.
///
std::optional<bool> QXmppExternalService::restricted() const
{
    return d->restricted;
}
///
/// Sets the restricted mode of the external service.
///
void QXmppExternalService::setRestricted(std::optional<bool> restricted)
{
    d->restricted = restricted;
}

///
/// Returns the transport type of the external service.
///
std::optional<QXmppExternalService::Transport> QXmppExternalService::transport() const
{
    return d->transport;
}

///
/// Sets the transport type of the external service.
///
void QXmppExternalService::setTransport(std::optional<Transport> transport)
{
    d->transport = transport;
}

///
/// Returns the username of the external service.
///
std::optional<QString> QXmppExternalService::username() const
{
    return d->username;
}

///
/// Sets the username of the external service.
///
void QXmppExternalService::setUsername(std::optional<QString> username)
{
    d->username = std::move(username);
}

///
/// Returns true if the element is a valid external service and can be parsed.
///
bool QXmppExternalService::isExternalService(const QDomElement &element)
{
    if (element.tagName() != "service") {
        return false;
    }

    return element.hasAttribute("host") && !element.attribute("host").isEmpty() &&
        element.hasAttribute("type") && !element.attribute("type").isEmpty();
}

///
/// Parses given DOM element as an external service.
///
void QXmppExternalService::parse(const QDomElement &el)
{
    QDomNamedNodeMap attributes = el.attributes();

    setHost(el.attribute("host"));
    setType(el.attribute("type"));

    d->action = actionFromString(el.attribute("action"));

    if (attributes.contains("expires")) {
        setExpires(QXmppUtils::datetimeFromString(el.attribute("expires")));
    }

    if (attributes.contains("name")) {
        setName(el.attribute("name"));
    }

    if (attributes.contains("password")) {
        setPassword(el.attribute("password"));
    }

    if (attributes.contains("port")) {
        setPort(el.attribute("port").toInt());
    }

    if (attributes.contains("restricted")) {
        bool isRestricted {
            el.attribute("restricted") == "true" ||
            el.attribute("restricted") == "1"
        };

        setRestricted(isRestricted);
    }

    d->transport = transportFromString(el.attribute("transport"));

    if (attributes.contains("username")) {
        setUsername(el.attribute("username"));
    }
}

///
/// \brief QXmppExternalService::toXml
///
/// Translates the external service to XML using the provided XML stream writer.
///
/// \param writer
///

void QXmppExternalService::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("service");
    helperToXmlAddAttribute(writer, "host", d->host);
    helperToXmlAddAttribute(writer, "type", d->type);

    if (d->action) {
        helperToXmlAddAttribute(writer, "action", actionToString(d->action.value()));
    }

    if (d->expires) {
        helperToXmlAddAttribute(writer, "expires", d->expires->toString(Qt::ISODateWithMs));
    }

    if (d->name) {
        helperToXmlAddAttribute(writer, "name", d->name.value());
    }

    if (d->password) {
        helperToXmlAddAttribute(writer, "password", d->password.value());
    }

    if (d->port) {
        helperToXmlAddAttribute(writer, "port", QString::number(d->port.value()));
    }

    if (d->restricted) {
        helperToXmlAddAttribute(writer, "restricted", d->restricted.value() ? "true" : "false");
    }

    if (d->transport) {
        helperToXmlAddAttribute(writer, "transport", transportToString(d->transport.value()));
    }

    if (d->username) {
        helperToXmlAddAttribute(writer, "username", d->username.value());
    }

    writer->writeEndElement();
}

///
/// \brief The QXmppExternalServiceDiscoveryIq class represents an IQ used to discover external
/// services as defined by \xep{0215}: External Service Discovery.
///
/// \ingroup Stanzas
///
/// \since QXmpp 1.6
///

///
/// Constructs an external service discovery IQ.
///
QXmppExternalServiceDiscoveryIq::QXmppExternalServiceDiscoveryIq()
    : d(new QXmppExternalServiceDiscoveryIqPrivate)
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppExternalServiceDiscoveryIq)

///
/// Returns the external services of the IQ.
///
QVector<QXmppExternalService> QXmppExternalServiceDiscoveryIq::externalServices()
{
    return d->externalServices;
}

///
/// Sets the external services of the IQ.
///
void QXmppExternalServiceDiscoveryIq::setExternalServices(const QVector<QXmppExternalService> &externalServices)
{
    d->externalServices = externalServices;
}

///
/// Adds an external service to the list of external services in the IQ.
///
void QXmppExternalServiceDiscoveryIq::addExternalService(const QXmppExternalService &externalService)
{
    d->externalServices.append(externalService);
}

///
/// Returns true if the provided DOM element is an external service discovery IQ.
///
bool QXmppExternalServiceDiscoveryIq::isExternalServiceDiscoveryIq(const QDomElement &element)
{
    auto child = element.firstChildElement();
    return checkIqType(child.tagName(), child.namespaceURI());
}

///
/// Returns true if the IQ is a valid external service discovery IQ.
///
bool QXmppExternalServiceDiscoveryIq::checkIqType(const QString &tagName, const QString &xmlNamespace)
{
    return tagName == QStringLiteral("services") && (xmlNamespace == ns_external_service_discovery);
}

/// \cond
void QXmppExternalServiceDiscoveryIq::parseElementFromChild(const QDomElement &element)
{
    for (auto el = element.firstChildElement("services").firstChildElement();
         !el.isNull();
         el = el.nextSiblingElement()) {
        if (QXmppExternalService::isExternalService(el)) {
            QXmppExternalService service;
            service.parse(el);
            d->externalServices.append(std::move(service));
        }
    }
}

void QXmppExternalServiceDiscoveryIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("services");
    writer->writeDefaultNamespace(ns_external_service_discovery);

    for (const QXmppExternalService &item : d->externalServices) {
        item.toXml(writer);
    }

    writer->writeEndElement();
}
/// \endcond
