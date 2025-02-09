// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPINCOMINGCLIENT_H
#define QXMPPINCOMINGCLIENT_H

#include "QXmppLogger.h"

#include <memory>

class QDomElement;
class QSslSocket;
class QXmppNonza;
class QXmppIncomingClientPrivate;
class QXmppPasswordChecker;

namespace QXmpp::Private {
struct StreamOpen;
}

///
/// \brief The QXmppIncomingClient class represents an incoming XMPP stream
/// from an XMPP client.
///
class QXMPP_EXPORT QXmppIncomingClient : public QXmppLoggable
{
    Q_OBJECT
public:
    QXmppIncomingClient(QSslSocket *socket, const QString &domain, QObject *parent = nullptr);
    ~QXmppIncomingClient() override;

    bool isConnected() const;
    QString jid() const;

    bool sendPacket(const QXmppNonza &);
    Q_SLOT bool sendData(const QByteArray &);
    void disconnectFromHost();

    void setInactivityTimeout(int secs);
    void setPasswordChecker(QXmppPasswordChecker *checker);

    /// This signal is emitted when an element is received.
    Q_SIGNAL void elementReceived(const QDomElement &element);

    /// This signal is emitted when the stream is connected.
    Q_SIGNAL void connected();

    /// This signal is emitted when the stream is disconnected.
    Q_SIGNAL void disconnected();

protected:
    /// \cond
    void handleStart();
    void handleStream(const QXmpp::Private::StreamOpen &);
    void handleStanza(const QDomElement &element);
    /// \endcond

private Q_SLOTS:
    void onDigestReply();
    void onPasswordReply();
    void onSocketDisconnected();
    void onTimeout();

private:
    void onSasl2Authenticated();
    void sendStreamFeatures();

    const std::unique_ptr<QXmppIncomingClientPrivate> d;
    friend class QXmppIncomingClientPrivate;
};

#endif
