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

/// \cond
class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementEnable : public QXmppNonza
{
public:
    explicit QXmppStreamManagementEnable(const bool resume = false, const unsigned max = 0);

    bool resume() const;
    void setResume(const bool resume);

    unsigned max() const;
    void setMax(const unsigned max);

    static bool isStreamManagementEnable(const QDomElement &element);

    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;

private:
    bool m_resume;
    unsigned m_max;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementEnabled : public QXmppNonza
{
public:
    explicit QXmppStreamManagementEnabled(const bool resume = false, const QString id = QString(),
                                          const unsigned max = 0, const QString location = QString());

    bool resume() const;
    void setResume(const bool resume);

    QString id() const;
    void setId(const QString id);

    unsigned max() const;
    void setMax(const unsigned max);

    QString location() const;
    void setLocation(const QString location);

    static bool isStreamManagementEnabled(const QDomElement &element);

    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;

private:
    bool m_resume;
    QString m_id;
    unsigned m_max;
    QString m_location;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementResume : public QXmppNonza
{
public:
    explicit QXmppStreamManagementResume(const unsigned h = 0, const QString &previd = QString());

    unsigned h() const;
    void setH(const unsigned h);

    QString prevId() const;
    void setPrevId(const QString &id);

    static bool isStreamManagementResume(const QDomElement &element);

    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;

private:
    unsigned m_h;
    QString m_previd;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementResumed : public QXmppNonza
{
public:
    explicit QXmppStreamManagementResumed(const unsigned h = 0, const QString &previd = QString());

    unsigned h() const;
    void setH(const unsigned h);

    QString prevId() const;
    void setPrevId(const QString &id);

    static bool isStreamManagementResumed(const QDomElement &element);

    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;

private:
    unsigned m_h;
    QString m_previd;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementFailed : public QXmppNonza
{
public:
    explicit QXmppStreamManagementFailed(const QXmppStanza::Error::Condition error = QXmppStanza::Error::UndefinedCondition);

    QXmppStanza::Error::Condition error() const;
    void setError(const QXmppStanza::Error::Condition error);

    static bool isStreamManagementFailed(const QDomElement &element);

    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;

private:
    QXmppStanza::Error::Condition m_error;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementAck : public QXmppNonza
{
public:
    explicit QXmppStreamManagementAck(const unsigned seqNo = 0);

    unsigned seqNo() const;
    void setSeqNo(const unsigned seqNo);

    static bool isStreamManagementAck(const QDomElement &element);

    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;

private:
    unsigned m_seqNo;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementReq
{
public:
    static bool isStreamManagementReq(const QDomElement &element);

    static void toXml(QXmlStreamWriter *writer);
};

namespace QXmpp::Private {

//
// This manager is used in the QXmppStream. It contains the parts of stream
// management that are shared between server and client connections.
//
class StreamAckManager
{
public:
    explicit StreamAckManager(XmppSocket &socket);
    ~StreamAckManager();

    bool enabled() const;
    unsigned int lastIncomingSequenceNumber() const;

    void handleDisconnect();
    void handleStart();
    void handlePacketSent(QXmppPacket &packet, bool sentData);
    bool handleStanza(const QDomElement &stanza);

    void resetCache();
    void enableStreamManagement(bool resetSequenceNumber);
    void setAcknowledgedSequenceNumber(unsigned int sequenceNumber);

    QXmppTask<QXmpp::SendResult> send(QXmppPacket &&);
    bool sendPacketCompat(QXmppPacket &&);
    std::tuple<bool, QXmppTask<QXmpp::SendResult>> internalSend(QXmppPacket &&);

private:
    void handleAcknowledgement(const QDomElement &element);

    void sendAcknowledgement();
    void sendAcknowledgementRequest();

    QXmpp::Private::XmppSocket &socket;

    bool m_enabled = false;
    QMap<unsigned int, QXmppPacket> m_unacknowledgedStanzas;
    unsigned int m_lastOutgoingSequenceNumber = 0;
    unsigned int m_lastIncomingSequenceNumber = 0;
};

}  // namespace QXmpp::Private
/// \endcond

#endif
