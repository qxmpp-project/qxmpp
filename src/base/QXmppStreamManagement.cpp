// SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppGlobal.h"
#include "QXmppPacket_p.h"
#include "QXmppStanza_p.h"
#include "QXmppStream.h"
#include "QXmppStreamManagement_p.h"
#include "QXmppUtils_p.h"

#include "XmppSocket.h"

using namespace QXmpp::Private;

/// \cond
QXmppStreamManagementEnable::QXmppStreamManagementEnable(const bool resume, const unsigned max)
    : m_resume(resume), m_max(max)
{
}

bool QXmppStreamManagementEnable::resume() const
{
    return m_resume;
}

void QXmppStreamManagementEnable::setResume(bool resume)
{
    m_resume = resume;
}

unsigned QXmppStreamManagementEnable::max() const
{
    return m_max;
}

void QXmppStreamManagementEnable::setMax(const unsigned max)
{
    m_max = max;
}

bool QXmppStreamManagementEnable::isStreamManagementEnable(const QDomElement &element)
{
    return element.tagName() == QLatin1String("enable") &&
        element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementEnable::parse(const QDomElement &element)
{
    QString resume = element.attribute(QStringLiteral("resume"));
    m_resume = resume == QStringLiteral("true") || resume == QStringLiteral("1");
    m_max = element.attribute(QStringLiteral("max")).toUInt();
}

void QXmppStreamManagementEnable::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("enable"));
    writer->writeDefaultNamespace(toString65(ns_stream_management));
    if (m_resume) {
        writer->writeAttribute(QSL65("resume"), QStringLiteral("true"));
    }
    if (m_max > 0) {
        writer->writeAttribute(QSL65("max"), QString::number(m_max));
    }
    writer->writeEndElement();
}

QXmppStreamManagementEnabled::QXmppStreamManagementEnabled(const bool resume, const QString id, const unsigned max, const QString location)
    : m_resume(resume), m_id(id), m_max(max), m_location(location)
{
}

bool QXmppStreamManagementEnabled::resume() const
{
    return m_resume;
}

void QXmppStreamManagementEnabled::setResume(const bool resume)
{
    m_resume = resume;
}

QString QXmppStreamManagementEnabled::id() const
{
    return m_id;
}

void QXmppStreamManagementEnabled::setId(const QString id)
{
    m_id = id;
}

unsigned QXmppStreamManagementEnabled::max() const
{
    return m_max;
}

void QXmppStreamManagementEnabled::setMax(const unsigned max)
{
    m_max = max;
}

QString QXmppStreamManagementEnabled::location() const
{
    return m_location;
}

void QXmppStreamManagementEnabled::setLocation(const QString location)
{
    m_location = location;
}

bool QXmppStreamManagementEnabled::isStreamManagementEnabled(const QDomElement &element)
{
    return element.tagName() == QLatin1String("enabled") &&
        element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementEnabled::parse(const QDomElement &element)
{
    QString resume = element.attribute(QStringLiteral("resume"));
    m_resume = resume == QStringLiteral("true") || resume == QStringLiteral("1");
    m_max = element.attribute(QStringLiteral("max")).toUInt();
    m_location = element.attribute(QStringLiteral("location"));
}

void QXmppStreamManagementEnabled::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("enable"));
    writer->writeDefaultNamespace(toString65(ns_stream_management));
    if (m_resume) {
        writer->writeAttribute(QSL65("resume"), QStringLiteral("true"));
    }
    if (m_max > 0) {
        writer->writeAttribute(QSL65("max"), QString::number(m_max));
    }
    if (!m_location.isEmpty()) {
        writer->writeAttribute(QSL65("location"), m_location);
    }
    writer->writeEndElement();
}

QXmppStreamManagementResume::QXmppStreamManagementResume(const unsigned h, const QString &previd)
    : m_h(h), m_previd(previd)
{
}

unsigned QXmppStreamManagementResume::h() const
{
    return m_h;
}

void QXmppStreamManagementResume::setH(const unsigned h)
{
    m_h = h;
}

QString QXmppStreamManagementResume::prevId() const
{
    return m_previd;
}

void QXmppStreamManagementResume::setPrevId(const QString &previd)
{
    m_previd = previd;
}

