/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */


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
class QXmppStreamManagement;


/// \brief The QXmppOutgoingClient class represents an outgoing XMPP stream
/// to an XMPP server.
///

class QXMPP_EXPORT QXmppOutgoingClient : public QXmppStream
{
    Q_OBJECT

public:
    QXmppOutgoingClient(QObject *parent);
    ~QXmppOutgoingClient();

    void connectToHost();
    bool isAuthenticated() const;
    bool isConnected() const;
    bool sendPacket(const QXmppStanza &stanza);
    void sendStreamManagementRequest();

    QSslSocket *socket() const { return QXmppStream::socket(); };
    QXmppStanza::Error::Condition xmppStreamError();

    QXmppConfiguration& configuration();

signals:
    /// This signal is emitted when an error is encountered.
    void error(QXmppClient::Error);

    /// This signal is emitted when an element is received.
    void elementReceived(const QDomElement &element, bool &handled);

    /// This signal is emitted when a presence is received.
    void presenceReceived(const QXmppPresence&);

    /// This signal is emitted when a message is received.
    void messageReceived(const QXmppMessage&);

    /// This signal is emitted when an IQ is received.
    void iqReceived(const QXmppIq&);

    /// This singal is emitted when the server AKC/NACK a message
    void messageAcknowledged(const QXmppMessage&, const bool);

    /// This singal is emitted when the server AKC/NACK a presence
    void presenceAcknowledged(const QXmppPresence&, const bool);

    /// This singal is emitted when the server AKC a iq
    void iqAcknowledged(const QXmppIq&, const bool);

    /// This signal is emitted when an error is ecountered in the stream management
    void streamManagementError(QXmppStanza::Error::Condition);

    /// This signal is emitted when the Stream Management has been enabled
    void streamManagementEnabled(bool resumeEnabled);



protected:
    /// \cond
    // Overridable methods
    virtual void handleStart();
    virtual void handleStanza(const QDomElement &element);
    virtual void handleStream(const QDomElement &element);
    /// \endcond

private slots:
    void _q_dnsLookupFinished();
    void _q_socketDisconnected();
    void socketError(QAbstractSocket::SocketError);
    void socketSslErrors(const QList<QSslError>&);

    void pingStart();
    void pingStop();
    void pingSend();
    void pingTimeout();

private:
    void sendNonSASLAuth(bool plaintext);
    void sendNonSASLAuthQuery();

    // XEP-0198: Stream Management
    void enableStreamManagement();

    void sendStreamManagementEnable(const bool resume);
    void sendStreamManagementAck();

    void handleStreamManagement(const QDomElement &element);

    bool isStreamManagement(const QDomElement &element);



    friend class QXmppOutgoingClientPrivate;
    QXmppOutgoingClientPrivate * const d;

};

#endif // QXMPPOUTGOINGCLIENT_H
