// SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppGlobal.h"
#include "QXmppPacket_p.h"
#include "QXmppStanza_p.h"
#include "QXmppStreamManagement_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"
#include "XmppSocket.h"

namespace QXmpp::Private {

std::optional<SmEnable> SmEnable::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"enable" || el.namespaceURI() != ns_stream_management) {
        return {};
    }

    const auto resume = el.attribute(u"resume"_s);
    return SmEnable {
        .resume = resume == u"true" || resume == u"1",
        .max = el.attribute(u"max"_s).toULongLong(),
    };
}

void SmEnable::toXml(QXmlStreamWriter *w) const
{
    w->writeStartElement(QSL65("enable"));
    w->writeDefaultNamespace(toString65(ns_stream_management));
    if (resume) {
        w->writeAttribute(QSL65("resume"), u"true"_s);
    }
    if (max > 0) {
        w->writeAttribute(QSL65("max"), QString::number(max));
    }
    w->writeEndElement();
}

std::optional<SmEnabled> SmEnabled::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"enabled" || el.namespaceURI() != ns_stream_management) {
        return {};
    }

    const auto resume = el.attribute(u"resume"_s);
    return SmEnabled {
        .resume = (resume == u"true" || resume == u"1"),
        .id = el.attribute(u"id"_s),
        .max = el.attribute(u"max"_s).toULongLong(),
        .location = el.attribute(u"location"_s),
    };
}

void SmEnabled::toXml(QXmlStreamWriter *w) const
{
    w->writeStartElement(QSL65("enable"));
    w->writeDefaultNamespace(toString65(ns_stream_management));
    if (resume) {
        w->writeAttribute(QSL65("resume"), u"true"_s);
    }
    writeOptionalXmlAttribute(w, u"id", id);
    if (max > 0) {
        w->writeAttribute(QSL65("max"), QString::number(max));
    }
    if (!location.isEmpty()) {
        w->writeAttribute(QSL65("location"), location);
    }
    w->writeEndElement();
}

std::optional<SmResume> SmResume::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"resume" || el.namespaceURI() != ns_stream_management) {
        return {};
    }
    return SmResume {
        el.attribute(u"h"_s).toUInt(),
        el.attribute(u"previd"_s),
    };
}

void SmResume::toXml(QXmlStreamWriter *w) const
{
    w->writeStartElement(QSL65("resume"));
    w->writeDefaultNamespace(toString65(ns_stream_management));
    w->writeAttribute(QSL65("h"), QString::number(h));
    w->writeAttribute(QSL65("previd"), previd);
    w->writeEndElement();
}

std::optional<SmResumed> SmResumed::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"resumed" || el.namespaceURI() != ns_stream_management) {
        return {};
    }
    return SmResumed {
        el.attribute(u"h"_s).toUInt(),
        el.attribute(u"previd"_s),
    };
}

void SmResumed::toXml(QXmlStreamWriter *w) const
{
    w->writeStartElement(QSL65("resumed"));
    w->writeDefaultNamespace(toString65(ns_stream_management));
    w->writeAttribute(QSL65("h"), QString::number(h));
    w->writeAttribute(QSL65("previd"), previd);
    w->writeEndElement();
}

std::optional<SmFailed> SmFailed::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"failed" || el.namespaceURI() != ns_stream_management) {
        return {};
    }

    return SmFailed {
        conditionFromString(firstChildElement(el, {}, ns_stanza).tagName()),
    };
}

void SmFailed::toXml(QXmlStreamWriter *w) const
{
    w->writeStartElement(QSL65("failed"));
    w->writeDefaultNamespace(toString65(ns_stream_management));
    if (error) {
        writeEmptyElement(w, conditionToString(*error), ns_stanza);
    }
    w->writeEndElement();
}

std::optional<SmAck> SmAck::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"a" || el.namespaceURI() != ns_stream_management) {
        return {};
    }
    return SmAck { el.attribute(u"h"_s).toUInt() };
}

void SmAck::toXml(QXmlStreamWriter *w) const
{
    w->writeStartElement(QSL65("a"));
    w->writeDefaultNamespace(toString65(ns_stream_management));
    w->writeAttribute(QSL65("h"), QString::number(seqNo));
    w->writeEndElement();
}

std::optional<SmRequest> SmRequest::fromDom(const QDomElement &el)
{
    if (el.tagName() == u"r" && el.namespaceURI() == ns_stream_management) {
        return SmRequest();
    }
    return {};
}

void SmRequest::toXml(QXmlStreamWriter *w) const
{
    w->writeStartElement(QSL65("r"));
    w->writeDefaultNamespace(toString65(ns_stream_management));
    w->writeEndElement();
}

StreamAckManager::StreamAckManager(XmppSocket &socket)
    : socket(socket)
{
}

bool StreamAckManager::handleStanza(const QDomElement &stanza)
{
    if (auto ack = SmAck::fromDom(stanza)) {
        handleAcknowledgement(*ack);
        return true;
    }
    if (auto req = SmRequest::fromDom(stanza)) {
        sendAcknowledgement();
        return true;
    }

    auto tagName = stanza.tagName();
    if (tagName == u"message" || tagName == u"presence" || tagName == u"iq") {
        m_lastIncomingSequenceNumber++;
    }
    return false;
}

void StreamAckManager::onSessionClosed()
{
    m_enabled = false;
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
    // the writtenToSocket parameter is just for backwards compat
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
                u"Couldn't write data to socket. No stream management enabled."_s,
                QXmpp::SendError::SocketWriteError,
            });
        }
    }

    return { writtenToSocket, packet.task() };
}

void StreamAckManager::handleAcknowledgement(SmAck ack)
{
    if (!m_enabled) {
        return;
    }

    setAcknowledgedSequenceNumber(ack.seqNo);
}

void StreamAckManager::sendAcknowledgement()
{
    if (!m_enabled) {
        return;
    }

    socket.sendData(serializeXml(SmAck { m_lastIncomingSequenceNumber }));
}

void StreamAckManager::sendAcknowledgementRequest()
{
    if (!m_enabled) {
        return;
    }

    // send packet
    socket.sendData(serializeXml(SmRequest {}));
}

void StreamAckManager::resetCache()
{
    for (auto &packet : m_unacknowledgedStanzas) {
        packet.reportFinished(QXmppError {
            u"Disconnected"_s,
            QXmpp::SendError::Disconnected });
    }

    m_unacknowledgedStanzas.clear();
}

}  // namespace QXmpp::Private
