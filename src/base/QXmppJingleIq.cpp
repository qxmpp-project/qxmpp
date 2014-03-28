/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppJingleIq.h"
#include "QXmppUtils.h"

static const char* ns_jingle_rtp_info = "urn:xmpp:jingle:apps:rtp:info:1";

static const char* jingle_actions[] = {
    "content-accept",
    "content-add",
    "content-modify",
    "content-reject",
    "content-remove",
    "description-info",
    "security-info",
    "session-accept",
    "session-info",
    "session-initiate",
    "session-terminate",
    "transport-accept",
    "transport-info",
    "transport-reject",
    "transport-replace",
};

static const char* jingle_reasons[] = {
    "",
    "alternative-session",
    "busy",
    "cancel",
    "connectivity-error",
    "decline",
    "expired",
    "failed-application",
    "failed-transport",
    "general-error",
    "gone",
    "incompatible-parameters",
    "media-error",
    "security-error",
    "success",
    "timeout",
    "unsupported-applications",
    "unsupported-transports",
};

QXmppJingleIq::Content::Content()
{
}

QString QXmppJingleIq::Content::creator() const
{
    return m_creator;
}

void QXmppJingleIq::Content::setCreator(const QString &creator)
{
    m_creator = creator;
}

QString QXmppJingleIq::Content::name() const
{
    return m_name;
}

void QXmppJingleIq::Content::setName(const QString &name)
{
    m_name = name;
}

QString QXmppJingleIq::Content::senders() const
{
    return m_senders;
}

void QXmppJingleIq::Content::setSenders(const QString &senders)
{
    m_senders = senders;
}

QString QXmppJingleIq::Content::descriptionMedia() const
{
    return m_descriptionMedia;
}

void QXmppJingleIq::Content::setDescriptionMedia(const QString &media)
{
    m_descriptionMedia = media;
}

void QXmppJingleIq::Content::addPayloadType(const QXmppJinglePayloadType &payload)
{
    m_descriptionType = ns_jingle_rtp;
    m_payloadTypes << payload;
}

QList<QXmppJinglePayloadType> QXmppJingleIq::Content::payloadTypes() const
{
    return m_payloadTypes;
}

void QXmppJingleIq::Content::setPayloadTypes(const QList<QXmppJinglePayloadType> &payloadTypes)
{
    m_descriptionType = payloadTypes.isEmpty() ? QString() : ns_jingle_rtp;
    m_payloadTypes = payloadTypes;
}

void QXmppJingleIq::Content::addTransportCandidate(const QXmppJingleCandidate &candidate)
{
    m_transportType = ns_jingle_ice_udp;
    m_transportCandidates << candidate;
}

QList<QXmppJingleCandidate> QXmppJingleIq::Content::transportCandidates() const
{
    return m_transportCandidates;
}

QString QXmppJingleIq::Content::transportUser() const
{
    return m_transportUser;
}

void QXmppJingleIq::Content::setTransportUser(const QString &user)
{
    m_transportUser = user;
}

QString QXmppJingleIq::Content::transportPassword() const
{
    return m_transportPassword;
}

void QXmppJingleIq::Content::setTransportPassword(const QString &password)
{
    m_transportPassword = password;
}

/// \cond
void QXmppJingleIq::Content::parse(const QDomElement &element)
{
    m_creator = element.attribute("creator");
    m_disposition = element.attribute("disposition");
    m_name = element.attribute("name");
    m_senders = element.attribute("senders");

    // description
    QDomElement descriptionElement = element.firstChildElement("description");
    m_descriptionType = descriptionElement.namespaceURI();
    m_descriptionMedia = descriptionElement.attribute("media");
    QDomElement child = descriptionElement.firstChildElement("payload-type");
    while (!child.isNull())
    {
        QXmppJinglePayloadType payload;
        payload.parse(child);
        m_payloadTypes << payload;
        child = child.nextSiblingElement("payload-type");
    }

    // transport
    QDomElement transportElement = element.firstChildElement("transport");
    m_transportType = transportElement.namespaceURI();
    m_transportUser = transportElement.attribute("ufrag");
    m_transportPassword = transportElement.attribute("pwd");
    child = transportElement.firstChildElement("candidate");
    while (!child.isNull())
    {
        QXmppJingleCandidate candidate;
        candidate.parse(child);
        m_transportCandidates << candidate;
        child = child.nextSiblingElement("candidate");
    }
}

