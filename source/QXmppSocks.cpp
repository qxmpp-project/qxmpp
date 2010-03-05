/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#include <QDataStream>
#include <QEventLoop>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "QXmppSocks.h"

const static char SocksVersion = 5;

enum AuthenticationMethod {
    NoAuthentication = 0,
    GSSAPI = 1,
    UsernamePassword = 2,
};

enum Command {
    ConnectCommand = 1,
    BindCommand = 2,
    AssociateCommand = 3,
};

enum AddressType {
    IPv4Address = 1,
    DomainName = 3,
    IPv6Address = 4,
};

enum ReplyType {
    Succeeded = 0,
    SocksFailure = 1,
    ConnectionNotAllowed = 2,
    NetworkUnreachable = 3,
    HostUnreachable = 4,
    ConnectionRefused = 5,
    TtlExpired = 6,
    CommandNotSupported = 7,
    AddressTypeNotSupported = 8,
};

enum State {
    ConnectState = 0,
    CommandState = 1,
    ReadyState = 2,
};

static QByteArray encodeHostAndPort(quint8 type, const QByteArray &host, quint16 port)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    // set host name
    quint8 hostLength = host.size();
    stream << type;
    stream << hostLength;
    stream.writeRawData(host.constData(), hostLength);
    // set port
    stream << port;
    return buffer;
}

static bool parseHostAndPort(const QByteArray buffer, quint8 &type, QByteArray &host, quint16 &port)
{
    if (buffer.size() < 4)
        return false;

    QDataStream stream(buffer);
    // get host name
    quint8 hostLength;
    stream >> type;
    stream >> hostLength;
    if (buffer.size() < hostLength + 4)
    {
        qWarning("Invalid host length");
        return false;
    }
    host.resize(hostLength);
    stream.readRawData(host.data(), hostLength);
    // get port
    stream >> port;
    return true;
}

