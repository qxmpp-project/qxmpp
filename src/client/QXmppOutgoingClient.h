// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOUTGOINGCLIENT_H
#define QXMPPOUTGOINGCLIENT_H

#include "QXmppClient.h"
#include "QXmppStanza.h"
#include "QXmppStream.h"

class QDomElement;
class QSslError;

class QXmppConfiguration;
class QXmppPresence;
class QXmppIq;
class QXmppMessage;

class QXmppOutgoingClientPrivate;

/// \brief The QXmppOutgoingClient class represents an outgoing XMPP stream
/// to an XMPP server.
///

class QXMPP_EXPORT QXmppOutgoingClient : public QXmppStream
{
    Q_OBJECT

public:
    QXmppOutgoingClient(QObject *parent);
    ~QXmppOutgoingClient() override;

    void connectToHost();
    bool isAuthenticated() const;
    bool isConnected() const override;
    bool isClientStateIndicationEnabled() const;
    bool isStreamManagementEnabled() const;
    bool isStreamResumed() const;
    QXmppTask<IqResult> sendIq(QXmppIq &&);

    /// Returns the used socket
    QSslSocket *socket() const { return QXmppStream::socket(); };
    QXmppStanza::Error::Condition xmppStreamError();

    QXmppConfiguration &configuration();

Q_SIGNALS:
    /// This signal is emitted when an error is encountered.
    void error(QXmppClient::Error);

    /// This signal is emitted when an element is received.
    void elementReceived(const QDomElement &element, bool &handled);

    /// This signal is emitted when a presence is received.
    void presenceReceived(const QXmppPresence &);

    /// This signal is emitted when a message is received.
    void messageReceived(const QXmppMessage &);

    /// This signal is emitted when an IQ response (type result or error) has
    /// been received that was not handled by elementReceived().
    void iqReceived(const QXmppIq &);

    /// This signal is emitted when SSL errors are encountered.
    void sslErrors(const QList<QSslError> &errors);

protected:
    /// \cond
    // Overridable methods
    void handleStart() override;
    void handleStanza(const QDomElement &element) override;
    void handleStream(const QDomElement &element) override;
    /// \endcond

public Q_SLOTS:
    void disconnectFromHost() override;

private Q_SLOTS:
    void _q_dnsLookupFinished();
    void _q_socketDisconnected();
    void socketError(QAbstractSocket::SocketError);
    void socketSslErrors(const QList<QSslError> &);

    void pingStart();
    void pingStop();
    void pingSend();
    void pingTimeout();

private:
    bool setResumeAddress(const QString &address);
    static std::pair<QString, int> parseHostAddress(const QString &address);

    friend class QXmppOutgoingClientPrivate;
    friend class tst_QXmppOutgoingClient;

    const std::unique_ptr<QXmppOutgoingClientPrivate> d;
};

#endif  // QXMPPOUTGOINGCLIENT_H
