/*
 * Copyright (C) 2010 Sjors Gielen
 *
 * Author:
 *  Sjors Gielen
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

#include <QSslSocket>

#include "QXmppServer.h"
#include "QXmppServerConnection.h"
#include "QXmppClientServer.h"

#define QXMPPSERVER_DEBUG

#define ASSERT_CONNECT(a, b, c, d) \
      { bool check = connect(a, b, c, d); \
      Q_ASSERT( check ); }

const QByteArray XMPP_STANDARD_SERVER_STREAM_START = "<stream:stream "
  "xmlns=\"jabber:server\" xmlns:stream=\"http://etherx.jabber.org/streams\""
  " version=\"1.0\">";
const QByteArray XMPP_STANDARD_CLIENT_STREAM_START = "<stream:stream "
  "xmlns=\"jabber:client\" xmlns:stream=\"http://etherx.jabber.org/streams\""
  " version=\"1.0\">";
const QByteArray XMPP_STANDARD_POLICY_STREAM_ERROR = "<stream:error "
  "xmlns:stream=\"http://etherx.jabber.org/streams\">"
  "<policy-violation xmlns=\"urn:ietf:params:xml:ns:xmpp-streams\"/>"
  "<text xmlns=\"urn:ietf:params:xml:ns:xmpp-streams'\" xml:lang=\"en\">"
    "Clients of your type are not allowed to connect to this port."
  "</text>"
  "</stream:error>";
const QByteArray XMPP_STANDARD_INVALIDNS_STREAM_ERROR = "<stream:error "
  "xmlns:stream=\"http://etherx.jabber.org/streams\">"
  "<invalid-namespace xmlns=\"urn:ietf:params:xml:ns:xmpp-streams\"/>"
  "</stream:error>";
const QByteArray XMPP_STANDARD_NOTWELLFORMED_STREAM_ERROR = "<stream:error "
  "xmlns:stream=\"http://etherx.jabber.org/streams\">"
  "<invalid-xml xmlns=\"urn:ietf:params:xml:ns:xmpp-streams\"/>"
  "</stream:error>";
const QByteArray XMPP_STANDARD_END_STREAM = "</stream:stream>";

QXmppServer::QXmppServer(bool acceptsServers, bool acceptsClients, QObject *parent)
: QTcpServer(parent)
, m_logger(0)
, m_localCertificate(QByteArray())
, m_privateKey(QSslKey())
, m_acceptsClients(acceptsClients)
, m_acceptsServers(acceptsServers)
{
    m_logger = QXmppLogger::getLogger();
}

QXmppServer::~QXmppServer()
{
    for( int i = 0; i < m_unknownClients.size(); ++i )
    {
        closeSocket( m_unknownClients[i] );
    }
    m_unknownClients.clear();
    Q_ASSERT( m_streamReaders.isEmpty() );
    Q_ASSERT( m_cache.isEmpty() );

    for( int i = 0; i < m_clientConnections.size(); ++i )
    {
        delete m_clientConnections[i];
    }

    for( int i = 0; i < m_serverConnections.size(); ++i )
    {
        delete m_serverConnections[i];
    }
}

bool QXmppServer::acceptsClients() const
{
    return m_acceptsClients;
}

bool QXmppServer::acceptsServers() const
{
    return m_acceptsServers;
}

void QXmppServer::addCaCertificates( const QList<QSslCertificate> &certificates )
{
    m_certificates.append( certificates );
}

void QXmppServer::addCaCertificates( const QSslCertificate &certificate )
{
    m_certificates.append( certificate );
}

void QXmppServer::closeSocket( QSslSocket *socket )
{
    forgetSocket( socket );
    delete socket;
}

/**
 * @brief Determine the socket type of this socket.
 *
 * This method determines whether the connecting peer is a client on the network, or
 * another server. To do this, it analyses the 'xmlns' attribute to the
 * &lt;stream:stream&gt; start tag.
 */
