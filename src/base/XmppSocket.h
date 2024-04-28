// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef XMPPSOCKET_H
#define XMPPSOCKET_H

#include "QXmppLogger.h"

#include <QDomDocument>
#include <QXmlStreamReader>

class QDomElement;
class QSslSocket;
class TestStream;

namespace QXmpp::Private {

struct StreamOpen;

class SendDataInterface
{
public:
    virtual bool sendData(const QByteArray &) = 0;
};

class DomReader
{
public:
    enum State {
        Finished,
        Unfinished,
        ErrorOccurred,
    };

    State process(QXmlStreamReader &);
    QDomElement element() const { return doc.documentElement(); }

private:
    QDomDocument doc;
    QDomElement currentElement;
    uint depth = 0;
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
    void disconnectFromHost();
    bool sendData(const QByteArray &) override;

    Q_SIGNAL void started();
    Q_SIGNAL void stanzaReceived(const QDomElement &);
    Q_SIGNAL void streamReceived(const QXmpp::Private::StreamOpen &);
    Q_SIGNAL void streamClosed();

private:
    void processData(const QString &data);

    friend class ::TestStream;

    QXmlStreamReader m_reader;
    std::optional<DomReader> m_domReader;
    bool m_streamReceived = false;

    QSslSocket *m_socket = nullptr;

    // incoming stream state
    QString m_streamOpenElement;
};

}  // namespace QXmpp::Private

#endif  // XMPPSOCKET_H
