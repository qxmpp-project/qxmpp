// SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSTREAMMANAGEMENT_P_H
#define QXMPPSTREAMMANAGEMENT_P_H

#include "QXmppGlobal.h"
#include "QXmppSendResult.h"
#include "QXmppStanza.h"
#include "QXmppTask.h"

#include <QDomDocument>
#include <QXmlStreamWriter>

class QXmppStream;
class QXmppPacket;

namespace QXmpp::Private {
class XmppSocket;
}

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.  It exists for the convenience
// of the QXmppIncomingClient and QXmppOutgoingClient classes.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

namespace QXmpp::Private {

struct SmEnable {
    static std::optional<SmEnable> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *w) const;

    bool resume = false;
    quint64 max = 0;
};

struct SmEnabled {
    static std::optional<SmEnabled> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *w) const;

    bool resume = false;
    QString id;
    quint64 max = 0;
    QString location;
};

struct SmResume {
    static std::optional<SmResume> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *w) const;

    quint32 h = 0;
    QString previd;
};

struct SmResumed {
    static std::optional<SmResumed> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *w) const;

    quint32 h = 0;
    QString previd;
};

struct SmFailed {
    static std::optional<SmFailed> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *w) const;

    std::optional<QXmppStanza::Error::Condition> error;
};

struct SmAck {
    static std::optional<SmAck> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *w) const;

    quint32 seqNo = 0;
};

struct SmRequest {
    static std::optional<SmRequest> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *w) const;
};

//
// This manager handles sending and receiving of stream management acks.
// Enabling of stream management and stream resumption is done in the C2sStreamManager.
//
class StreamAckManager
{
public:
    explicit StreamAckManager(XmppSocket &socket);

    bool enabled() const { return m_enabled; }
    unsigned int lastIncomingSequenceNumber() const { return m_lastIncomingSequenceNumber; }

    void handlePacketSent(QXmppPacket &packet, bool sentData);
    bool handleStanza(const QDomElement &stanza);
    void onSessionClosed();

    void resetCache();
    void enableStreamManagement(bool resetSequenceNumber);
    void setAcknowledgedSequenceNumber(unsigned int sequenceNumber);

    QXmppTask<QXmpp::SendResult> send(QXmppPacket &&);
    bool sendPacketCompat(QXmppPacket &&);
    std::tuple<bool, QXmppTask<QXmpp::SendResult>> internalSend(QXmppPacket &&);

    void sendAcknowledgementRequest();

private:
    void handleAcknowledgement(SmAck ack);

    void sendAcknowledgement();

    QXmpp::Private::XmppSocket &socket;

    bool m_enabled = false;
    QMap<unsigned int, QXmppPacket> m_unacknowledgedStanzas;
    unsigned int m_lastOutgoingSequenceNumber = 0;
    unsigned int m_lastIncomingSequenceNumber = 0;
};

}  // namespace QXmpp::Private

#endif
