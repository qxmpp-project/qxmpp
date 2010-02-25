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

#include <QTcpServer>
#include <QTcpSocket>

#ifdef Q_OS_WIN
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

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

QByteArray encodeHostAndPort(quint8 type, const QByteArray &host, quint16 port)
{
    QByteArray buffer;
    buffer.resize(2);
    // set host name
    buffer[0] = type;
    buffer[1] = host.size();
    buffer.append(host);
    // set port
    int pos = buffer.size();
    buffer.resize(pos + 2);
    quint16 p = htons(port);
    memcpy(buffer.data() + pos, &p, 2);
    return buffer;
}

bool parseHostAndPort(const QByteArray buffer, quint8 &type, QByteArray &host, quint16 &port)
{
    if (buffer.size() < 4)
        return false;

    // parse host type
    int pos = 0;
    type = buffer.at(pos);
    pos++;

    // parse host name
    quint8 hostLength = buffer.at(pos);
    pos++;
    if (buffer.size() < hostLength + 4)
    {
        qWarning("Invalid host length");
        return false;
    }
    host = buffer.mid(pos, hostLength);
    pos += hostLength;

    // parse host port
    quint16 p;
    memcpy(&p, buffer.data() + pos, 2);
    port = ntohs(p);
    return true;
}

QXmppSocksClient::QXmppSocksClient(const QHostAddress &proxyAddress, quint16 proxyPort, QObject *parent)
    : QObject(parent), m_proxyAddress(proxyAddress), m_proxyPort(proxyPort)
{
    m_socket = new QTcpSocket(this);
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

bool QXmppSocksClient::waitForConnected(int msecs)
{
    if (!m_socket->waitForConnected(msecs))
        return false;

    // send connect to server
    QByteArray buffer;
    buffer.resize(3);
    buffer[0] = SocksVersion;
    buffer[1] = 0x01; // number of methods
    buffer[2] = NoAuthentication;
    m_socket->write(buffer);

    // wait for connect to server response
    if (!m_socket->waitForReadyRead(msecs))
        return false;
    buffer = m_socket->readAll();
    if (buffer.size() != 2 || buffer.at(0) != SocksVersion || buffer.at(1) != NoAuthentication)
    {
        qWarning("QXmppSocksClient received an invalid response during handshake");
        return false;
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

    // wait for CONNECT response
    if (!m_socket->waitForReadyRead(msecs))
        return false;
    buffer = m_socket->readAll();
    if (buffer.size() < 6 ||
        buffer.at(0) != SocksVersion ||
        buffer.at(1) != Succeeded ||
        buffer.at(2) != 0)
    {
        qWarning("QXmppSocksClient received an invalid response to CONNECT command");
        return false;
    }

    // parse host
    quint8 hostType;
    QByteArray hostName;
    quint16 hostPort;
    if (!parseHostAndPort(buffer.mid(3), hostType, hostName, hostPort))
    {
        qWarning("QXmppSocksClient could not parse type/host/port");
        return false;
    }
    // FIXME : what do we do with the resulting name / port?

    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(m_socket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
    return true;
}

QXmppSocksServer::QXmppSocksServer(QObject *parent)
    : QObject(parent),
    m_hostPort(0),
    m_socket(0)
{
    m_server = new QTcpServer(this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
}

void QXmppSocksServer::close()
{
    m_server->close();
    if (m_socket)
        m_socket->close();
}

bool QXmppSocksServer::listen(const QHostAddress &address, quint16 port)
{
    return m_server->listen(address, port);
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
    m_socket = m_server->nextPendingConnection();
    if (!m_socket)
        return;

    // wait for connect to server
    if (!m_socket->waitForReadyRead())
        return;
    QByteArray buffer = m_socket->readAll();
    if (buffer.size() < 3 ||
        buffer.at(0) != SocksVersion ||
        buffer.at(1) != 0x01 ||
        buffer.at(2) != NoAuthentication)
    {
        qWarning("QXmppSocksServer received invalid handshake");
        m_socket->close();
        return;
    }

    // send connect to server response
    buffer.resize(2);
    buffer[0] = SocksVersion;
    buffer[1] = NoAuthentication;
    m_socket->write(buffer);

    // wait for connect command
    if (!m_socket->waitForReadyRead())
        return;
    buffer = m_socket->readAll();
    if (buffer.size() < 4 ||
        buffer.at(0) != SocksVersion ||
        buffer.at(1) != ConnectCommand ||
        buffer.at(2) != 0x00)
    {
        qWarning("QXmppSocksServer received an invalid command");
        m_socket->close();
        return;
    }

    // parse host
    quint8 hostType;
    QByteArray hostName;
    quint16 hostPort;
    if (!parseHostAndPort(buffer.mid(3), hostType, hostName, hostPort))
    {
        qWarning("QXmppSocksServer could not parse type/host/port");
        m_socket->close();
        return;
    }
    if (hostName != m_hostName || hostPort != m_hostPort)
    {
        qWarning("QXmppSocksServer got wrong host or port");
        m_socket->close();
        return;
    }

    //send connect response
    buffer.resize(3);
    buffer[0] = SocksVersion;
    buffer[1] = Succeeded;
    buffer[2] = 0x00;
    buffer.append(encodeHostAndPort(
        DomainName,
        m_server->serverAddress().toString().toAscii(),
        m_server->serverPort()));
    m_socket->write(buffer);

    // connect signals
    m_server->close();
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SIGNAL(bytesWritten(qint64)));
}

void QXmppSocksServer::setHostName(const QString &hostName)
{
    m_hostName = hostName;
}

void QXmppSocksServer::setHostPort(quint16 hostPort)
{
    m_hostPort = hostPort;
}

void QXmppSocksServer::write(const QByteArray &data)
{
    if (m_socket)
        m_socket->write(data);
}
