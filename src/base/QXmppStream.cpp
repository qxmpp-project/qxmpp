/*
 * Copyright (C) 2008-2021 The QXmpp developers
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

#include "QXmppStream.h"

#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppIq.h"
#include "QXmppLogger.h"
#include "QXmppPacket_p.h"
#include "QXmppStanza.h"
#include "QXmppStreamManagement_p.h"
#include "QXmppUtils.h"

#include <QBuffer>
#include <QDomDocument>
#include <QFuture>
#include <QFutureInterface>
#include <QFutureWatcher>
#include <QHostAddress>
#include <QMap>
#include <QRegularExpression>
#include <QSslSocket>
#include <QStringList>
#include <QTime>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
static bool randomSeeded = false;
#endif

using IqState = QFutureInterface<QXmppStream::IqResult>;

class QXmppStreamPrivate
{
public:
    QXmppStreamPrivate(QXmppStream *stream);

    QString dataBuffer;
    QSslSocket *socket;

    // incoming stream state
    QString streamOpenElement;

    // stream management
    QXmppStreamManager streamManager;

    // iq response handling
    QMap<QString, IqState> runningIqs;
};

QXmppStreamPrivate::QXmppStreamPrivate(QXmppStream *stream)
    : socket(nullptr),
      streamManager(stream)
{
}

///
/// \typedef QXmppStream::IqResult
///
/// Contains a QDomElement containing the IQ response or if the request couldn't
/// be sent a QXmpp::PacketState.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///

///
/// Constructs a base XMPP stream.
///
/// \param parent
///
QXmppStream::QXmppStream(QObject *parent)
    : QXmppLoggable(parent),
      d(new QXmppStreamPrivate(this))
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    // Make sure the random number generator is seeded
    if (!randomSeeded) {
        qsrand(QTime(0, 0, 0).msecsTo(QTime::currentTime()) ^ reinterpret_cast<quintptr>(this));
        randomSeeded = true;
    }
#endif
}

///
/// Destroys a base XMPP stream.
///
QXmppStream::~QXmppStream()
{
    cancelOngoingIqs();
    delete d;
}

///
/// Disconnects from the remote host.
///
void QXmppStream::disconnectFromHost()
{
    d->streamManager.handleDisconnect();

    if (d->socket) {
        if (d->socket->state() == QAbstractSocket::ConnectedState) {
            sendData(QByteArrayLiteral("</stream:stream>"));
            d->socket->flush();
        }
        // FIXME: according to RFC 6120 section 4.4, we should wait for
        // the incoming stream to end before closing the socket
        d->socket->disconnectFromHost();
    }
}

///
/// Handles a stream start event, which occurs when the underlying transport
/// becomes ready (socket connected, encryption started).
///
/// If you redefine handleStart(), make sure to call the base class's method.
///
void QXmppStream::handleStart()
{
    d->streamManager.handleStart();
    d->dataBuffer.clear();
    d->streamOpenElement.clear();
}

///
/// Returns true if the stream is connected.
///
bool QXmppStream::isConnected() const
{
    return d->socket &&
        d->socket->state() == QAbstractSocket::ConnectedState;
}

///
/// Sends raw data to the peer.
///
/// \param data
///
bool QXmppStream::sendData(const QByteArray &data)
{
    logSent(QString::fromUtf8(data));
    if (!d->socket || d->socket->state() != QAbstractSocket::ConnectedState)
        return false;
    return d->socket->write(data) == data.size();
}

///
/// Sends an XMPP packet to the peer.
///
/// \param stanza
///
bool QXmppStream::sendPacket(const QXmppNonza &stanza)
{
    bool success;
//    send(stanza, success);
    return false;
}

///
/// Sends an XMPP packet to the peer.
///
/// \since QXmpp 1.5
///
QFuture<QXmpp::SendResult> QXmppStream::send(QXmppNonza &&nonza)
{
    bool success;
    return send(std::move(nonza), success);
}

QFuture<QXmpp::SendResult> QXmppStream::send(QXmppNonza &&nonza, bool &writtenToSocket)
{
    QXmppPacket packet(nonza);
    writtenToSocket = sendData(packet.data());

    // handle stream management
    d->streamManager.handlePacketSent(packet, writtenToSocket);

    return packet.future();
}

///
/// Sends an IQ packet and returns the response asynchronously.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
QFuture<QXmppStream::IqResult> QXmppStream::sendIq(QXmppIq &&iq)
{
    using namespace QXmpp;

    if (iq.id().isEmpty()) {
        warning(QStringLiteral("QXmppStream::sendIq() error: ID is empty. Using random ID."));
        iq.setId(QXmppUtils::generateStanzaUuid());
    }
    if (d->runningIqs.contains(iq.id())) {
        warning(QStringLiteral("QXmppStream::sendIq() error:"
                               "The IQ's ID (\"%1\") is already in use. Using random ID.")
                    .arg(iq.id()));
        iq.setId(QXmppUtils::generateStanzaUuid());
    }

    const auto id = iq.id();
    auto sendFuture = send(std::move(iq));
    if (sendFuture.isFinished()) {
        if (std::holds_alternative<SendError>(sendFuture.result())) {
            // early exit (saves QFutureWatcher)
            return makeReadyFuture<IqResult>(std::get<SendError>(sendFuture.result()));
        }
    } else {
        awaitLast(sendFuture, this, [this, id](SendResult result) {
            if (std::holds_alternative<SendError>(result)) {
                if (auto itr = d->runningIqs.find(id); itr != d->runningIqs.end()) {
                    itr.value().reportResult(std::get<SendError>(result));
                    itr.value().reportFinished();

                    d->runningIqs.erase(itr);
                }
            }
        });
    }

    IqState interface(IqState::Started);
    d->runningIqs.insert(iq.id(), interface);
    return interface.future();
}

///
/// Cancels all ongoing IQ requests and reports QXmpp::SendError::Disconnected.
///
/// \since QXmpp 1.5
///
void QXmppStream::cancelOngoingIqs()
{
    for (auto &state : d->runningIqs) {
        state.reportResult(QXmpp::SendError {
            QStringLiteral("IQ has been cancelled."),
            QXmpp::SendError::Disconnected
        });
        state.reportFinished();
    }
    d->runningIqs.clear();
}

///
/// Resets the stream management packages cache.
///
/// This can be done to prevent that packages from the last connection are being
/// resent.
///
/// \since QXmpp 1.4
///
void QXmppStream::resetPacketCache()
{
    d->streamManager.resetCache();
}

///
/// Returns the QSslSocket used for this stream.
///
QSslSocket *QXmppStream::socket() const
{
    return d->socket;
}

///
/// Sets the QSslSocket used for this stream.
///
void QXmppStream::setSocket(QSslSocket *socket)
{
    d->socket = socket;
    if (!d->socket)
        return;

    // socket events
    connect(socket, &QAbstractSocket::connected, this, &QXmppStream::_q_socketConnected);
    connect(socket, &QSslSocket::encrypted, this, &QXmppStream::_q_socketEncrypted);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(socket, &QSslSocket::errorOccurred, this, &QXmppStream::_q_socketError);
#else
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QSslSocket::error), this, &QXmppStream::_q_socketError);
#endif
    connect(socket, &QIODevice::readyRead, this, &QXmppStream::_q_socketReadyRead);
}

void QXmppStream::_q_socketConnected()
{
    info(QStringLiteral("Socket connected to %1 %2").arg(d->socket->peerAddress().toString(), QString::number(d->socket->peerPort())));
    handleStart();
}

void QXmppStream::_q_socketEncrypted()
{
    debug("Socket encrypted");
    handleStart();
}

void QXmppStream::_q_socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    warning(QStringLiteral("Socket error: ") + socket()->errorString());
}

void QXmppStream::_q_socketReadyRead()
{
    processData(QString::fromUtf8(d->socket->readAll()));
}

void QXmppStream::processData(const QString &data)
{
    // As we may only have partial XML content, we need to cache the received
    // data until it has been successfully parsed. In case it can't be parsed,
    //
    // There are only two small problems with the current strategy:
    //  * When we receive a full stanza + a partial one, we can't parse the
    //    first stanza until another stanza arrives that is complete.
    //  * We don't know when we received invalid XML (would cause a growing
    //    cache and a timeout after some time).
    // However, both issues could only be solved using an XML stream reader
    // which would cause many other problems since we don't actually use it for
    // parsing the content.
    d->dataBuffer.append(data);

    //
    // Check for whitespace pings
    //
    if (d->dataBuffer.isEmpty() || d->dataBuffer.trimmed().isEmpty()) {
        d->dataBuffer.clear();

        logReceived({});
        handleStanza({});
        return;
    }

    //
    // Check whether we received a stream open or closing tag
    //
    static const QRegularExpression streamStartRegex(R"(^(<\?xml.*\?>)?\s*<stream:stream[^>]*>)");
    static const QRegularExpression streamEndRegex("</stream:stream>$");

    QRegularExpressionMatch streamOpenMatch;
    bool hasStreamOpen = d->streamOpenElement.isEmpty() &&
        (streamOpenMatch = streamStartRegex.match(d->dataBuffer)).hasMatch();

    bool hasStreamClose = streamEndRegex.match(d->dataBuffer).hasMatch();

    //
    // The stream start/end and stanza packets can't be parsed without any
    // modifications with QDomDocument. This is because of multiple reasons:
    //  * The <stream:stream> open element is not considered valid without the
    //    closing tag.
    //  * Only the closing tag is of course not valid too.
    //  * Stanzas/Nonzas need to have the correct stream namespaces set:
    //     * For being able to parse <stream:features/>
    //     * For having the correct namespace (e.g. 'jabber:client') set to
    //       stanzas and their child elements (e.g. <body/> of a message).
    //
    // The wrapping strategy looks like this:
    //  * The stream open tag is cached once it arrives, for later access
    //  * Incoming XML that has no <stream> open tag will be prepended by the
    //    cached <stream> tag.
    //  * Incoming XML that has no <stream> close tag will be appended by a
    //    generic string "</stream:stream>"
    //
    // The result is parsed by QDomDocument and the child elements of the stream
    // are processed. In case the received data contained a stream open tag,
    // the stream is processed (before the stanzas are processed). In case we
    // received a </stream> closing tag, the connection is closed.
    //
    auto wrappedStanzas = d->dataBuffer;
    if (!hasStreamOpen) {
        wrappedStanzas.prepend(d->streamOpenElement);
    }
    if (!hasStreamClose) {
        wrappedStanzas.append(QStringLiteral("</stream:stream>"));
    }

    //
    // Try to parse the wrapped XML
    //
    QDomDocument doc;
    if (!doc.setContent(wrappedStanzas, true))
        return;

    //
    // Success: We can clear the buffer and send a 'received' log message
    //
    logReceived(d->dataBuffer);
    d->dataBuffer.clear();

    // process stream start
    if (hasStreamOpen) {
        d->streamOpenElement = streamOpenMatch.captured();
        handleStream(doc.documentElement());
    }

    // process stanzas
    auto stanza = doc.documentElement().firstChildElement();
    for (; !stanza.isNull(); stanza = stanza.nextSiblingElement()) {
        // handle possible stream management packets first
        if (d->streamManager.handleStanza(stanza) || handleIqResponse(stanza))
            continue;

        // process all other kinds of packets
        handleStanza(stanza);
    }

    // process stream end
    if (hasStreamClose) {
        disconnectFromHost();
    }
}

bool QXmppStream::handleIqResponse(const QDomElement &stanza)
{
    if (stanza.tagName() != QStringLiteral("iq")) {
        return false;
    }

    // only accept "result" and "error" types
    const auto iqType = stanza.attribute(QStringLiteral("type"));
    if (iqType != QStringLiteral("result") && iqType != QStringLiteral("error")) {
        return false;
    }

    if (auto itr = d->runningIqs.find(stanza.attribute(QStringLiteral("id")));
        itr != d->runningIqs.end()) {

        itr.value().reportResult(stanza);
        itr.value().reportFinished();

        d->runningIqs.erase(itr);
        return true;
    }

    return false;
}

///
/// Enables Stream Management acks / reqs (\xep{0198}).
///
/// \param resetSequenceNumber Indicates if the sequence numbers should be
/// reset. This must be done if the stream is not resumed.
///
/// \since QXmpp 1.0
///
void QXmppStream::enableStreamManagement(bool resetSequenceNumber)
{
    d->streamManager.enableStreamManagement(resetSequenceNumber);
}

///
/// Returns the sequence number of the last incoming stanza (\xep{0198}).
///
/// \since QXmpp 1.0
///
unsigned int QXmppStream::lastIncomingSequenceNumber() const
{
    return d->streamManager.lastIncomingSequenceNumber();
}

///
/// Sets the last acknowledged sequence number for outgoing stanzas
/// (\xep{0198}).
///
/// \since QXmpp 1.0
///
void QXmppStream::setAcknowledgedSequenceNumber(unsigned int sequenceNumber)
{
    d->streamManager.setAcknowledgedSequenceNumber(sequenceNumber);
}