QXmppSocksClient::QXmppSocksClient(const QHostAddress &proxyAddress, quint16 proxyPort, QObject *parent)
    : QObject(parent),
    m_proxyAddress(proxyAddress),
    m_proxyPort(proxyPort),
    m_step(ConnectState)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(m_socket, SIGNAL(connected()), this, SLOT(slotConnected()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
}

void QXmppSocksClient::close()
{
    m_socket->close();
}

void QXmppSocksClient::connectToHost(const QString &hostName, quint16 hostPort)
{
    m_hostName = hostName;
    m_hostPort = hostPort;
    m_socket->connectToHost(m_proxyAddress, m_proxyPort);
}

QString QXmppSocksClient::errorString() const
{
    return m_socket->errorString();
}

QByteArray QXmppSocksClient::readAll()
{
    return m_socket->readAll();
}

void QXmppSocksClient::slotConnected()
{
    // send connect to server
    QByteArray buffer;
    buffer.resize(3);
    buffer[0] = SocksVersion;
    buffer[1] = 0x01; // number of methods
    buffer[2] = NoAuthentication;
    m_socket->write(buffer);
}

void QXmppSocksClient::slotReadyRead()
{
    if (m_step == ConnectState)
    {
        m_step++;

        // receive connect to server response
        QByteArray buffer = m_socket->readAll();
        if (buffer.size() != 2 || buffer.at(0) != SocksVersion || buffer.at(1) != NoAuthentication)
        {
            qWarning("QXmppSocksClient received an invalid response during handshake");
            m_socket->close();
            return;
        }

        // send CONNECT command
        buffer.resize(3);
        buffer[0] = SocksVersion;
        buffer[1] = ConnectCommand;
        buffer[2] = 0x00; // reserved
        buffer.append(encodeHostAndPort(
            DomainName,
            m_hostName.toAscii(),
            m_hostPort));
        m_socket->write(buffer);

    } else if (m_step == CommandState) {
        m_step++;

        // receive CONNECT response
        QByteArray buffer = m_socket->readAll();
        if (buffer.size() < 6 ||
            buffer.at(0) != SocksVersion ||
            buffer.at(1) != Succeeded ||
            buffer.at(2) != 0)
        {
            qWarning("QXmppSocksClient received an invalid response to CONNECT command");
            m_socket->close();
            return;
        }

        // parse host
        quint8 hostType;
        QByteArray hostName;
        quint16 hostPort;
        if (!parseHostAndPort(buffer.mid(3), hostType, hostName, hostPort))
        {
            qWarning("QXmppSocksClient could not parse type/host/port");
            m_socket->close();
            return;
        }
        // FIXME : what do we do with the resulting name / port?

        // from now on, forward signals
        disconnect(m_socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        connect(m_socket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));

        // notify of connection
        emit connected();

    }
}

bool QXmppSocksClient::waitForConnected(int msecs)
{
    QEventLoop loop;
    connect(this, SIGNAL(connected()), &loop, SLOT(quit()));
    connect(this, SIGNAL(disconnected()), &loop, SLOT(quit()));
    QTimer::singleShot(msecs, &loop, SLOT(quit()));
    loop.exec();

    if (m_step == ReadyState && m_socket->isValid())
        return true;
    else
        return false;
}

qint64 QXmppSocksClient::write(const QByteArray &data)
{
    return m_socket->write(data);
}

QTcpSocket *QXmppSocksClient::socket()
{
    return m_socket;
}

QXmppSocksServer::QXmppSocksServer(QObject *parent)
    : QObject(parent)
{
    m_server = new QTcpServer(this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
}

void QXmppSocksServer::close()
{
    m_server->close();
}

bool QXmppSocksServer::listen(const QHostAddress &address, quint16 port)
{
    return m_server->listen(address, port);
}

bool QXmppSocksServer::isListening() const
{
    return m_server->isListening();
}

QHostAddress QXmppSocksServer::serverAddress() const
{
    return m_server->serverAddress();
}

quint16 QXmppSocksServer::serverPort() const
{
    return m_server->serverPort();
}

void QXmppSocksServer::slotNewConnection()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    if (!socket)
        return;

    // register socket
    m_states.insert(socket, ConnectState);
    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
}

void QXmppSocksServer::slotReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket || !m_states.contains(socket))
        return;

    if (m_states.value(socket) == ConnectState)
    {
        m_states.insert(socket, CommandState);

        // receive connect to server request
        QByteArray buffer = socket->readAll();
        if (buffer.size() < 3 ||
            buffer.at(0) != SocksVersion ||
            buffer.at(1) + 2 != buffer.size())
        {
            qWarning("QXmppSocksServer received invalid handshake");
            socket->close();
            return;
        }

        // check authentication method
        bool foundMethod = false;
        for (int i = 2; i < buffer.size(); i++)
        {
            if (buffer.at(i) == NoAuthentication)
            {
                foundMethod = true;
                break;
            }
        }
        if (!foundMethod)
        {
            qWarning("QXmppSocksServer received bad authentication method");
            socket->close();
            return;
        }

        // send connect to server response
        buffer.resize(2);
        buffer[0] = SocksVersion;
        buffer[1] = NoAuthentication;
        socket->write(buffer);

    } else if (m_states.value(socket) == CommandState) {
        m_states.insert(socket, ReadyState);

        // disconnect from signals
        disconnect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));

        // receive command
        QByteArray buffer = socket->readAll();
        if (buffer.size() < 4 ||
            buffer.at(0) != SocksVersion ||
            buffer.at(1) != ConnectCommand ||
            buffer.at(2) != 0x00)
        {
            qWarning("QXmppSocksServer received an invalid command");
            socket->close();
            return;
        }

        // parse host
        quint8 hostType;
        QByteArray hostName;
        quint16 hostPort;
        if (!parseHostAndPort(buffer.mid(3), hostType, hostName, hostPort))
        {
            qWarning("QXmppSocksServer could not parse type/host/port");
            socket->close();
            return;
        }

        // notify of connection
        emit newConnection(socket, hostName, hostPort);

        // send response
        buffer.resize(3);
        buffer[0] = SocksVersion;
        buffer[1] = Succeeded;
        buffer[2] = 0x00;
        buffer.append(encodeHostAndPort(
            DomainName,
            hostName,
            hostPort));
        socket->write(buffer);
    }
}