void QXmppServer::determineSocketType()
{
    QSslSocket *socket = static_cast<QSslSocket*>(sender());
    Q_ASSERT( socket != 0 );

    Q_ASSERT( m_unknownClients.contains( socket ) );
    Q_ASSERT( m_streamReaders. contains( socket ) );
    Q_ASSERT( m_cache.         contains( socket ) );
    QXmlStreamReader *reader = m_streamReaders.value( socket );

    // Previous errors should already be handled here
    Q_ASSERT( !reader->hasError() );

    QByteArray data = socket->readAll();
    m_cache[socket].append( data );
    reader->addData( data );

    bool isStartStream = false;
    QXmlStreamNamespaceDeclarations xmlns;
    while( !reader->atEnd() )
    {
      QXmlStreamReader::TokenType type = reader->readNext();
      if( type == QXmlStreamReader::StartElement )
      {
          xmlns = reader->namespaceDeclarations();
          if( reader->qualifiedName() == "stream:stream" )
          {
              // Got the start of a stream!
#ifdef QXMPPSERVER_DEBUG
              qDebug() << "Got the start of a stream.";
#endif
              isStartStream = true;
          }
          else
          {
              qWarning() << "Received another start element than stream:stream"
                         << reader->qualifiedName();
              socket->write( XMPP_STANDARD_SERVER_STREAM_START );
              socket->write( XMPP_STANDARD_NOTWELLFORMED_STREAM_ERROR );
              socket->write( XMPP_STANDARD_END_STREAM );
              socket->flush();
              closeSocket( socket );
              return;
          }
      }
      else if( !isStartStream && type != QXmlStreamReader::StartDocument )
      {
          qWarning() << "Received XML data before start of stream: "
                     << reader->qualifiedName()
                     << "type: " << type
                     << "error: " << reader->errorString();
          if( type == 1 && tryOtherProtocol( socket ) )
          {
              // Apparantly, the socket wasn't speaking XMPP.
              // tryOtherProtocol() has returned another message, now close
              socket->flush();
              closeSocket( socket );
              return;
          }
          else if( reader->hasError() && reader->error() !=
              QXmlStreamReader::PrematureEndOfDocumentError )
          {
              socket->write( XMPP_STANDARD_SERVER_STREAM_START );
              if( reader->qualifiedName() == "stream:stream" )
              {
                  socket->write( XMPP_STANDARD_INVALIDNS_STREAM_ERROR );
              }
              else
              {
                  socket->write( XMPP_STANDARD_NOTWELLFORMED_STREAM_ERROR );
              }
              socket->write( XMPP_STANDARD_END_STREAM );
              socket->flush();
              closeSocket( socket );
              return;
          }
      }
    }

    if( !isStartStream )
    {
        // Didn't find a stream:stream yet, return immediately.
        return;
    }

    bool containsServerXmlns;
    bool containsClientXmlns;
    for(int i = 0; i < xmlns.size(); ++i )
    {
#ifdef QXMPPSERVER_DEBUG
        qDebug() << "XMLNS prefix=" << xmlns.at(i).prefix() << "; namespaceUri="
                 << xmlns.at(i).namespaceUri();
#endif
        if( xmlns.at(i).prefix().isEmpty() )
        {
            if( xmlns.at(i).namespaceUri() == "jabber:client" )
            {
                containsClientXmlns = true;
            }
            else if( xmlns.at(i).namespaceUri() == "jabber:server" )
            {
                containsServerXmlns = true;
            }
            else
            {
                qWarning() << "Didn't find proper XML namespace in stream.";
                socket->write( XMPP_STANDARD_SERVER_STREAM_START );
                socket->write( XMPP_STANDARD_INVALIDNS_STREAM_ERROR );
                socket->write( XMPP_STANDARD_END_STREAM );
                socket->flush();
                closeSocket( socket );
                return;
            }
        }
    }

    Q_ASSERT( containsServerXmlns || containsClientXmlns );

    if( containsServerXmlns )
    {
#ifdef QXMPPSERVER_DEBUG
        qDebug() << "It's a server socket!";
#endif

        if( !m_acceptsServers )
        {
            // send a stream error
            socket->write( XMPP_STANDARD_SERVER_STREAM_START );
            socket->write( XMPP_STANDARD_POLICY_STREAM_ERROR );
            socket->write( XMPP_STANDARD_END_STREAM );
            socket->flush();
            closeSocket( socket );
            return;
        }

        QByteArray pastData = m_cache[socket];
        forgetSocket( socket );

        socket->addCaCertificates( m_certificates );
        socket->setLocalCertificate( m_localCertificate );
        socket->setPrivateKey( m_privateKey );
        socket->setProtocol( QSsl::AnyProtocol );

        QXmppServerConnection *sc = new QXmppServerConnection( socket, pastData );
        sc->setLogger( m_logger );
        m_serverConnections.append( sc );
    }
    else if( containsClientXmlns )
    {
#ifdef QXMPPSERVER_DEBUG
        qDebug() << "It's a client socket!";
#endif

        if( !m_acceptsClients )
        {
            // send a stream error
            socket->write( XMPP_STANDARD_CLIENT_STREAM_START );
            socket->write( XMPP_STANDARD_POLICY_STREAM_ERROR );
            socket->write( XMPP_STANDARD_END_STREAM );
            socket->flush();
            closeSocket( socket );
            return;
        }

        QByteArray pastData = m_cache[socket];
        forgetSocket( socket );

        QXmppClientServer *cs = new QXmppClientServer( socket, pastData );
        cs->setLogger( m_logger );
        m_clientConnections.append( cs );
    }
}

