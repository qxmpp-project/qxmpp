/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppJingleIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDate>
#include <QDateTime>
#include <QDomElement>
#include <QRegExp>

static const int RTP_COMPONENT = 1;

static const char *ns_jingle_rtp_info = "urn:xmpp:jingle:apps:rtp:info:1";
static const char *ns_jingle_dtls = "urn:xmpp:jingle:apps:dtls:0";

static const char *jingle_actions[] = {
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

static const char *jingle_reasons[] = {
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

static QString formatFingerprint(const QByteArray &digest)
{
    QString fingerprint;
    const QString hx = digest.toHex().toUpper();
    for (int i = 0; i < hx.size(); i += 2) {
        if (!fingerprint.isEmpty())
            fingerprint += ':';
        fingerprint += hx.mid(i, 2);
    }
    return fingerprint;
}

static QByteArray parseFingerprint(const QString &fingerprint)
{
    QString z = fingerprint;
    z.replace(':', "");
    return QByteArray::fromHex(z.toUtf8());
}

static QString addressToSdp(const QHostAddress &host)
{
    return QStringLiteral("IN %1 %2").arg(host.protocol() == QAbstractSocket::IPv6Protocol ? QStringLiteral("IP6") : QStringLiteral("IP4"), host.toString());
}

static bool candidateParseSdp(QXmppJingleCandidate *candidate, const QString &sdp)
{
    if (!sdp.startsWith(QStringLiteral("candidate:")))
        return false;

    const QStringList bits = sdp.mid(10).split(" ");
    if (bits.size() < 6)
        return false;

    candidate->setFoundation(bits[0]);
    candidate->setComponent(bits[1].toInt());
    candidate->setProtocol(bits[2].toLower());
    candidate->setPriority(bits[3].toInt());
    candidate->setHost(QHostAddress(bits[4]));
    candidate->setPort(bits[5].toInt());
    for (int i = 6; i < bits.size() - 1; i += 2) {
        if (bits[i] == QStringLiteral("typ")) {
            bool ok;
            candidate->setType(QXmppJingleCandidate::typeFromString(bits[i + 1], &ok));
            if (!ok)
                return false;
        } else if (bits[i] == QStringLiteral("generation")) {
            candidate->setGeneration(bits[i + 1].toInt());
        } else {
            qWarning() << "Candidate SDP contains unknown attribute" << bits[i];
            return false;
        }
    }
    return true;
}

static QString candidateToSdp(const QXmppJingleCandidate &candidate)
{
    return QStringLiteral("candidate:%1 %2 %3 %4 %5 %6 typ %7 generation %8").arg(candidate.foundation(), QString::number(candidate.component()), candidate.protocol(), QString::number(candidate.priority()), candidate.host().toString(), QString::number(candidate.port()), QXmppJingleCandidate::typeToString(candidate.type()), QString::number(candidate.generation()));
}

class QXmppJingleIqContentPrivate : public QSharedData
{
public:
    QXmppJingleIqContentPrivate();

    QString creator;
    QString disposition;
    QString name;
    QString senders;

    QString descriptionMedia;
    quint32 descriptionSsrc;
    QString descriptionType;
    QString transportType;
    QString transportUser;
    QString transportPassword;

    QByteArray transportFingerprint;
    QString transportFingerprintHash;
    QString transportFingerprintSetup;

    QList<QXmppJinglePayloadType> payloadTypes;
    QList<QXmppJingleCandidate> transportCandidates;
};

QXmppJingleIqContentPrivate::QXmppJingleIqContentPrivate()
    : descriptionSsrc(0)
{
}

/// Constructs an empty content.

QXmppJingleIq::Content::Content()
    : d(new QXmppJingleIqContentPrivate())
{
}

/// Constructs a copy of other.
///
/// \param other

QXmppJingleIq::Content::Content(const QXmppJingleIq::Content &other)
    : d(other.d)
{
}

/// Assigns the other content to this one.
///
/// \param other

QXmppJingleIq::Content &QXmppJingleIq::Content::operator=(const QXmppJingleIq::Content &other)
{
    d = other.d;
    return *this;
}

QXmppJingleIq::Content::~Content()
{
}

QString QXmppJingleIq::Content::creator() const
{
    return d->creator;
}

void QXmppJingleIq::Content::setCreator(const QString &creator)
{
    d->creator = creator;
}

QString QXmppJingleIq::Content::name() const
{
    return d->name;
}

void QXmppJingleIq::Content::setName(const QString &name)
{
    d->name = name;
}

QString QXmppJingleIq::Content::senders() const
{
    return d->senders;
}

void QXmppJingleIq::Content::setSenders(const QString &senders)
{
    d->senders = senders;
}

QString QXmppJingleIq::Content::descriptionMedia() const
{
    return d->descriptionMedia;
}

void QXmppJingleIq::Content::setDescriptionMedia(const QString &media)
{
    d->descriptionMedia = media;
}

quint32 QXmppJingleIq::Content::descriptionSsrc() const
{
    return d->descriptionSsrc;
}

void QXmppJingleIq::Content::setDescriptionSsrc(quint32 ssrc)
{
    d->descriptionSsrc = ssrc;
}

void QXmppJingleIq::Content::addPayloadType(const QXmppJinglePayloadType &payload)
{
    d->descriptionType = ns_jingle_rtp;
    d->payloadTypes << payload;
}

QList<QXmppJinglePayloadType> QXmppJingleIq::Content::payloadTypes() const
{
    return d->payloadTypes;
}

void QXmppJingleIq::Content::setPayloadTypes(const QList<QXmppJinglePayloadType> &payloadTypes)
{
    d->descriptionType = payloadTypes.isEmpty() ? QString() : ns_jingle_rtp;
    d->payloadTypes = payloadTypes;
}

void QXmppJingleIq::Content::addTransportCandidate(const QXmppJingleCandidate &candidate)
{
    d->transportType = ns_jingle_ice_udp;
    d->transportCandidates << candidate;
}

QList<QXmppJingleCandidate> QXmppJingleIq::Content::transportCandidates() const
{
    return d->transportCandidates;
}

///
/// Sets a list of transport candidates.
///
/// \since QXmpp 0.9.2
///
void QXmppJingleIq::Content::setTransportCandidates(const QList<QXmppJingleCandidate> &candidates)
{
    d->transportType = candidates.isEmpty() ? QString() : ns_jingle_ice_udp;
    d->transportCandidates = candidates;
}

QString QXmppJingleIq::Content::transportUser() const
{
    return d->transportUser;
}

void QXmppJingleIq::Content::setTransportUser(const QString &user)
{
    d->transportUser = user;
}

QString QXmppJingleIq::Content::transportPassword() const
{
    return d->transportPassword;
}

void QXmppJingleIq::Content::setTransportPassword(const QString &password)
{
    d->transportPassword = password;
}

/// Returns the fingerprint hash value for the transport key.
///
/// This is used for DTLS-SRTP as defined in \xep{0320}.

QByteArray QXmppJingleIq::Content::transportFingerprint() const
{
    return d->transportFingerprint;
}

/// Sets the fingerprint hash value for the transport key.
///
/// This is used for DTLS-SRTP as defined in \xep{0320}.

void QXmppJingleIq::Content::setTransportFingerprint(const QByteArray &fingerprint)
{
    d->transportFingerprint = fingerprint;
}

/// Returns the fingerprint hash algorithm for the transport key.
///
/// This is used for DTLS-SRTP as defined in \xep{0320}.

QString QXmppJingleIq::Content::transportFingerprintHash() const
{
    return d->transportFingerprintHash;
}

/// Sets the fingerprint hash algorithm for the transport key.
///
/// This is used for DTLS-SRTP as defined in \xep{0320}.

void QXmppJingleIq::Content::setTransportFingerprintHash(const QString &hash)
{
    d->transportFingerprintHash = hash;
}

/// Returns the setup role for the encrypted transport.
///
/// This is used for DTLS-SRTP as defined in \xep{0320}.

QString QXmppJingleIq::Content::transportFingerprintSetup() const
{
    return d->transportFingerprintSetup;
}

/// Sets the setup role for the encrypted transport.
///
/// This is used for DTLS-SRTP as defined in \xep{0320}.

void QXmppJingleIq::Content::setTransportFingerprintSetup(const QString &setup)
{
    d->transportFingerprintSetup = setup;
}

/// \cond
void QXmppJingleIq::Content::parse(const QDomElement &element)
{
    d->creator = element.attribute(QStringLiteral("creator"));
    d->disposition = element.attribute(QStringLiteral("disposition"));
    d->name = element.attribute(QStringLiteral("name"));
    d->senders = element.attribute(QStringLiteral("senders"));

    // description
    QDomElement descriptionElement = element.firstChildElement(QStringLiteral("description"));
    d->descriptionType = descriptionElement.namespaceURI();
    d->descriptionMedia = descriptionElement.attribute(QStringLiteral("media"));
    d->descriptionSsrc = descriptionElement.attribute(QStringLiteral("ssrc")).toULong();
    QDomElement child = descriptionElement.firstChildElement(QStringLiteral("payload-type"));
    while (!child.isNull()) {
        QXmppJinglePayloadType payload;
        payload.parse(child);
        d->payloadTypes << payload;
        child = child.nextSiblingElement(QStringLiteral("payload-type"));
    }

    // transport
    QDomElement transportElement = element.firstChildElement(QStringLiteral("transport"));
    d->transportType = transportElement.namespaceURI();
    d->transportUser = transportElement.attribute(QStringLiteral("ufrag"));
    d->transportPassword = transportElement.attribute(QStringLiteral("pwd"));
    child = transportElement.firstChildElement(QStringLiteral("candidate"));
    while (!child.isNull()) {
        QXmppJingleCandidate candidate;
        candidate.parse(child);
        d->transportCandidates << candidate;
        child = child.nextSiblingElement(QStringLiteral("candidate"));
    }
    child = transportElement.firstChildElement(QStringLiteral("fingerprint"));

    /// XEP-0320
    if (!child.isNull()) {
        d->transportFingerprint = parseFingerprint(child.text());
        d->transportFingerprintHash = child.attribute(QStringLiteral("hash"));
        d->transportFingerprintSetup = child.attribute(QStringLiteral("setup"));
    }
}

void QXmppJingleIq::Content::toXml(QXmlStreamWriter *writer) const
{
    if (d->creator.isEmpty() || d->name.isEmpty())
        return;

    writer->writeStartElement(QStringLiteral("content"));
    helperToXmlAddAttribute(writer, QStringLiteral("creator"), d->creator);
    helperToXmlAddAttribute(writer, QStringLiteral("disposition"), d->disposition);
    helperToXmlAddAttribute(writer, QStringLiteral("name"), d->name);
    helperToXmlAddAttribute(writer, QStringLiteral("senders"), d->senders);

    // description
    if (!d->descriptionType.isEmpty() || !d->payloadTypes.isEmpty()) {
        writer->writeStartElement(QStringLiteral("description"));
        writer->writeDefaultNamespace(d->descriptionType);
        helperToXmlAddAttribute(writer, QStringLiteral("media"), d->descriptionMedia);
        if (d->descriptionSsrc)
            writer->writeAttribute(QStringLiteral("ssrc"), QString::number(d->descriptionSsrc));
        for (const auto &payload : d->payloadTypes)
            payload.toXml(writer);
        writer->writeEndElement();
    }

    // transport
    if (!d->transportType.isEmpty() || !d->transportCandidates.isEmpty()) {
        writer->writeStartElement(QStringLiteral("transport"));
        writer->writeDefaultNamespace(d->transportType);
        helperToXmlAddAttribute(writer, QStringLiteral("ufrag"), d->transportUser);
        helperToXmlAddAttribute(writer, QStringLiteral("pwd"), d->transportPassword);
        for (const auto &candidate : d->transportCandidates)
            candidate.toXml(writer);

        // XEP-0320
        if (!d->transportFingerprint.isEmpty() && !d->transportFingerprintHash.isEmpty()) {
            writer->writeStartElement(QStringLiteral("fingerprint"));
            writer->writeDefaultNamespace(ns_jingle_dtls);
            writer->writeAttribute(QStringLiteral("hash"), d->transportFingerprintHash);
            writer->writeAttribute(QStringLiteral("setup"), d->transportFingerprintSetup);
            writer->writeCharacters(formatFingerprint(d->transportFingerprint));
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();
}

bool QXmppJingleIq::Content::parseSdp(const QString &sdp)
{
    QList<QXmppJinglePayloadType> payloads;
    QString line;
    for (auto &line : sdp.split('\n')) {
        if (line.endsWith('\r'))
            line.resize(line.size() - 1);
        if (line.startsWith(QStringLiteral("a="))) {
            int idx = line.indexOf(':');
            const QString attrName = idx != -1 ? line.mid(2, idx - 2) : line.mid(2);
            const QString attrValue = idx != -1 ? line.mid(idx + 1) : "";

            if (attrName == QStringLiteral("candidate")) {
                QXmppJingleCandidate candidate;
                if (!candidateParseSdp(&candidate, line.mid(2))) {
                    qWarning() << "Could not parse candidate" << line;
                    return false;
                }
                addTransportCandidate(candidate);
            } else if (attrName == QStringLiteral("fingerprint")) {
                const QStringList bits = attrValue.split(' ');
                if (bits.size() > 1) {
                    d->transportFingerprintHash = bits[0];
                    d->transportFingerprint = parseFingerprint(bits[1]);
                }
            } else if (attrName == QStringLiteral("fmtp")) {
                int spIdx = attrValue.indexOf(' ');
                if (spIdx == -1) {
                    qWarning() << "Could not parse payload parameters" << line;
                    return false;
                }
                const int id = attrValue.left(spIdx).toInt();
                const QString paramStr = attrValue.mid(spIdx + 1);
                for (auto &payload : payloads) {
                    if (payload.id() == id) {
                        QMap<QString, QString> params;
                        if (payload.name() == QStringLiteral("telephone-event")) {
                            params.insert(QStringLiteral("events"), paramStr);
                        } else {
                            for (const auto &p : paramStr.split(QRegExp(";\\s*"))) {
                                QStringList bits = p.split('=');
                                if (bits.size() == 2)
                                    params.insert(bits[0], bits[1]);
                            }
                        }
                        payload.setParameters(params);
                    }
                }
            } else if (attrName == QStringLiteral("rtpmap")) {
                // payload type map
                const QStringList bits = attrValue.split(' ');
                if (bits.size() != 2)
                    continue;
                bool ok = false;
                const int id = bits[0].toInt(&ok);
                if (!ok)
                    continue;

                const QStringList args = bits[1].split('/');
                for (auto &payload : payloads) {
                    if (payload.id() == id) {
                        payload.setName(args[0]);
                        if (args.size() > 1)
                            payload.setClockrate(args[1].toInt());
                        if (args.size() > 2)
                            payload.setChannels(args[2].toInt());
                    }
                }
            } else if (attrName == QStringLiteral("ice-ufrag")) {
                d->transportUser = attrValue;
            } else if (attrName == QStringLiteral("ice-pwd")) {
                d->transportPassword = attrValue;
            } else if (attrName == QStringLiteral("setup")) {
                d->transportFingerprintSetup = attrValue;
            } else if (attrName == QStringLiteral("ssrc")) {
                const QStringList bits = attrValue.split(' ');
                if (bits.isEmpty()) {
                    qWarning() << "Could not parse ssrc" << line;
                    return false;
                }
                d->descriptionSsrc = bits[0].toULong();
            }
        } else if (line.startsWith(QStringLiteral("m="))) {
            // FIXME: what do we do with the profile (bits[2]) ?
            QStringList bits = line.mid(2).split(' ');
            if (bits.size() < 3) {
                qWarning() << "Could not parse media" << line;
                return false;
            }
            d->descriptionMedia = bits[0];

            // parse payload types
            for (int i = 3; i < bits.size(); ++i) {
                bool ok = false;
                int id = bits[i].toInt(&ok);
                if (!ok)
                    continue;
                QXmppJinglePayloadType payload;
                payload.setId(id);
                payloads << payload;
            }
        }
    }
    setPayloadTypes(payloads);
    return true;
}

static bool candidateLessThan(const QXmppJingleCandidate &c1, const QXmppJingleCandidate &c2)
{
    if (c1.type() == c2.type())
        return c1.priority() > c2.priority();
    else
        return c1.type() == QXmppJingleCandidate::ServerReflexiveType;
}

QString QXmppJingleIq::Content::toSdp() const
{
    // get default candidate
    QHostAddress localRtpAddress = QHostAddress::Any;
    quint16 localRtpPort = 0;
    QList<QXmppJingleCandidate> sortedCandidates = d->transportCandidates;
    std::sort(sortedCandidates.begin(), sortedCandidates.end(), candidateLessThan);
    for (const auto &candidate : sortedCandidates) {
        if (candidate.component() == RTP_COMPONENT) {
            localRtpAddress = candidate.host();
            localRtpPort = candidate.port();
            break;
        }
    }

    QStringList sdp;

    // media
    QString payloads;
    QStringList attrs;
    for (const QXmppJinglePayloadType &payload : d->payloadTypes) {
        payloads += " " + QString::number(payload.id());
        QString rtpmap = QString::number(payload.id()) + " " + payload.name() + "/" + QString::number(payload.clockrate());
        if (payload.channels() > 1)
            rtpmap += "/" + QString::number(payload.channels());
        attrs << "a=rtpmap:" + rtpmap;

        // payload parameters
        QStringList paramList;
        const QMap<QString, QString> params = payload.parameters();
        if (payload.name() == QStringLiteral("telephone-event")) {
            if (params.contains(QStringLiteral("events")))
                paramList << params.value(QStringLiteral("events"));
        } else {
            QMap<QString, QString>::const_iterator i;
            for (i = params.begin(); i != params.end(); ++i)
                paramList << i.key() + QStringLiteral("=") + i.value();
        }
        if (!paramList.isEmpty())
            attrs << QStringLiteral("a=fmtp:") + QByteArray::number(payload.id()) + QStringLiteral(" ") + paramList.join("; ");
    }
    sdp << QStringLiteral("m=%1 %2 RTP/AVP%3").arg(d->descriptionMedia, QString::number(localRtpPort), payloads);
    sdp << QStringLiteral("c=%1").arg(addressToSdp(localRtpAddress));
    sdp += attrs;

    // transport
    for (const auto &candidate : d->transportCandidates)
        sdp << QStringLiteral("a=%1").arg(candidateToSdp(candidate));
    if (!d->transportUser.isEmpty())
        sdp << QStringLiteral("a=ice-ufrag:%1").arg(d->transportUser);
    if (!d->transportPassword.isEmpty())
        sdp << QStringLiteral("a=ice-pwd:%1").arg(d->transportPassword);
    if (!d->transportFingerprint.isEmpty() && !d->transportFingerprintHash.isEmpty())
        sdp << QStringLiteral("a=fingerprint:%1 %2").arg(d->transportFingerprintHash, formatFingerprint(d->transportFingerprint));
    if (!d->transportFingerprintSetup.isEmpty())
        sdp << QStringLiteral("a=setup:%1").arg(d->transportFingerprintSetup);

    return sdp.join("\r\n") + "\r\n";
}

/// \endcond

QXmppJingleIq::Reason::Reason()
    : m_type(None)
{
}

/// Returns the reason's textual description.

QString QXmppJingleIq::Reason::text() const
{
    return m_text;
}

/// Sets the reason's textual description.

void QXmppJingleIq::Reason::setText(const QString &text)
{
    m_text = text;
}

/// Gets the reason's type.

QXmppJingleIq::Reason::Type QXmppJingleIq::Reason::type() const
{
    return m_type;
}

/// Sets the reason's type.

void QXmppJingleIq::Reason::setType(QXmppJingleIq::Reason::Type type)
{
    m_type = type;
}

/// \cond
void QXmppJingleIq::Reason::parse(const QDomElement &element)
{
    m_text = element.firstChildElement(QStringLiteral("text")).text();
    for (int i = AlternativeSession; i <= UnsupportedTransports; i++) {
        if (!element.firstChildElement(jingle_reasons[i]).isNull()) {
            m_type = static_cast<Type>(i);
            break;
        }
    }
}

void QXmppJingleIq::Reason::toXml(QXmlStreamWriter *writer) const
{
    if (m_type < AlternativeSession || m_type > UnsupportedTransports)
        return;

    writer->writeStartElement(QStringLiteral("reason"));
    if (!m_text.isEmpty())
        helperToXmlAddTextElement(writer, QStringLiteral("text"), m_text);
    writer->writeEmptyElement(jingle_reasons[m_type]);
    writer->writeEndElement();
}
/// \endcond

class QXmppJingleIqPrivate : public QSharedData
{
public:
    QXmppJingleIqPrivate();

    QXmppJingleIq::Action action;
    QString initiator;
    QString responder;
    QString sid;

    QList<QXmppJingleIq::Content> contents;
    QXmppJingleIq::Reason reason;
    bool ringing;
};

QXmppJingleIqPrivate::QXmppJingleIqPrivate()
    : action(QXmppJingleIq::ContentAccept), ringing(false)
{
}

/// Constructs a QXmppJingleIq.

QXmppJingleIq::QXmppJingleIq()
    : d(new QXmppJingleIqPrivate())
{
}

/// Constructs a copy of other.
///
/// \param other

QXmppJingleIq::QXmppJingleIq(const QXmppJingleIq &other)
    : QXmppIq(other), d(other.d)
{
}

QXmppJingleIq::~QXmppJingleIq()
{
}

/// Assigns the other Jingle IQ to this one.
///
/// \param other

QXmppJingleIq &QXmppJingleIq::operator=(const QXmppJingleIq &other)
{
    d = other.d;
    return *this;
}

/// Returns the Jingle IQ's action.

QXmppJingleIq::Action QXmppJingleIq::action() const
{
    return d->action;
}

/// Sets the Jingle IQ's action.
///
/// \param action

void QXmppJingleIq::setAction(QXmppJingleIq::Action action)
{
    d->action = action;
}

///
/// Adds an element to the IQ's content elements.
///
/// \since QXmpp 0.9.2
///
void QXmppJingleIq::addContent(const QXmppJingleIq::Content &content)
{
    d->contents << content;
}

///
/// Returns the IQ's content elements.
///
/// \since QXmpp 0.9.2
///
QList<QXmppJingleIq::Content> QXmppJingleIq::contents() const
{
    return d->contents;
}

///
/// Sets the IQ's content elements.
///
/// \since QXmpp 0.9.2
///
void QXmppJingleIq::setContents(const QList<QXmppJingleIq::Content> &contents)
{
    d->contents = contents;
}

/// Returns the session initiator.

QString QXmppJingleIq::initiator() const
{
    return d->initiator;
}

/// Sets the session initiator.
///
/// \param initiator

void QXmppJingleIq::setInitiator(const QString &initiator)
{
    d->initiator = initiator;
}

/// Returns a reference to the IQ's reason element.

QXmppJingleIq::Reason &QXmppJingleIq::reason()
{
    return d->reason;
}

/// Returns a const reference to the IQ's reason element.

const QXmppJingleIq::Reason &QXmppJingleIq::reason() const
{
    return d->reason;
}

/// Returns the session responder.

QString QXmppJingleIq::responder() const
{
    return d->responder;
}

/// Sets the session responder.
///
/// \param responder

void QXmppJingleIq::setResponder(const QString &responder)
{
    d->responder = responder;
}

/// Returns true if the call is ringing.

bool QXmppJingleIq::ringing() const
{
    return d->ringing;
}

/// Set to true if the call is ringing.
///
/// \param ringing

void QXmppJingleIq::setRinging(bool ringing)
{
    d->ringing = ringing;
}

/// Returns the session ID.

QString QXmppJingleIq::sid() const
{
    return d->sid;
}

/// Sets the session ID.
///
/// \param sid

void QXmppJingleIq::setSid(const QString &sid)
{
    d->sid = sid;
}

/// \cond
bool QXmppJingleIq::isJingleIq(const QDomElement &element)
{
    QDomElement jingleElement = element.firstChildElement(QStringLiteral("jingle"));
    return (jingleElement.namespaceURI() == ns_jingle);
}

void QXmppJingleIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement jingleElement = element.firstChildElement(QStringLiteral("jingle"));
    const QString action = jingleElement.attribute(QStringLiteral("action"));
    for (int i = ContentAccept; i <= TransportReplace; i++) {
        if (action == jingle_actions[i]) {
            d->action = static_cast<Action>(i);
            break;
        }
    }
    d->initiator = jingleElement.attribute(QStringLiteral("initiator"));
    d->responder = jingleElement.attribute(QStringLiteral("responder"));
    d->sid = jingleElement.attribute(QStringLiteral("sid"));

    // content
    d->contents.clear();
    QDomElement contentElement = jingleElement.firstChildElement(QStringLiteral("content"));
    while (!contentElement.isNull()) {
        QXmppJingleIq::Content content;
        content.parse(contentElement);
        addContent(content);
        contentElement = contentElement.nextSiblingElement(QStringLiteral("content"));
    }
    QDomElement reasonElement = jingleElement.firstChildElement(QStringLiteral("reason"));
    d->reason.parse(reasonElement);

    // ringing
    QDomElement ringingElement = jingleElement.firstChildElement(QStringLiteral("ringing"));
    d->ringing = (ringingElement.namespaceURI() == ns_jingle_rtp_info);
}

void QXmppJingleIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("jingle"));
    writer->writeDefaultNamespace(ns_jingle);
    helperToXmlAddAttribute(writer, QStringLiteral("action"), jingle_actions[d->action]);
    helperToXmlAddAttribute(writer, QStringLiteral("initiator"), d->initiator);
    helperToXmlAddAttribute(writer, QStringLiteral("responder"), d->responder);
    helperToXmlAddAttribute(writer, QStringLiteral("sid"), d->sid);
    for (const auto &content : d->contents)
        content.toXml(writer);
    d->reason.toXml(writer);

    // ringing
    if (d->ringing) {
        writer->writeStartElement(QStringLiteral("ringing"));
        writer->writeDefaultNamespace(ns_jingle_rtp_info);
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond

class QXmppJingleCandidatePrivate : public QSharedData
{
public:
    QXmppJingleCandidatePrivate();

    int component;
    QString foundation;
    int generation;
    QHostAddress host;
    QString id;
    int network;
    quint16 port;
    QString protocol;
    int priority;
    QXmppJingleCandidate::Type type;
};

QXmppJingleCandidatePrivate::QXmppJingleCandidatePrivate()
    : component(0), generation(0), network(0), port(0), priority(0), type(QXmppJingleCandidate::HostType)
{
}

/// Constructs an empty candidate.

QXmppJingleCandidate::QXmppJingleCandidate()
    : d(new QXmppJingleCandidatePrivate())
{
}

/// Constructs a copy of other.
///
/// \param other

QXmppJingleCandidate::QXmppJingleCandidate(const QXmppJingleCandidate &other)
    : d(other.d)
{
}

QXmppJingleCandidate::~QXmppJingleCandidate()
{
}

/// Assigns the other candidate to this one.
///
/// \param other

QXmppJingleCandidate &QXmppJingleCandidate::operator=(const QXmppJingleCandidate &other)
{
    d = other.d;
    return *this;
}

/// Returns the candidate's component ID.

int QXmppJingleCandidate::component() const
{
    return d->component;
}

/// Sets the candidates's component ID.
///
/// \param component

void QXmppJingleCandidate::setComponent(int component)
{
    d->component = component;
}

/// Returns the candidate's foundation.

QString QXmppJingleCandidate::foundation() const
{
    return d->foundation;
}

/// Sets the candidate's foundation.
///
/// \param foundation

void QXmppJingleCandidate::setFoundation(const QString &foundation)
{
    d->foundation = foundation;
}

/// Returns the candidate's generation.

int QXmppJingleCandidate::generation() const
{
    return d->generation;
}

/// Sets the candidate's generation.
///
/// \param generation

void QXmppJingleCandidate::setGeneration(int generation)
{
    d->generation = generation;
}

/// Returns the candidate's host address.
///

QHostAddress QXmppJingleCandidate::host() const
{
    return d->host;
}

/// Sets the candidate's host address.
///
/// \param host

void QXmppJingleCandidate::setHost(const QHostAddress &host)
{
    d->host = host;
}

/// Returns the candidate's unique identifier.
///

QString QXmppJingleCandidate::id() const
{
    return d->id;
}

/// Sets the candidate's unique identifier.
///
/// \param id

void QXmppJingleCandidate::setId(const QString &id)
{
    d->id = id;
}

/// Returns the network index (starting at 0) the candidate is on.
///

int QXmppJingleCandidate::network() const
{
    return d->network;
}

/// Sets the network index (starting at 0) the candidate is on.
///
/// \param network

void QXmppJingleCandidate::setNetwork(int network)
{
    d->network = network;
}

/// Returns the candidate's port number.
///

quint16 QXmppJingleCandidate::port() const
{
    return d->port;
}

/// Sets the candidate's port number.
///
/// \param port

void QXmppJingleCandidate::setPort(quint16 port)
{
    d->port = port;
}

/// Returns the candidate's priority.
///

int QXmppJingleCandidate::priority() const
{
    return d->priority;
}

/// Sets the candidate's priority.
///
/// \param priority

void QXmppJingleCandidate::setPriority(int priority)
{
    d->priority = priority;
}

/// Returns the candidate's protocol (e.g. "udp").
///

QString QXmppJingleCandidate::protocol() const
{
    return d->protocol;
}

/// Sets the candidate's protocol (e.g. "udp").
///
/// \param protocol

void QXmppJingleCandidate::setProtocol(const QString &protocol)
{
    d->protocol = protocol;
}

/// Returns the candidate type (e.g. "host").
///

QXmppJingleCandidate::Type QXmppJingleCandidate::type() const
{
    return d->type;
}

/// Sets the candidate type (e.g. "host").
///
/// \param type

void QXmppJingleCandidate::setType(QXmppJingleCandidate::Type type)
{
    d->type = type;
}

/// Returns true if the host address or port are empty.
///

bool QXmppJingleCandidate::isNull() const
{
    return d->host.isNull() || !d->port;
}

/// \cond
void QXmppJingleCandidate::parse(const QDomElement &element)
{
    d->component = element.attribute(QStringLiteral("component")).toInt();
    d->foundation = element.attribute(QStringLiteral("foundation"));
    d->generation = element.attribute(QStringLiteral("generation")).toInt();
    d->host = QHostAddress(element.attribute(QStringLiteral("ip")));
    d->id = element.attribute(QStringLiteral("id"));
    d->network = element.attribute(QStringLiteral("network")).toInt();
    d->port = element.attribute(QStringLiteral("port")).toInt();
    d->priority = element.attribute(QStringLiteral("priority")).toInt();
    d->protocol = element.attribute(QStringLiteral("protocol"));
    d->type = typeFromString(element.attribute(QStringLiteral("type")));
}

void QXmppJingleCandidate::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("candidate"));
    helperToXmlAddAttribute(writer, QStringLiteral("component"), QString::number(d->component));
    helperToXmlAddAttribute(writer, QStringLiteral("foundation"), d->foundation);
    helperToXmlAddAttribute(writer, QStringLiteral("generation"), QString::number(d->generation));
    helperToXmlAddAttribute(writer, QStringLiteral("id"), d->id);
    helperToXmlAddAttribute(writer, QStringLiteral("ip"), d->host.toString());
    helperToXmlAddAttribute(writer, QStringLiteral("network"), QString::number(d->network));
    helperToXmlAddAttribute(writer, QStringLiteral("port"), QString::number(d->port));
    helperToXmlAddAttribute(writer, QStringLiteral("priority"), QString::number(d->priority));
    helperToXmlAddAttribute(writer, QStringLiteral("protocol"), d->protocol);
    helperToXmlAddAttribute(writer, QStringLiteral("type"), typeToString(d->type));
    writer->writeEndElement();
}

QXmppJingleCandidate::Type QXmppJingleCandidate::typeFromString(const QString &typeStr, bool *ok)
{
    QXmppJingleCandidate::Type type;
    if (typeStr == QStringLiteral("host"))
        type = HostType;
    else if (typeStr == QStringLiteral("prflx"))
        type = PeerReflexiveType;
    else if (typeStr == QStringLiteral("srflx"))
        type = ServerReflexiveType;
    else if (typeStr == QStringLiteral("relay"))
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
    switch (type) {
    case HostType:
        typeStr = QStringLiteral("host");
        break;
    case PeerReflexiveType:
        typeStr = QStringLiteral("prflx");
        break;
    case ServerReflexiveType:
        typeStr = QStringLiteral("srflx");
        break;
    case RelayedType:
        typeStr = QStringLiteral("relay");
        break;
    }
    return typeStr;
}
/// \endcond

class QXmppJinglePayloadTypePrivate : public QSharedData
{
public:
    QXmppJinglePayloadTypePrivate();

    unsigned char channels;
    unsigned int clockrate;
    unsigned char id;
    unsigned int maxptime;
    QString name;
    QMap<QString, QString> parameters;
    unsigned int ptime;
};

QXmppJinglePayloadTypePrivate::QXmppJinglePayloadTypePrivate()
    : channels(1), clockrate(0), id(0), maxptime(0), ptime(0)
{
}

QXmppJinglePayloadType::QXmppJinglePayloadType()
    : d(new QXmppJinglePayloadTypePrivate())
{
}

/// Constructs a copy of other.
///
/// \param other

QXmppJinglePayloadType::QXmppJinglePayloadType(const QXmppJinglePayloadType &other)
    : d(other.d)
{
}

QXmppJinglePayloadType::~QXmppJinglePayloadType()
{
}

/// Returns the number of channels (e.g. 1 for mono, 2 for stereo).
///

unsigned char QXmppJinglePayloadType::channels() const
{
    return d->channels;
}

/// Sets the number of channels (e.g. 1 for mono, 2 for stereo).
///
/// \param channels

void QXmppJinglePayloadType::setChannels(unsigned char channels)
{
    d->channels = channels;
}

/// Returns the clockrate in Hz, i.e. the number of samples per second.
///

unsigned int QXmppJinglePayloadType::clockrate() const
{
    return d->clockrate;
}

/// Sets the clockrate in Hz, i.e. the number of samples per second.
///
/// \param clockrate

void QXmppJinglePayloadType::setClockrate(unsigned int clockrate)
{
    d->clockrate = clockrate;
}

/// Returns the payload type identifier.
///

unsigned char QXmppJinglePayloadType::id() const
{
    return d->id;
}

/// Sets the payload type identifier.
///

void QXmppJinglePayloadType::setId(unsigned char id)
{
    Q_ASSERT(id <= 127);
    d->id = id;
}

/// Returns the maximum packet time in milliseconds.
///

unsigned int QXmppJinglePayloadType::maxptime() const
{
    return d->maxptime;
}

/// Sets the maximum packet type in milliseconds.
///
/// \param maxptime

void QXmppJinglePayloadType::setMaxptime(unsigned int maxptime)
{
    d->maxptime = maxptime;
}

/// Returns the payload type name.
///

QString QXmppJinglePayloadType::name() const
{
    return d->name;
}

/// Sets the payload type name.
///
/// \param name

void QXmppJinglePayloadType::setName(const QString &name)
{
    d->name = name;
}

/// Returns the payload parameters.

QMap<QString, QString> QXmppJinglePayloadType::parameters() const
{
    return d->parameters;
}

/// Sets the payload parameters.

void QXmppJinglePayloadType::setParameters(const QMap<QString, QString> &parameters)
{
    d->parameters = parameters;
}

/// Returns the packet time in milliseconds (20 by default).
///

unsigned int QXmppJinglePayloadType::ptime() const
{
    return d->ptime ? d->ptime : 20;
}

/// Sets the packet time in milliseconds (20 by default).
///
/// \param ptime

void QXmppJinglePayloadType::setPtime(unsigned int ptime)
{
    d->ptime = ptime;
}

/// \cond
void QXmppJinglePayloadType::parse(const QDomElement &element)
{
    d->id = element.attribute(QStringLiteral("id")).toInt();
    d->name = element.attribute(QStringLiteral("name"));
    d->channels = element.attribute(QStringLiteral("channels")).toInt();
    if (!d->channels)
        d->channels = 1;
    d->clockrate = element.attribute(QStringLiteral("clockrate")).toInt();
    d->maxptime = element.attribute(QStringLiteral("maxptime")).toInt();
    d->ptime = element.attribute(QStringLiteral("ptime")).toInt();

    QDomElement child = element.firstChildElement(QStringLiteral("parameter"));
    while (!child.isNull()) {
        d->parameters.insert(child.attribute(QStringLiteral("name")), child.attribute(QStringLiteral("value")));
        child = child.nextSiblingElement(QStringLiteral("parameter"));
    }
}

void QXmppJinglePayloadType::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("payload-type"));
    helperToXmlAddAttribute(writer, QStringLiteral("id"), QString::number(d->id));
    helperToXmlAddAttribute(writer, QStringLiteral("name"), d->name);
    if (d->channels > 1)
        helperToXmlAddAttribute(writer, QStringLiteral("channels"), QString::number(d->channels));
    if (d->clockrate > 0)
        helperToXmlAddAttribute(writer, QStringLiteral("clockrate"), QString::number(d->clockrate));
    if (d->maxptime > 0)
        helperToXmlAddAttribute(writer, QStringLiteral("maxptime"), QString::number(d->maxptime));
    if (d->ptime > 0)
        helperToXmlAddAttribute(writer, QStringLiteral("ptime"), QString::number(d->ptime));

    for (const auto &key : d->parameters.keys()) {
        writer->writeStartElement(QStringLiteral("parameter"));
        writer->writeAttribute(QStringLiteral("name"), key);
        writer->writeAttribute(QStringLiteral("value"), d->parameters.value(key));
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
/// \endcond

/// Assigns the other payload type to this one.
///
/// \param other

QXmppJinglePayloadType &QXmppJinglePayloadType::operator=(const QXmppJinglePayloadType &other)
{
    d = other.d;
    return *this;
}

/// Returns true if this QXmppJinglePayloadType and \a other refer to the same payload type.
///
/// \param other

bool QXmppJinglePayloadType::operator==(const QXmppJinglePayloadType &other) const
{
    // FIXME : what to do with m_ptime and m_maxptime?
    if (d->id <= 95)
        return other.d->id == d->id && other.d->clockrate == d->clockrate;
    else
        return other.d->channels == d->channels &&
            other.d->clockrate == d->clockrate &&
            other.d->name.toLower() == d->name.toLower();
}
