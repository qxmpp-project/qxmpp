// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOUTGOINGSERVER_H
#define QXMPPOUTGOINGSERVER_H

#include "QXmppStream.h"

#include <QAbstractSocket>

class QSslError;
class QXmppDialback;
class QXmppOutgoingServer;
class QXmppOutgoingServerPrivate;

/// \brief The QXmppOutgoingServer class represents an outgoing XMPP stream
/// to another XMPP server.
///

class QXMPP_EXPORT QXmppOutgoingServer : public QXmppStream
{
    Q_OBJECT

public:
    QXmppOutgoingServer(const QString &domain, QObject *parent);
    ~QXmppOutgoingServer() override;

    bool isConnected() const override;

    QString localStreamKey() const;
    void setLocalStreamKey(const QString &key);
    void setVerify(const QString &id, const QString &key);

    QString remoteDomain() const;

Q_SIGNALS:
    /// This signal is emitted when a dialback verify response is received.
    void dialbackResponseReceived(const QXmppDialback &response);

protected:
    /// \cond
    void handleStart() override;
    void handleStream(const QDomElement &streamElement) override;
    void handleStanza(const QDomElement &stanzaElement) override;
    /// \endcond

public Q_SLOTS:
    void connectToHost(const QString &domain);
    void queueData(const QByteArray &data);

private Q_SLOTS:
    void _q_dnsLookupFinished();
    void _q_socketDisconnected();
    void sendDialback();
    void slotSslErrors(const QList<QSslError> &errors);
    void socketError(QAbstractSocket::SocketError error);

private:
    Q_DISABLE_COPY(QXmppOutgoingServer)
    QXmppOutgoingServerPrivate *const d;
};

#endif