bool QXmppStreamManagementResume::isStreamManagementResume(const QDomElement &element)
{
    return element.tagName() == QLatin1String("resume") &&
        element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementResume::parse(const QDomElement &element)
{
    m_h = element.attribute(QStringLiteral("h")).toUInt();
    m_previd = element.attribute(QStringLiteral("previd"));
}

void QXmppStreamManagementResume::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("resume"));
    writer->writeAttribute(QSL65("h"), QString::number(m_h));
    writer->writeAttribute(QSL65("previd"), m_previd);
    writer->writeEndElement();
}

QXmppStreamManagementResumed::QXmppStreamManagementResumed(const unsigned h, const QString &previd)
    : m_h(h), m_previd(previd)
{
}

unsigned QXmppStreamManagementResumed::h() const
{
    return m_h;
}

void QXmppStreamManagementResumed::setH(const unsigned h)
{
    m_h = h;
}

QString QXmppStreamManagementResumed::prevId() const
{
    return m_previd;
}

void QXmppStreamManagementResumed::setPrevId(const QString &previd)
{
    m_previd = previd;
}

bool QXmppStreamManagementResumed::isStreamManagementResumed(const QDomElement &element)
{
    return element.tagName() == QLatin1String("resumed") &&
        element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementResumed::parse(const QDomElement &element)
{
    m_h = element.attribute(QStringLiteral("h")).toUInt();
    m_previd = element.attribute(QStringLiteral("previd"));
}

void QXmppStreamManagementResumed::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("resumed"));
    writer->writeAttribute(QSL65("h"), QString::number(m_h));
    writer->writeAttribute(QSL65("previd"), m_previd);
    writer->writeEndElement();
}

QXmppStreamManagementFailed::QXmppStreamManagementFailed(const QXmppStanza::Error::Condition error)
    : m_error(error)
{
}

QXmppStanza::Error::Condition QXmppStreamManagementFailed::error() const
{
    return m_error;
}

void QXmppStreamManagementFailed::setError(const QXmppStanza::Error::Condition error)
{
    m_error = error;
}

bool QXmppStreamManagementFailed::isStreamManagementFailed(const QDomElement &element)
{
    return element.tagName() == QLatin1String("failed") &&
        element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementFailed::parse(const QDomElement &element)
{
    QDomElement childElement = element.firstChildElement();
    if (!childElement.isNull() && childElement.namespaceURI() == ns_stanza) {
        m_error = conditionFromString(childElement.tagName()).value_or(QXmppStanza::Error::Condition(-1));
    }
}

void QXmppStreamManagementFailed::toXml(QXmlStreamWriter *writer) const
{
    QString errorString = conditionToString(m_error);

    writer->writeStartElement(QSL65("failed"));
    writer->writeDefaultNamespace(toString65(ns_stream_management));
    writer->writeStartElement(errorString);
    writer->writeDefaultNamespace(toString65(ns_stanza));
    writer->writeEndElement();
    writer->writeEndElement();
}

QXmppStreamManagementAck::QXmppStreamManagementAck(const unsigned seqNo)
    : m_seqNo(seqNo)
{
}

unsigned QXmppStreamManagementAck::seqNo() const
{
    return m_seqNo;
}

void QXmppStreamManagementAck::setSeqNo(const unsigned seqNo)
{
    m_seqNo = seqNo;
}

void QXmppStreamManagementAck::parse(const QDomElement &element)
{
    m_seqNo = element.attribute(QStringLiteral("h")).toUInt();
}

void QXmppStreamManagementAck::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("a"));
    writer->writeDefaultNamespace(toString65(ns_stream_management));
    writer->writeAttribute(QSL65("h"), QString::number(m_seqNo));
    writer->writeEndElement();
}

bool QXmppStreamManagementAck::isStreamManagementAck(const QDomElement &element)
{
    return element.tagName() == QLatin1String("a") &&
        element.namespaceURI() == ns_stream_management;
}

