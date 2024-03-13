// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppStream.h"

#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppIq.h"
#include "QXmppPacket_p.h"
#include "QXmppStreamError_p.h"
#include "QXmppStreamManagement_p.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include <QDomDocument>
#include <QHostAddress>
#include <QRegularExpression>
#include <QSslSocket>

using namespace QXmpp;
using namespace QXmpp::Private;

constexpr quint16 XMPP_PORT_FALLBACK = 5222;

class QXmppStreamPrivate
{
public:
    QXmppStreamPrivate(QXmppStream *stream);

    XmppSocket socket;

    // stream management
    StreamAckManager streamAckManager;

    // iq response handling
    OutgoingIqManager iqManager;
};

QXmppStreamPrivate::QXmppStreamPrivate(QXmppStream *stream)
    : socket(stream),
      streamAckManager(socket),
      iqManager(stream, streamAckManager)
{
}

///
/// Constructs a base XMPP stream.
///
/// \param parent
///
QXmppStream::QXmppStream(QObject *parent)
    : QXmppLoggable(parent),
      d(std::make_unique<QXmppStreamPrivate>(this))
{
    connect(&d->socket, &XmppSocket::started, this, &QXmppStream::handleStart);
    connect(&d->socket, &XmppSocket::stanzaReceived, this, &QXmppStream::onStanzaReceived);
    connect(&d->socket, &XmppSocket::streamReceived, this, &QXmppStream::handleStream);
    connect(&d->socket, &XmppSocket::streamClosed, this, &QXmppStream::disconnectFromHost);
}

///
/// Destroys a base XMPP stream.
///
QXmppStream::~QXmppStream()
{
    // causes tasks to be finished
    d->streamAckManager.resetCache();
    d->iqManager.cancelAll();
}

///
/// Disconnects from the remote host.
///
void QXmppStream::disconnectFromHost()
{
    d->streamAckManager.handleDisconnect();
    d->socket.disconnectFromHost();
}

///
/// Handles a stream start event, which occurs when the underlying transport
/// becomes ready (socket connected, encryption started).
///
/// If you redefine handleStart(), make sure to call the base class's method.
///
void QXmppStream::handleStart()
{
    d->streamAckManager.handleStart();
}

///
/// Returns true if the stream is connected.
///
bool QXmppStream::isConnected() const
{
    return d->socket.isConnected();
}

///
/// Sends raw data to the peer.
///
/// \param data
///
bool QXmppStream::sendData(const QByteArray &data)
{
    return d->socket.sendData(data);
}

///
/// Sends an XMPP packet to the peer.
///
/// \param nonza
///
bool QXmppStream::sendPacket(const QXmppNonza &nonza)
{
    return d->streamAckManager.sendPacketCompat(nonza);
}

///
/// Returns access to the XMPP socket.
///
XmppSocket &QXmppStream::xmppSocket() const
{
    return d->socket;
}

///
/// Returns the manager for Stream Management
///
StreamAckManager &QXmppStream::streamAckManager() const
{
    return d->streamAckManager;
}

///
/// Returns the manager for outgoing IQ request tracking.
///
OutgoingIqManager &QXmppStream::iqManager() const
{
    return d->iqManager;
}

///
/// Returns the QSslSocket used for this stream.
///
QSslSocket *QXmppStream::socket() const
{
    return d->socket.socket();
}

///
/// Sets the QSslSocket used for this stream.
///
void QXmppStream::setSocket(QSslSocket *socket)
{
    d->socket.setSocket(socket);
}

void QXmppStream::onStanzaReceived(const QDomElement &stanza)
{
    // handle possible stream management packets first
    if (streamAckManager().handleStanza(stanza) || iqManager().handleStanza(stanza)) {
        return;
    }

    // process all other kinds of packets
    handleStanza(stanza);
}

