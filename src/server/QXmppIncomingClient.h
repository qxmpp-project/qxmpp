// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPINCOMINGCLIENT_H
#define QXMPPINCOMINGCLIENT_H

#include "QXmppStream.h"

class QXmppIncomingClientPrivate;
class QXmppPasswordChecker;

/// \brief Interface for password checkers.
///

/// \brief The QXmppIncomingClient class represents an incoming XMPP stream
/// from an XMPP client.
///

class QXMPP_EXPORT QXmppIncomingClient : public QXmppStream
{
    Q_OBJECT

public:
    QXmppIncomingClient(QSslSocket *socket, const QString &domain, QObject *parent = nullptr);
    ~QXmppIncomingClient() override;

    bool isConnected() const override;
    QString jid() const;

    void setInactivityTimeout(int secs);
    void setPasswordChecker(QXmppPasswordChecker *checker);

Q_SIGNALS:
    /// This signal is emitted when an element is received.
    void elementReceived(const QDomElement &element);

protected:
    /// \cond
    void handleStream(const QDomElement &element) override;
    void handleStanza(const QDomElement &element) override;
    /// \endcond

private Q_SLOTS:
    void onDigestReply();
    void onPasswordReply();
    void onSocketDisconnected();
    void onTimeout();

private:
    void sendStreamFeatures();

    const std::unique_ptr<QXmppIncomingClientPrivate> d;
    friend class QXmppIncomingClientPrivate;
};

#endif
