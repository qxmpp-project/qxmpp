// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef XMPPSOCKET_H
#define XMPPSOCKET_H

#include "QXmppLogger.h"

class QDomElement;
class QSslSocket;
class TestStream;
class tst_QXmppStream;

namespace QXmpp::Private {

struct ServerAddress {
    enum ConnectionType {
        Tcp,
        Tls,
    };

    ConnectionType type;
    QString host;
    quint16 port;
};

class SendDataInterface
{
public:
    virtual bool sendData(const QByteArray &) = 0;
};

class QXMPP_EXPORT XmppSocket : public QXmppLoggable, public SendDataInterface
{
    Q_OBJECT
public:
    XmppSocket(QObject *parent);
    ~XmppSocket() override = default;

    QSslSocket *socket() const { return m_socket; }
    void setSocket(QSslSocket *socket);

    bool isConnected() const;
    void connectToHost(const ServerAddress &);
    void disconnectFromHost();
    bool sendData(const QByteArray &) override;

    Q_SIGNAL void started();
    Q_SIGNAL void stanzaReceived(const QDomElement &);
    Q_SIGNAL void streamReceived(const QDomElement &);
    Q_SIGNAL void streamClosed();

private:
    void processData(const QString &data);

    friend class ::tst_QXmppStream;

    QString m_dataBuffer;
    bool m_directTls = false;
    QSslSocket *m_socket = nullptr;

    // incoming stream state
    QString m_streamOpenElement;
};

}  // namespace QXmpp::Private

#endif  // XMPPSOCKET_H