namespace QXmpp::Private {

constexpr auto STREAM_ERROR_CONDITIONS = to_array<QStringView>({
    u"bad-format",
    u"bad-namespace-prefix",
    u"conflict",
    u"connection-timeout",
    u"host-gone",
    u"host-unknown",
    u"improper-addressing",
    u"internal-server-error",
    u"invalid-from",
    u"invalid-id",
    u"invalid-namespace",
    u"invalid-xml",
    u"not-authorized",
    u"not-well-formed",
    u"policy-violation",
    u"remote-connection-failed",
    u"reset",
    u"resource-constraint",
    u"restricted-xml",
    u"system-shutdown",
    u"undefined-condition",
    u"unsupported-encoding",
    u"unsupported-stanza-type",
    u"unsupported-version",
});

/// \cond
std::variant<StreamErrorElement, QXmppError> StreamErrorElement::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"error" || el.namespaceURI() != ns_stream) {
        return QXmppError { QStringLiteral("Invalid dom element."), {} };
    }

    std::optional<StreamErrorElement::Condition> condition;
    QString errorText;

    for (const auto &subEl : iterChildElements(el, {}, ns_stream_error)) {
        auto tagName = subEl.tagName();
        if (tagName == u"text") {
            errorText = subEl.text();
        } else if (auto conditionEnum = enumFromString<StreamError>(STREAM_ERROR_CONDITIONS, tagName)) {
            condition = conditionEnum;
        } else if (tagName == u"see-other-host") {
            if (auto [host, port] = parseHostAddress(subEl.text()); !host.isEmpty()) {
                condition = SeeOtherHost { host, port > 0 ? quint16(port) : XMPP_PORT_FALLBACK };
            }
        }
    }

    if (!condition) {
        return QXmppError { QStringLiteral("Stream error is missing valid error condition."), {} };
    }

    return StreamErrorElement {
        std::move(*condition),
        std::move(errorText),
    };
}
/// \endcond

XmppSocket::XmppSocket(QObject *parent)
    : QXmppLoggable(parent)
{
}

void XmppSocket::setSocket(QSslSocket *socket)
{
    m_socket = socket;
    if (!m_socket) {
        return;
    }

    QObject::connect(socket, &QAbstractSocket::connected, this, [this]() {
        info(QStringLiteral("Socket connected to %1 %2")
                 .arg(m_socket->peerAddress().toString(),
                      QString::number(m_socket->peerPort())));
        m_dataBuffer.clear();
        m_streamOpenElement.clear();
        Q_EMIT started();
    });
    QObject::connect(socket, &QSslSocket::encrypted, this, [this]() {
        debug(QStringLiteral("Socket encrypted"));
        m_dataBuffer.clear();
        m_streamOpenElement.clear();
        Q_EMIT started();
    });
    QObject::connect(socket, &QSslSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        warning(QStringLiteral("Socket error: ") + m_socket->errorString());
    });
    QObject::connect(socket, &QSslSocket::readyRead, this, [this]() {
        processData(QString::fromUtf8(m_socket->readAll()));
    });
}

bool XmppSocket::isConnected() const
{
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

void XmppSocket::disconnectFromHost()
{
    if (m_socket) {
        if (m_socket->state() == QAbstractSocket::ConnectedState) {
            sendData(QByteArrayLiteral("</stream:stream>"));
            m_socket->flush();
        }
        // FIXME: according to RFC 6120 section 4.4, we should wait for
        // the incoming stream to end before closing the socket
        m_socket->disconnectFromHost();
    }
}

bool XmppSocket::sendData(const QByteArray &data)
{
    logSent(QString::fromUtf8(data));
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        return false;
    }
    return m_socket->write(data) == data.size();
}

void XmppSocket::processData(const QString &data)
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
    m_dataBuffer.append(data);

    //
    // Check for whitespace pings
    //
    if (m_dataBuffer.isEmpty() || m_dataBuffer.trimmed().isEmpty()) {
        m_dataBuffer.clear();

        logReceived({});
        Q_EMIT stanzaReceived(QDomElement());
        return;
    }

    //
    // Check whether we received a stream open or closing tag
    //
    static const QRegularExpression streamStartRegex(QStringLiteral(R"(^(<\?xml.*\?>)?\s*<stream:stream[^>]*>)"));
    static const QRegularExpression streamEndRegex(QStringLiteral("</stream:stream>$"));

    auto streamOpenMatch = streamStartRegex.match(m_dataBuffer);
    bool hasStreamOpen = streamOpenMatch.hasMatch();

    bool hasStreamClose = streamEndRegex.match(m_dataBuffer).hasMatch();

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
    auto wrappedStanzas = m_dataBuffer;
    if (!hasStreamOpen) {
        wrappedStanzas.prepend(m_streamOpenElement);
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
    logReceived(m_dataBuffer);
    m_dataBuffer.clear();

    // process stream start
    if (hasStreamOpen) {
        m_streamOpenElement = streamOpenMatch.captured();
        Q_EMIT streamReceived(doc.documentElement());
    }

    // process stanzas
    auto stanza = doc.documentElement().firstChildElement();
    for (; !stanza.isNull(); stanza = stanza.nextSiblingElement()) {
        Q_EMIT stanzaReceived(stanza);
    }

    // process stream end
    if (hasStreamClose) {
        Q_EMIT streamClosed();
    }
}

struct IqState {
    QXmppPromise<IqResult> interface;
    QString jid;
};

OutgoingIqManager::OutgoingIqManager(QXmppLoggable *l, StreamAckManager &streamAckManager)
    : l(l),
      m_streamAckManager(streamAckManager)
{
}

OutgoingIqManager::~OutgoingIqManager() = default;