bool QXmppStreamManagementReq::isStreamManagementReq(const QDomElement &element)
{
    return element.tagName() == QLatin1String("r") &&
        element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementReq::toXml(QXmlStreamWriter *writer)
{
    writer->writeStartElement(QSL65("r"));
    writer->writeDefaultNamespace(toString65(ns_stream_management));
    writer->writeEndElement();
}

namespace QXmpp::Private {

StreamAckManager::StreamAckManager(XmppSocket &socket)
    : socket(socket)
{
}

StreamAckManager::~StreamAckManager()
{
}

bool StreamAckManager::enabled() const
{
    return m_enabled;
}

unsigned int StreamAckManager::lastIncomingSequenceNumber() const
{
    return m_lastIncomingSequenceNumber;
}

void StreamAckManager::handleDisconnect()
{
    m_enabled = false;
}

void StreamAckManager::handleStart()
{
    m_enabled = false;
}

bool StreamAckManager::handleStanza(const QDomElement &stanza)
{
    if (QXmppStreamManagementAck::isStreamManagementAck(stanza)) {
        handleAcknowledgement(stanza);
        return true;
    }
    if (QXmppStreamManagementReq::isStreamManagementReq(stanza)) {
        sendAcknowledgement();
        return true;
    }

    if (stanza.tagName() == QLatin1String("message") ||
        stanza.tagName() == QLatin1String("presence") ||
        stanza.tagName() == QLatin1String("iq")) {
        m_lastIncomingSequenceNumber++;
    }
    return false;
}

void StreamAckManager::enableStreamManagement(bool resetSequenceNumber)
{
    m_enabled = true;

    if (resetSequenceNumber) {
        m_lastOutgoingSequenceNumber = 0;
        m_lastIncomingSequenceNumber = 0;

        // resend unacked stanzas
        if (!m_unacknowledgedStanzas.isEmpty()) {
            auto oldUnackedStanzas = m_unacknowledgedStanzas;
            m_unacknowledgedStanzas.clear();

            for (auto &packet : oldUnackedStanzas) {
                m_unacknowledgedStanzas.insert(++m_lastOutgoingSequenceNumber, packet);
                socket.sendData(packet.data());
            }

            sendAcknowledgementRequest();
        }
    } else {
        // resend unacked stanzas
        if (!m_unacknowledgedStanzas.isEmpty()) {
            for (auto &packet : m_unacknowledgedStanzas) {
                socket.sendData(packet.data());
            }

            sendAcknowledgementRequest();
        }
    }
}

void StreamAckManager::setAcknowledgedSequenceNumber(unsigned int sequenceNumber)
{
    for (auto it = m_unacknowledgedStanzas.begin(); it != m_unacknowledgedStanzas.end();) {
        if (it.key() <= sequenceNumber) {
            it->reportFinished(QXmpp::SendSuccess { true });
            it = m_unacknowledgedStanzas.erase(it);
        } else {
            break;
        }
    }
}

QXmppTask<SendResult> StreamAckManager::send(QXmppPacket &&packet)
{
    return std::get<1>(internalSend(std::move(packet)));
}

bool StreamAckManager::sendPacketCompat(QXmppPacket &&packet)
{
    return std::get<0>(internalSend(std::move(packet)));
}

// Returns written to socket (bool) and QXmppTask
std::tuple<bool, QXmppTask<SendResult>> StreamAckManager::internalSend(QXmppPacket &&packet)
{
    // the writtenToSocket parameter is just for backwards compat (see
    // QXmppStream::sendPacket())
    bool writtenToSocket = socket.sendData(packet.data());

    // handle stream management
    if (m_enabled && packet.isXmppStanza()) {
        m_unacknowledgedStanzas.insert(++m_lastOutgoingSequenceNumber, packet);
        sendAcknowledgementRequest();
    } else {
        if (writtenToSocket) {
            packet.reportFinished(QXmpp::SendSuccess { false });
        } else {
            packet.reportFinished(QXmppError {
                QStringLiteral("Couldn't write data to socket. No stream management enabled."),
                QXmpp::SendError::SocketWriteError,
            });
        }
    }

    return { writtenToSocket, packet.task() };
}

void StreamAckManager::handleAcknowledgement(const QDomElement &element)
{
    if (!m_enabled) {
        return;
    }

    QXmppStreamManagementAck ack;
    ack.parse(element);
    setAcknowledgedSequenceNumber(ack.seqNo());
}

void StreamAckManager::sendAcknowledgement()
{
    if (!m_enabled) {
        return;
    }

    socket.sendData(serializeXml(QXmppStreamManagementAck(m_lastIncomingSequenceNumber)));
}

void StreamAckManager::sendAcknowledgementRequest()
{
    if (!m_enabled) {
        return;
    }

    // prepare packet
    QByteArray data;
    QXmlStreamWriter xmlStream(&data);
    QXmppStreamManagementReq::toXml(&xmlStream);

    // send packet
    socket.sendData(data);
}

void StreamAckManager::resetCache()
{
    for (auto &packet : m_unacknowledgedStanzas) {
        packet.reportFinished(QXmppError {
            QStringLiteral("Disconnected"),
            QXmpp::SendError::Disconnected });
    }

    m_unacknowledgedStanzas.clear();
}

}  // namespace QXmpp::Private
/// \endcond