void QXmppJingleIq::Content::toXml(QXmlStreamWriter *writer) const
{
    if (m_creator.isEmpty() || m_name.isEmpty())
        return;

    writer->writeStartElement("content");
    helperToXmlAddAttribute(writer, "creator", m_creator);
    helperToXmlAddAttribute(writer, "disposition", m_disposition);
    helperToXmlAddAttribute(writer, "name", m_name);
    helperToXmlAddAttribute(writer, "senders", m_senders);

    // description
    if (!m_descriptionType.isEmpty() || !m_payloadTypes.isEmpty())
    {
        writer->writeStartElement("description");
        writer->writeAttribute("xmlns", m_descriptionType);
        helperToXmlAddAttribute(writer, "media", m_descriptionMedia);
        foreach (const QXmppJinglePayloadType &payload, m_payloadTypes)
            payload.toXml(writer);
        writer->writeEndElement();
    }

    // transport
    if (!m_transportType.isEmpty() || !m_transportCandidates.isEmpty())
    {
        writer->writeStartElement("transport");
        writer->writeAttribute("xmlns", m_transportType);
        helperToXmlAddAttribute(writer, "ufrag", m_transportUser);
        helperToXmlAddAttribute(writer, "pwd", m_transportPassword);
        foreach (const QXmppJingleCandidate &candidate, m_transportCandidates)
            candidate.toXml(writer);
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
/// \endcond

QXmppJingleIq::Reason::Reason()
    : m_type(None)
{
}

QString QXmppJingleIq::Reason::text() const
{
    return m_text;
}

void QXmppJingleIq::Reason::setText(const QString &text)
{
    m_text = text;
}

QXmppJingleIq::Reason::Type QXmppJingleIq::Reason::type() const
{
    return m_type;
}

void QXmppJingleIq::Reason::setType(QXmppJingleIq::Reason::Type type)
{
    m_type = type;
}

/// \cond
void QXmppJingleIq::Reason::parse(const QDomElement &element)
{
    m_text = element.firstChildElement("text").text();
    for (int i = AlternativeSession; i <= UnsupportedTransports; i++)
    {
        if (!element.firstChildElement(jingle_reasons[i]).isNull())
        {
            m_type = static_cast<Type>(i);
            break;
        }
    }
}

void QXmppJingleIq::Reason::toXml(QXmlStreamWriter *writer) const
{
    if (m_type < AlternativeSession || m_type > UnsupportedTransports)
        return;

    writer->writeStartElement("reason");
    if (!m_text.isEmpty())
        helperToXmlAddTextElement(writer, "text", m_text);
    writer->writeEmptyElement(jingle_reasons[m_type]);
    writer->writeEndElement();
}
/// \endcond

/// Constructs a QXmppJingleIq.

QXmppJingleIq::QXmppJingleIq()
    : m_ringing(false)
{
}

/// Returns the Jingle IQ's action.

QXmppJingleIq::Action QXmppJingleIq::action() const
{
    return m_action;
}

/// Sets the Jingle IQ's action.
///
/// \param action

void QXmppJingleIq::setAction(QXmppJingleIq::Action action)
{
    m_action = action;
}

/// Returns the session initiator.

QString QXmppJingleIq::initiator() const
{
    return m_initiator;
}

/// Sets the session initiator.
///
/// \param initiator

void QXmppJingleIq::setInitiator(const QString &initiator)
{
    m_initiator = initiator;
}

/// Returns the session responder.

QString QXmppJingleIq::responder() const
{
    return m_responder;
}

/// Sets the session responder.
///
/// \param responder

void QXmppJingleIq::setResponder(const QString &responder)
{
    m_responder = responder;
}

/// Returns the session ID.

QString QXmppJingleIq::sid() const
{
    return m_sid;
}

/// Sets the session ID.
///
/// \param sid

void QXmppJingleIq::setSid(const QString &sid)
{
    m_sid = sid;
}

/// Returns true if the call is ringing.

bool QXmppJingleIq::ringing() const
{
    return m_ringing;
}

/// Set to true if the call is ringing.
///
/// \param ringing

void QXmppJingleIq::setRinging(bool ringing)
{
    m_ringing = ringing;
}

/// \cond
bool QXmppJingleIq::isJingleIq(const QDomElement &element)
{
    QDomElement jingleElement = element.firstChildElement("jingle");
    return (jingleElement.namespaceURI() == ns_jingle);
}

void QXmppJingleIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement jingleElement = element.firstChildElement("jingle");
    const QString action = jingleElement.attribute("action");
    for (int i = ContentAccept; i <= TransportReplace; i++)
    {
        if (action == jingle_actions[i])
        {
            m_action = static_cast<Action>(i);
            break;
        }
    }
    m_initiator = jingleElement.attribute("initiator");
    m_responder = jingleElement.attribute("responder");
    m_sid = jingleElement.attribute("sid");

    // content
    QDomElement contentElement = jingleElement.firstChildElement("content");
    m_content.parse(contentElement);
    QDomElement reasonElement = jingleElement.firstChildElement("reason");
    m_reason.parse(reasonElement);

    // ringing
    QDomElement ringingElement = jingleElement.firstChildElement("ringing");
    m_ringing = (ringingElement.namespaceURI() == ns_jingle_rtp_info);
}

void QXmppJingleIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("jingle");
    writer->writeAttribute("xmlns", ns_jingle);
    helperToXmlAddAttribute(writer, "action", jingle_actions[m_action]);
    helperToXmlAddAttribute(writer, "initiator", m_initiator);
    helperToXmlAddAttribute(writer, "responder", m_responder);
    helperToXmlAddAttribute(writer, "sid", m_sid);
    m_content.toXml(writer);
    m_reason.toXml(writer);

    // ringing
    if (m_ringing)
    {
        writer->writeStartElement("ringing");
        writer->writeAttribute("xmlns", ns_jingle_rtp_info);
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond

QXmppJingleCandidate::QXmppJingleCandidate()
    : m_component(0),
    m_foundation(0),
    m_generation(0),
    m_network(0),
    m_port(0),
    m_priority(0),
    m_type(HostType)
{
}

/// Returns the candidate's component ID.

int QXmppJingleCandidate::component() const
{
    return m_component;
}

/// Sets the candidates's component ID.
///
/// \param component

void QXmppJingleCandidate::setComponent(int component)
{
    m_component = component;
}

/// Returns the candidate's foundation.

int QXmppJingleCandidate::foundation() const
{
    return m_foundation;
}

/// Sets the candidate's foundation.
///
/// \param foundation

void QXmppJingleCandidate::setFoundation(int foundation)
{
    m_foundation = foundation;
}

/// Returns the candidate's host address.
///

QHostAddress QXmppJingleCandidate::host() const
{
    return m_host;
}

/// Sets the candidate's host address.
///
/// \param host

void QXmppJingleCandidate::setHost(const QHostAddress &host)
{
    m_host = host;
}

/// Returns the candidate's unique identifier.
///

QString QXmppJingleCandidate::id() const
{
    return m_id;
}

/// Sets the candidate's unique identifier.
///
/// \param id

void QXmppJingleCandidate::setId(const QString &id)
{
    m_id = id;
}

/// Returns the network index (starting at 0) the candidate is on.
///

int QXmppJingleCandidate::network() const
{
    return m_network;
}

/// Sets the network index (starting at 0) the candidate is on.
///
/// \param network

void QXmppJingleCandidate::setNetwork(int network)
{
    m_network = network;
}

/// Returns the candidate's port number.
///

quint16 QXmppJingleCandidate::port() const
{
    return m_port;
}

/// Sets the candidate's port number.
///
/// \param port

void QXmppJingleCandidate::setPort(quint16 port)
{
    m_port = port;
}

/// Returns the candidate's priority.
///

int QXmppJingleCandidate::priority() const
{
    return m_priority;
}

/// Sets the candidate's priority.
///
/// \param priority

void QXmppJingleCandidate::setPriority(int priority)
{
    m_priority = priority;
}

/// Returns the candidate's protocol (e.g. "udp").
///

QString QXmppJingleCandidate::protocol() const
{
    return m_protocol;
}

/// Sets the candidate's protocol (e.g. "udp").
///
/// \param protocol

void QXmppJingleCandidate::setProtocol(const QString &protocol)
{
    m_protocol = protocol;
}

/// Returns the candidate type (e.g. "host").
///

QXmppJingleCandidate::Type QXmppJingleCandidate::type() const
{
    return m_type;
}

/// Sets the candidate type (e.g. "host").
///
/// \param type

void QXmppJingleCandidate::setType(QXmppJingleCandidate::Type type)
{
    m_type = type;
}

/// Returns true if the host address or port are empty.
///

bool QXmppJingleCandidate::isNull() const
{
    return m_host.isNull() || !m_port;
}

/// \cond
void QXmppJingleCandidate::parse(const QDomElement &element)
{
    m_component = element.attribute("component").toInt();
    m_foundation = element.attribute("foundation").toInt();
    m_generation = element.attribute("generation").toInt();
    m_host = QHostAddress(element.attribute("ip"));
    m_id = element.attribute("id");
    m_network = element.attribute("network").toInt();
    m_port = element.attribute("port").toInt();
    m_priority = element.attribute("priority").toInt();
    m_protocol = element.attribute("protocol");
    m_type = typeFromString(element.attribute("type"));
}

void QXmppJingleCandidate::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("candidate");
    helperToXmlAddAttribute(writer, "component", QString::number(m_component));
    helperToXmlAddAttribute(writer, "foundation", QString::number(m_foundation));
    helperToXmlAddAttribute(writer, "generation", QString::number(m_generation));
    helperToXmlAddAttribute(writer, "id", m_id);
    helperToXmlAddAttribute(writer, "ip", m_host.toString());
    helperToXmlAddAttribute(writer, "network", QString::number(m_network));
    helperToXmlAddAttribute(writer, "port", QString::number(m_port));
    helperToXmlAddAttribute(writer, "priority", QString::number(m_priority));
    helperToXmlAddAttribute(writer, "protocol", m_protocol);
    helperToXmlAddAttribute(writer, "type", typeToString(m_type));
    writer->writeEndElement();
}

QXmppJingleCandidate::Type QXmppJingleCandidate::typeFromString(const QString &typeStr, bool *ok)
{
    QXmppJingleCandidate::Type type;
    if (typeStr == "host")
        type = HostType;
    else if (typeStr == "prflx")
        type = PeerReflexiveType;
    else if (typeStr == "srflx")
        type = ServerReflexiveType;
    else if (typeStr == "relay")
        type = RelayedType;
    else {
        qWarning() << "Unknown candidate type" << typeStr;
        if (ok)
            *ok = false;
        return HostType;
    }
    if (ok)
        *ok = true;
    return type;
}

QString QXmppJingleCandidate::typeToString(QXmppJingleCandidate::Type type)
{
    QString typeStr;
    switch (type)
    {
    case HostType:
        typeStr = "host";
        break;
    case PeerReflexiveType:
        typeStr = "prflx";
        break;
    case ServerReflexiveType:
        typeStr = "srflx";
        break;
    case RelayedType:
        typeStr = "relay";
        break;
    }
    return typeStr;
}
/// \endcond

QXmppJinglePayloadType::QXmppJinglePayloadType()
    : m_channels(1),
    m_clockrate(0),
    m_id(0),
    m_maxptime(0),
    m_ptime(0)
{
}

/// Returns the number of channels (e.g. 1 for mono, 2 for stereo).
///

unsigned char QXmppJinglePayloadType::channels() const
{
    return m_channels;
}

/// Sets the number of channels (e.g. 1 for mono, 2 for stereo).
///
/// \param channels

void QXmppJinglePayloadType::setChannels(unsigned char channels)
{
    m_channels = channels;
}

/// Returns the clockrate in Hz, i.e. the number of samples per second.
///

unsigned int QXmppJinglePayloadType::clockrate() const
{
    return m_clockrate;
}

/// Sets the clockrate in Hz, i.e. the number of samples per second.
///
/// \param clockrate

void QXmppJinglePayloadType::setClockrate(unsigned int clockrate)
{
    m_clockrate = clockrate;
}

/// Returns the payload type identifier.
///

unsigned char QXmppJinglePayloadType::id() const
{
    return m_id;
}

/// Sets the payload type identifier.
///

void QXmppJinglePayloadType::setId(unsigned char id)
{
    Q_ASSERT(id <= 127);
    m_id = id;
}

/// Returns the maximum packet time in milliseconds.
///

unsigned int QXmppJinglePayloadType::maxptime() const
{
    return m_maxptime;
}

/// Sets the maximum packet type in milliseconds.
///
/// \param maxptime

void QXmppJinglePayloadType::setMaxptime(unsigned int maxptime)
{
    m_maxptime = maxptime;
}

/// Returns the payload type name.
///

QString QXmppJinglePayloadType::name() const
{
    return m_name;
}

/// Sets the payload type name.
///
/// \param name

void QXmppJinglePayloadType::setName(const QString &name)
{
    m_name = name;
}

/// Returns the payload parameters.

QMap<QString,QString> QXmppJinglePayloadType::parameters() const
{
    return m_parameters;
}

/// Sets the payload parameters.

void QXmppJinglePayloadType::setParameters(const QMap<QString, QString> &parameters)
{
    m_parameters = parameters;
}

/// Returns the packet time in milliseconds (20 by default).
///

unsigned int QXmppJinglePayloadType::ptime() const
{
    return m_ptime ? m_ptime : 20;
}

/// Sets the packet time in milliseconds (20 by default).
///
/// \param ptime

void QXmppJinglePayloadType::setPtime(unsigned int ptime)
{
    m_ptime = ptime;
}

/// \cond
void QXmppJinglePayloadType::parse(const QDomElement &element)
{
    m_id = element.attribute("id").toInt();
    m_name = element.attribute("name");
    m_channels = element.attribute("channels").toInt();
    if (!m_channels)
        m_channels = 1;
    m_clockrate = element.attribute("clockrate").toInt();
    m_maxptime = element.attribute("maxptime").toInt();
    m_ptime = element.attribute("ptime").toInt();

    QDomElement child = element.firstChildElement("parameter");
    while (!child.isNull()) {
        m_parameters.insert(child.attribute("name"), child.attribute("value"));
        child = child.nextSiblingElement("parameter");
    }
}

void QXmppJinglePayloadType::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("payload-type");
    helperToXmlAddAttribute(writer, "id", QString::number(m_id));
    helperToXmlAddAttribute(writer, "name", m_name);
    if (m_channels > 1)
        helperToXmlAddAttribute(writer, "channels", QString::number(m_channels));
    if (m_clockrate > 0)
        helperToXmlAddAttribute(writer, "clockrate", QString::number(m_clockrate));
    if (m_maxptime > 0)
        helperToXmlAddAttribute(writer, "maxptime", QString::number(m_maxptime));
    if (m_ptime > 0)
        helperToXmlAddAttribute(writer, "ptime", QString::number(m_ptime));

    foreach (const QString &key, m_parameters.keys()) {
        writer->writeStartElement("parameter");
        writer->writeAttribute("name", key);
        writer->writeAttribute("value", m_parameters.value(key));
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
/// \endcond

/// Returns true if this QXmppJinglePayloadType and \a other refer to the same payload type.
///
/// \param other

bool QXmppJinglePayloadType::operator==(const QXmppJinglePayloadType &other) const
{
    // FIXME : what to do with m_ptime and m_maxptime?
    if (m_id <= 95)
        return other.m_id == m_id && other.m_clockrate == m_clockrate;
    else
        return other.m_channels == m_channels &&
               other.m_clockrate == m_clockrate &&
               other.m_name.toLower() == m_name.toLower();
}
