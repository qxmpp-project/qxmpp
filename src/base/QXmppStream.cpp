// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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

struct IqState
{
    QXmppPromise<QXmppStream::IqResult> interface;
    QString jid;
};

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
    if (!d->socket || d->socket->state() != QAbstractSocket::ConnectedState) {
        return false;
    }
    return d->socket->write(data) == data.size();
}

///
/// Sends an XMPP packet to the peer.
///
/// \param nonza
///
bool QXmppStream::sendPacket(const QXmppNonza &nonza)
{
    bool success;
    send(nonza, success);
    return success;
}

///
/// Sends an XMPP packet to the peer.
///
/// \since QXmpp 1.5
///
QXmppTask<QXmpp::SendResult> QXmppStream::send(QXmppNonza &&nonza)
{
    bool success;
    return send(QXmppPacket(nonza), success);
}

///
/// Sends an XMPP packet to the peer.
///
/// \since QXmpp 1.5
///
QXmppTask<QXmpp::SendResult> QXmppStream::send(QXmppPacket &&packet)
{
    bool success;
    return send(std::move(packet), success);
}

QXmppTask<QXmpp::SendResult> QXmppStream::send(QXmppPacket &&packet, bool &writtenToSocket)
{
    // the writtenToSocket parameter is just for backwards compat (see
    // QXmppStream::sendPacket())
    writtenToSocket = sendData(packet.data());

    // handle stream management
    d->streamManager.handlePacketSent(packet, writtenToSocket);

    return packet.task();
}

///
/// Sends an IQ packet and returns the response asynchronously.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppStream::IqResult> QXmppStream::sendIq(QXmppIq &&iq, const QString &to)
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

    return sendIq(QXmppPacket(iq), iq.id(), to);
}

///
/// Sends an IQ packet and returns the response asynchronously.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppStream::IqResult> QXmppStream::sendIq(QXmppPacket &&packet, const QString &id, const QString &to)
{
    using namespace QXmpp;

    if (id.isEmpty() || d->runningIqs.contains(id)) {
        return makeReadyTask<IqResult>(QXmppError {
            QStringLiteral("Invalid IQ id: empty or in use."),
            SendError::Disconnected });
    }

    if (to.isEmpty()) {
        return makeReadyTask<IqResult>(QXmppError {
            QStringLiteral("The 'to' address must be set so the stream can match the response."),
            SendError::Disconnected });
    }

    auto sendFuture = send(std::move(packet));
    if (sendFuture.isFinished()) {
        if (std::holds_alternative<QXmppError>(sendFuture.result())) {
            // early exit
            return makeReadyTask<IqResult>(std::get<QXmppError>(sendFuture.takeResult()));
        }
    } else {
        sendFuture.then(this, [this, id](SendResult result) {
            if (std::holds_alternative<QXmppError>(result)) {
                if (auto itr = d->runningIqs.find(id); itr != d->runningIqs.end()) {
                    itr.value().interface.finish(std::get<QXmppError>(result));
                    d->runningIqs.erase(itr);
                }
            }
        });
    }

    IqState state { {}, to };
    auto task = state.interface.task();
    d->runningIqs.insert(id, std::move(state));
    return task;
}

///
/// Cancels all ongoing IQ requests and reports QXmpp::SendError::Disconnected.
///
/// \since QXmpp 1.5
///
void QXmppStream::cancelOngoingIqs()
{
    for (auto &state : d->runningIqs) {
        state.interface.finish(QXmppError {
            QStringLiteral("IQ has been cancelled."),
            QXmpp::SendError::Disconnected });
    }
    d->runningIqs.clear();
}

///
/// Returns whether the IQ ID is currently in use.
///
/// \since QXmpp 1.5
///
bool QXmppStream::hasIqId(const QString &id) const
{
    return d->runningIqs.contains(id);
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
    if (!d->socket) {
        return;
    }

    // socket events
    connect(socket, &QAbstractSocket::connected, this, &QXmppStream::_q_socketConnected);
    connect(socket, &QSslSocket::encrypted, this, &QXmppStream::_q_socketEncrypted);
    connect(socket, &QSslSocket::errorOccurred, this, &QXmppStream::_q_socketError);
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
    if (!doc.setContent(wrappedStanzas, true)) {
        return;
    }

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
        if (d->streamManager.handleStanza(stanza) || handleIqResponse(stanza)) {
            continue;
        }

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

    const auto id = stanza.attribute(QStringLiteral("id"));
    if (auto itr = d->runningIqs.find(id);
        itr != d->runningIqs.end()) {
        const auto expectedFrom = itr.value().jid;
        // Check that the sender of the response matches the recipient of the request.
        // Stanzas coming from the server on behalf of the user's account must have no "from"
        // attribute or have it set to the user's bare JID.
        // If 'from' is empty, the IQ has been sent by the server. In this case we don't need to
        // do the check as we trust the server anyways.
        if (const auto from = stanza.attribute("from"); !from.isEmpty() && from != expectedFrom) {
            warning(QStringLiteral("Ignored received IQ response to request '%1' because of wrong sender '%2' instead of expected sender '%3'")
                        .arg(id, from, expectedFrom));
            return false;
        }

        itr.value().interface.finish(stanza);

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
