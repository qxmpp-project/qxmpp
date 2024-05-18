// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppStream.h"

#include "QXmppConstants_p.h"
#include "QXmppError.h"
#include "QXmppNonza.h"
#include "QXmppStreamError_p.h"
#include "QXmppUtils_p.h"

#include "Stream.h"
#include "XmppSocket.h"
#include "qxmlstream.h"

#include <QDomDocument>
#include <QHostAddress>
#include <QRegularExpression>
#include <QSslSocket>

using namespace QXmpp;
using namespace QXmpp::Private;

class QXmppStreamPrivate
{
public:
    QXmppStreamPrivate(QXmppStream *stream);

    XmppSocket socket;
};

QXmppStreamPrivate::QXmppStreamPrivate(QXmppStream *stream)
    : socket(stream)
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
    connect(&d->socket, &XmppSocket::stanzaReceived, this, &QXmppStream::handleStanza);
    connect(&d->socket, &XmppSocket::streamReceived, this, &QXmppStream::handleStream);
    connect(&d->socket, &XmppSocket::streamClosed, this, &QXmppStream::disconnectFromHost);
}

QXmppStream::~QXmppStream() = default;

///
/// Disconnects from the remote host.
///
void QXmppStream::disconnectFromHost()
{
    d->socket.disconnectFromHost();
}

///
/// Handles a stream start event, which occurs when the underlying transport
/// becomes ready (socket connected, encryption started).
///
void QXmppStream::handleStart()
{
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
    return d->socket.sendData(serializeXml(nonza));
}