QXmppTask<IqResult> OutgoingIqManager::sendIq(QXmppIq &&iq, const QString &to)
{
    if (iq.id().isEmpty()) {
        warning(QStringLiteral("QXmppStream::sendIq() error: ID is empty. Using random ID."));
        iq.setId(QXmppUtils::generateStanzaUuid());
    }
    if (hasId(iq.id())) {
        warning(QStringLiteral("QXmppStream::sendIq() error:"
                               "The IQ's ID (\"%1\") is already in use. Using random ID.")
                    .arg(iq.id()));
        iq.setId(QXmppUtils::generateStanzaUuid());
    }

    return sendIq(QXmppPacket(iq), iq.id(), to);
}

QXmppTask<IqResult> OutgoingIqManager::sendIq(QXmppPacket &&packet, const QString &id, const QString &to)
{
    auto task = start(id, to);

    // the task only finishes instantly if there was an error
    if (task.isFinished()) {
        return task;
    }

    // send request IQ and report sending errors (sending success is not reported in any way)
    m_streamAckManager.send(std::move(packet)).then(l, [this, id](SendResult result) {
        if (std::holds_alternative<QXmppError>(result)) {
            finish(id, std::get<QXmppError>(std::move(result)));
        }
    });

    return task;
}

bool OutgoingIqManager::hasId(const QString &id) const
{
    return m_requests.find(id) != m_requests.end();
}

bool OutgoingIqManager::isIdValid(const QString &id) const
{
    return !id.isEmpty() && !hasId(id);
}

QXmppTask<IqResult> OutgoingIqManager::start(const QString &id, const QString &to)
{
    if (!isIdValid(id)) {
        return makeReadyTask<IqResult>(
            QXmppError { QStringLiteral("Invalid IQ id: empty or in use."),
                         SendError::Disconnected });
    }

    if (to.isEmpty()) {
        return makeReadyTask<IqResult>(
            QXmppError { QStringLiteral("The 'to' address must be set so the stream can match the response."),
                         SendError::Disconnected });
    }

    auto [itr, success] = m_requests.emplace(id, IqState { {}, to });
    return itr->second.interface.task();
}

void OutgoingIqManager::finish(const QString &id, IqResult &&result)
{
    if (auto itr = m_requests.find(id); itr != m_requests.end()) {
        itr->second.interface.finish(std::move(result));
        m_requests.erase(itr);
    }
}

void OutgoingIqManager::cancelAll()
{
    for (auto &[id, state] : m_requests) {
        state.interface.finish(QXmppError {
            QStringLiteral("IQ has been cancelled."),
            QXmpp::SendError::Disconnected });
    }
    m_requests.clear();
}

bool OutgoingIqManager::handleStanza(const QDomElement &stanza)
{
    if (stanza.tagName() != u"iq") {
        return false;
    }

    // only accept "result" and "error" types
    const auto iqType = stanza.attribute(QStringLiteral("type"));
    if (iqType != u"result" && iqType != u"error") {
        return false;
    }

    const auto id = stanza.attribute(QStringLiteral("id"));
    auto itr = m_requests.find(id);
    if (itr == m_requests.end()) {
        return false;
    }

    auto &promise = itr->second.interface;
    const auto &expectedFrom = itr->second.jid;

    // Check that the sender of the response matches the recipient of the request.
    // Stanzas coming from the server on behalf of the user's account must have no "from"
    // attribute or have it set to the user's bare JID.
    // If 'from' is empty, the IQ has been sent by the server. In this case we don't need to
    // do the check as we trust the server anyways.
    if (auto from = stanza.attribute(QStringLiteral("from")); !from.isEmpty() && from != expectedFrom) {
        warning(QStringLiteral("Ignored received IQ response to request '%1' because of wrong sender '%2' instead of expected sender '%3'")
                    .arg(id, from, expectedFrom));
        return false;
    }

    // report IQ errors as QXmppError (this makes it impossible to parse the full error IQ,
    // but that is okay for now)
    if (iqType == u"error") {
        QXmppIq iq;
        iq.parse(stanza);
        if (auto err = iq.errorOptional()) {
            // report stanza error
            promise.finish(QXmppError { err->text(), *err });
        } else {
            // this shouldn't happen (no <error/> element in IQ of type error)
            using Err = QXmppStanza::Error;
            promise.finish(QXmppError { QStringLiteral("IQ error"), Err(Err::Cancel, Err::UndefinedCondition) });
        }
    } else {
        // report stanza element for parsing
        promise.finish(stanza);
    }

    m_requests.erase(itr);
    return true;
}

void OutgoingIqManager::warning(const QString &message)
{
    Q_EMIT l->logMessage(QXmppLogger::WarningMessage, message);
}

}  // namespace QXmpp::Private