void QXmppServer::forgetSocket( QSslSocket *socket )
{
    Q_ASSERT( m_unknownClients.contains( socket ) );
    Q_ASSERT( m_streamReaders. contains( socket ) );
    Q_ASSERT( m_cache.         contains( socket ) );

    socket->disconnect( this, 0 );

    QXmlStreamReader *reader = m_streamReaders.value( socket );
    m_unknownClients.removeAll( socket );
    m_streamReaders.remove( socket );
    m_cache.remove( socket );
    delete reader;
}

void QXmppServer::incomingConnection( int socketDescriptor )
{
    QSslSocket *socket = new QSslSocket;
    if( socket->setSocketDescriptor( socketDescriptor ) )
    {
        ASSERT_CONNECT( socket, SIGNAL(           readyRead() ),
                        this,   SLOT(   determineSocketType() ) );

        m_unknownClients.append( socket );
        m_streamReaders.insert( socket, new QXmlStreamReader() );
        m_cache.insert( socket, "" );
        emit unknownClientConnected( socket );
        return;
    }

    delete socket;
    qWarning() << "setSocketDescriptor failed";
}

QXmppLogger *QXmppServer::logger()
{
    return m_logger;
}

void QXmppServer::setAcceptsClients( bool acceptsClients )
{
    m_acceptsClients = acceptsClients;
}

void QXmppServer::setAcceptsServers( bool acceptsServers )
{
    m_acceptsServers = acceptsServers;
}

void QXmppServer::setLocalCertificate( const QSslCertificate &localCertificate )
{
    m_localCertificate = localCertificate;
}

void QXmppServer::setLogger(QXmppLogger *logger)
{
    m_logger = logger;
}

void QXmppServer::setPrivateKey( const QSslKey &privateKey )
{
    m_privateKey = privateKey;
}

/**
 * @brief Try to detect another protocol on our server line
 *
 * Sometimes, a client erroneously connects to the wrong port. For example, a
 * browser or IRC client might end up on our socket. The XML parser will
 * likely give an error then - here we check if the other side is speaking the
 * incorrect protocol. This will, of course, never work in protocols where the
 * server starts sending and the client waits indefinitely.
 *
 * This method should only write a message to the socket and return true if
 * a protocol was recognised; the socket will be flushed and closed elsewhere.
 *
 * Recognised protocols: http (first line ends with HTTP/x.y), irc (first line
 * starts with NICK)
 */
bool QXmppServer::tryOtherProtocol( QSslSocket *socket )
{
    Q_ASSERT( m_cache.         contains( socket ) );
    QByteArray &cache = m_cache[socket];

    int newline = cache.indexOf( "\n" );
    int carrret = cache.indexOf( "\r" );
    newline = ( carrret < newline && carrret != -1 ) ? carrret : newline;
    QByteArray firstLine = cache.mid( 0, newline );

    // Discover IRC
    if( firstLine.startsWith( "NICK " ) )
    {
        QByteArray nick = firstLine.mid( 5, newline - 5 );
        socket->write( ":xmppd 001 " + nick + " :Welcome to XMPP IRC\r\n" );
        socket->write( ":xmppd 002 " + nick + " :Your host is an XMPP IRC daemon\r\n" );
        socket->write( ":xmppd 375 " + nick + " :- XMPP IRC Daemon Message of the Day -\r\n" );
        socket->write( ":xmppd 372 " + nick + " :- This server is not an IRC server! It is\r\n" );
        socket->write( ":xmppd 372 " + nick + " :- an XMPP server. Please use an XMPP client\r\n" );
        socket->write( ":xmppd 372 " + nick + " :- to connect to it.\r\n" );
        socket->write( ":xmppd 376 " + nick + " :End of /MOTD command.\r\n" );
        socket->write( ":" + nick + " QUIT :Not an XMPP client.\r\n" );
        socket->write( "ERROR :Closing Link: Not an XMPP client.\r\n" );
        return true;
    }

    // Discover HTTP
    QByteArray httpVer = firstLine.right(8);
    if( httpVer.startsWith( "HTTP/" ) )
    {
        socket->write( "HTTP/1.0 200 OK\r\n" );
        socket->write( "Content-Type: text/html; charset=UTF-8\r\n" );
        socket->write( "Server: xmppd\r\n" );
        socket->write( "\r\n" );
        socket->write( "<html><head><title>Not a webserver</title></head>\r\n" );
        socket->write( "<body>You are connecting to an XMPP server as if it were" );
        socket->write( " a webserver. Please use an XMPP client to connect to " );
        socket->write( "it.</body></html>\r\n" );
        return true;
    }

    return false;
}