///
/// Returns access to the XMPP socket.
///
XmppSocket &QXmppStream::xmppSocket() const
{
    return d->socket;
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

namespace QXmpp::Private {

StreamOpen StreamOpen::fromXml(QXmlStreamReader &reader)
{
    Q_ASSERT(reader.isStartElement());
    Q_ASSERT(reader.name() == u"stream");
    Q_ASSERT(reader.namespaceUri() == ns_stream);

    StreamOpen out;
    const auto attributes = reader.attributes();
    auto attribute = [&](QStringView ns, QStringView name) {
        for (const auto &a : attributes) {
            if (a.name() == name && a.namespaceUri() == ns) {
                return a.value().toString();
            }
        }
        return QString();
    };

    out.from = attribute({}, u"from");
    out.to = attribute({}, u"to");
    out.id = attribute({}, u"id");
    out.version = attribute({}, u"version");

    const auto namespaceDeclarations = reader.namespaceDeclarations();
    for (const auto &ns : namespaceDeclarations) {
        if (ns.prefix().isEmpty()) {
            out.xmlns = ns.namespaceUri().toString();
        }
    }

    return out;
}

void StreamOpen::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartDocument();
    writer->writeStartElement(QSL65("stream:stream"));
    writeOptionalXmlAttribute(writer, u"from", from);
    writeOptionalXmlAttribute(writer, u"to", to);
    writeOptionalXmlAttribute(writer, u"id", id);
    writeOptionalXmlAttribute(writer, u"version", version);
    writer->writeDefaultNamespace(xmlns);
    writer->writeNamespace(toString65(ns_stream), QSL65("stream"));
    writer->writeCharacters({});
}

void CsiActive::toXml(QXmlStreamWriter *w) const
{
    writeEmptyElement(w, u"active", ns_csi);
}

void CsiInactive::toXml(QXmlStreamWriter *w) const
{
    writeEmptyElement(w, u"inactive", ns_csi);
}

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
QString StreamErrorElement::streamErrorToString(StreamError e)
{
    return STREAM_ERROR_CONDITIONS.at(size_t(e)).toString();
}

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
                condition = SeeOtherHost { host, quint16(port > 0 ? port : XMPP_DEFAULT_PORT) };
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

DomReader::State DomReader::process(QXmlStreamReader &r)
{
    while (true) {
        switch (r.tokenType()) {
        case QXmlStreamReader::Invalid:
            // error received
            if (r.error() == QXmlStreamReader::PrematureEndOfDocumentError) {
                return Unfinished;
            }
            return ErrorOccurred;
        case QXmlStreamReader::StartElement: {
            qDebug() << "start element token";
            auto child = r.prefix().isNull()
                ? doc.createElement(r.name().toString())
                : doc.createElementNS(r.namespaceUri().toString(), r.qualifiedName().toString());

            // xmlns attribute
            const auto nsDeclarations = r.namespaceDeclarations();
            for (const auto &ns : nsDeclarations) {
                if (ns.prefix().isEmpty()) {
                    child.setAttribute(QStringLiteral("xmlns"), ns.namespaceUri().toString());
                } else {
                    // namespace declarations are not supported in XMPP
                    qDebug() << "err namespace decl";
                    return ErrorOccurred;
                }
            }

            // other attributes
            const auto attributes = r.attributes();
            for (const auto &a : attributes) {
                child.setAttribute(a.name().toString(), a.value().toString());
            }

            if (currentElement.isNull()) {
                doc.appendChild(child);
            } else {
                currentElement.appendChild(child);
            }
            depth++;
            currentElement = child;
            break;
        }
        case QXmlStreamReader::EndElement:
            qDebug() << "end element token";
            if (depth == 0) {
                qDebug() << "depth == 0";
                return ErrorOccurred;
            }

            currentElement = currentElement.parentNode().toElement();
            depth--;
            qDebug() << "depth" << depth;
            if (depth == 0) {
                return Finished;
            }
            break;
        case QXmlStreamReader::Characters:
            if (depth == 0) {
                qDebug() << "depth == 0";
                return ErrorOccurred;
            }

            currentElement.appendChild(doc.createTextNode(r.text().toString()));
            break;
        case QXmlStreamReader::NoToken:
            // skip
            break;
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            qDebug() << "not allowed";
            // not allowed or unexpected
            return ErrorOccurred;
        }
        r.readNext();
    }
}

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
        m_reader.clear();
        m_streamReceived = false;
        Q_EMIT started();
    });
    QObject::connect(socket, &QSslSocket::encrypted, this, [this]() {
        debug(QStringLiteral("Socket encrypted"));
        m_reader.clear();
        m_streamReceived = false;
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
    // Check for whitespace pings
    if (data.isEmpty()) {
        qDebug() << "empty ping received";
        logReceived({});
        Q_EMIT stanzaReceived(QDomElement());
        return;
    }

    // log data received and process
    logReceived(data);
    m_reader.addData(data);

    // we're still reading a previously started top-level element
    if (m_domReader) {
        m_reader.readNext();
        switch (m_domReader->process(m_reader)) {
        case DomReader::Finished:
            Q_EMIT stanzaReceived(m_domReader->element());
            m_domReader.reset();
            break;
        case DomReader::Unfinished:
            return;
        case DomReader::ErrorOccurred:
            // emit error
            break;
        }
    }

    do {
        switch (m_reader.readNext()) {
        case QXmlStreamReader::Invalid:
            // error received
            if (m_reader.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
                // emit error
            }
            break;
        case QXmlStreamReader::StartDocument:
            // pre-stream open
            break;
        case QXmlStreamReader::EndDocument:
            // post-stream close
            break;
        case QXmlStreamReader::StartElement:
            // stream open or stream-level element
            if (m_reader.name() == u"stream" && m_reader.namespaceUri() == ns_stream) {
                m_streamReceived = true;
                Q_EMIT streamReceived(StreamOpen::fromXml(m_reader));
            } else if (!m_streamReceived) {
                // error: expected stream open element
                qDebug() << "err no stream recevied";
            } else {
                qDebug() << "start el";
                // parse top-level stream element
                m_domReader = DomReader();

                switch (m_domReader->process(m_reader)) {
                case DomReader::Finished:
                    Q_EMIT stanzaReceived(m_domReader->element());
                    m_domReader.reset();
                    break;
                case DomReader::Unfinished:
                    qDebug() << "unfi";
                    return;
                case DomReader::ErrorOccurred:
                    qDebug() << "el err";
                    // emit error
                    break;
                }
            }
            break;
        case QXmlStreamReader::EndElement:
            // end of stream
            Q_EMIT streamClosed();
            break;
        case QXmlStreamReader::Characters:
            if (m_reader.isWhitespace()) {
                logReceived({});
                Q_EMIT stanzaReceived(QDomElement());
            } else {
                // invalid: emit error
            }
            break;
        case QXmlStreamReader::NoToken:
            // skip
            break;
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            // not allowed in XMPP: emit error
            break;
        }
    } while (!m_reader.hasError());
}

}  // namespace QXmpp::Private
