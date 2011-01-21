/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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


#ifndef QXMPPSTREAM_H
#define QXMPPSTREAM_H

#include <QObject>
#include "QXmppLogger.h"

class QDomElement;
class QSslSocket;
class QXmppPacket;
class QXmppStreamPrivate;

/// \brief The QXmppStream class is the base class for all XMPP streams.
///

class QXmppStream : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppStream(QObject *parent);
    ~QXmppStream();

    virtual bool isConnected() const;
    virtual void disconnectFromHost();

    virtual bool sendData(const QByteArray&);
    bool sendElement(const QDomElement&);
    bool sendPacket(const QXmppPacket&);

signals:
    /// This signal is emitted when the stream is connected.
    void connected();

    /// This signal is emitted when the stream is disconnected.
    void disconnected();

protected:
    // Access to underlying socket
    QSslSocket *socket();
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

private slots:
    void socketConnected();
    void socketDisconnected();
    void socketEncrypted();
    void socketReadyRead();

private:
    QXmppStreamPrivate * const d;
};

#endif // QXMPPSTREAM_H
