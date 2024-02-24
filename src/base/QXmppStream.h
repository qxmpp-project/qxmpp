// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSTREAM_H
#define QXMPPSTREAM_H

#include "QXmppLogger.h"
#include "QXmppSendResult.h"

#include <memory>
#include <unordered_map>
#include <variant>

#include <QAbstractSocket>
#include <QObject>

class QDomElement;
template<typename T>
class QXmppTask;
template<typename T>
class QFuture;
template<typename T>
class QFutureInterface;
class QSslSocket;
class QXmppIq;
class QXmppNonza;
class QXmppPacket;
class QXmppStanza;
class QXmppStreamManager;
class QXmppStreamPrivate;
class TestStream;

namespace QXmpp::Private {

class XmppSocket;
class StreamAckManager;
class OutgoingIqManager;
struct IqState;

}  // namespace QXmpp::Private

///
/// \brief The QXmppStream class is the base class for all XMPP streams.
///
class QXMPP_EXPORT QXmppStream : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppStream(QObject *parent);
    ~QXmppStream() override;

    virtual bool isConnected() const;

    bool sendPacket(const QXmppNonza &);

    QXmpp::Private::XmppSocket &xmppSocket() const;
    QXmpp::Private::StreamAckManager &streamAckManager() const;
    QXmpp::Private::OutgoingIqManager &iqManager() const;

Q_SIGNALS:
    /// This signal is emitted when the stream is connected.
    void connected();

    /// This signal is emitted when the stream is disconnected.
    void disconnected();

protected:
    // Access to underlying socket
    QSslSocket *socket() const;
    void setSocket(QSslSocket *socket);

    // Overridable methods
    virtual void handleStart();

    /// Handles an incoming XMPP stanza.
    ///
    /// \param element
    virtual void handleStanza(const QDomElement &element) = 0;

    /// Handles an incoming XMPP stream start.
    ///
    /// \param element
    virtual void handleStream(const QDomElement &element) = 0;

public Q_SLOTS:
    virtual void disconnectFromHost();
    virtual bool sendData(const QByteArray &);

private:
    friend class QXmpp::Private::XmppSocket;
    friend class tst_QXmppStream;
    friend class TestClient;

    void onStanzaReceived(const QDomElement &);

    // for unit tests, see TestClient
    void enableStreamManagement(bool resetSequenceNumber);
    bool handleIqResponse(const QDomElement &);

    const std::unique_ptr<QXmppStreamPrivate> d;
};

namespace QXmpp::Private {

using IqResult = std::variant<QDomElement, QXmppError>;

class OutgoingIqManager
{
public:
    OutgoingIqManager(QXmppLoggable *l, StreamAckManager &streamAckMananger);

    QXmppTask<IqResult> sendIq(QXmppIq &&, const QString &to);
    QXmppTask<IqResult> sendIq(QXmppPacket &&, const QString &id, const QString &to);

    bool hasId(const QString &id) const;
    bool isIdValid(const QString &id) const;

    QXmppTask<IqResult> start(const QString &id, const QString &to);
    void finish(const QString &id, IqResult &&result);
    void cancelAll();

    bool handleStanza(const QDomElement &stanza);

private:
    void warning(const QString &message);

    QXmppLoggable *l;
    StreamAckManager &m_streamAckManager;
    std::unordered_map<QString, IqState> m_requests;
};

class QXMPP_EXPORT XmppSocket
{
public:
    XmppSocket(QXmppStream *q);

    QSslSocket *socket() const { return m_socket; }
    void setSocket(QSslSocket *socket);

    bool isConnected() const;
    void disconnectFromHost();
    bool sendData(const QByteArray &);

private:
    void processData(const QString &data);

    friend class ::TestStream;

    QXmppStream *q;

    QString m_dataBuffer;
    QSslSocket *m_socket = nullptr;

    // incoming stream state
    QString m_streamOpenElement;
};

}  // namespace QXmpp::Private

#endif  // QXMPPSTREAM_H
