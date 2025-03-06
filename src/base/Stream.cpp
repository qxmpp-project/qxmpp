// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "Stream.h"

#include "QXmppConstants_p.h"
#include "QXmppStreamError_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"
#include "XmppSocket.h"

#include <QDomDocument>
#include <QHostAddress>
#include <QRegularExpression>
#include <QSslSocket>
#include <QXmlStreamWriter>

using namespace QXmpp;
using namespace QXmpp::Private;

// helper for std::visit
template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

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

std::optional<StarttlsRequest> StarttlsRequest::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"starttls" || el.namespaceURI() != ns_tls) {
        return {};
    }
    return StarttlsRequest {};
}

void StarttlsRequest::toXml(QXmlStreamWriter *w) const
{
    writeEmptyElement(w, u"starttls", ns_tls);
}

std::optional<StarttlsProceed> StarttlsProceed::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"proceed" || el.namespaceURI() != ns_tls) {
        return {};
    }
    return StarttlsProceed {};
}

void StarttlsProceed::toXml(QXmlStreamWriter *w) const
{
    writeEmptyElement(w, u"proceed", ns_tls);
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
    u"unsupported-feature",
    u"unsupported-stanza-type",
    u"unsupported-version",
});

QString StreamErrorElement::streamErrorToString(StreamError e)
{
    return STREAM_ERROR_CONDITIONS.at(size_t(e)).toString();
}

std::variant<StreamErrorElement, QXmppError> StreamErrorElement::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"error" || el.namespaceURI() != ns_stream) {
        return QXmppError { u"Invalid dom element."_s, {} };
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
        return QXmppError { u"Stream error is missing valid error condition."_s, {} };
    }

    return StreamErrorElement {
        std::move(*condition),
        std::move(errorText),
    };
}

void StreamErrorElement::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(u"stream:error"_s);
    if (const auto *streamError = std::get_if<StreamError>(&condition)) {
        writer->writeStartElement(toString65(STREAM_ERROR_CONDITIONS.at(size_t(*streamError))));
        writer->writeDefaultNamespace(toString65(ns_stream_error));
        writer->writeEndElement();
    } else if (const auto *seeOtherHost = std::get_if<SeeOtherHost>(&condition)) {
        writer->writeStartElement(u"see-other-host"_s);
        writer->writeDefaultNamespace(toString65(ns_stream_error));
        writer->writeCharacters(seeOtherHost->host + u':' + QString::number(seeOtherHost->port));
        writer->writeEndElement();
    }
    writeOptionalXmlTextElement(writer, u"text", text);
    writer->writeEndElement();
}

static QString restrictedXmlErrorText(QXmlStreamReader::TokenType token)
{
    switch (token) {
    case QXmlStreamReader::Comment:
        return u"XML comments are not allowed in XMPP."_s;
    case QXmlStreamReader::DTD:
        return u"XML DTDs are not allowed in XMPP."_s;
    case QXmlStreamReader::EntityReference:
        return u"XML entity references are not allowed in XMPP."_s;
    case QXmlStreamReader::ProcessingInstruction:
        return u"XML processing instructions are not allowed in XMPP."_s;
    default:
        return {};
    }
}

DomReader::Result DomReader::process(QXmlStreamReader &r)
{
    while (true) {
        switch (r.tokenType()) {
        case QXmlStreamReader::Invalid:
            // error received
            if (r.error() == QXmlStreamReader::PrematureEndOfDocumentError) {
                return Unfinished {};
            }
            return Error { NotWellFormed, r.errorString() };
        case QXmlStreamReader::StartElement: {
            auto child = r.prefix().isNull()
                ? doc.createElement(r.name().toString())
                : doc.createElementNS(r.namespaceUri().toString(), r.qualifiedName().toString());

            // xmlns attribute
            const auto nsDeclarations = r.namespaceDeclarations();
            for (const auto &ns : nsDeclarations) {
                if (ns.prefix().isEmpty()) {
                    child.setAttribute(u"xmlns"_s, ns.namespaceUri().toString());
                } else {
                    // namespace declarations are not supported in XMPP
                    return Error { UnsupportedXmlFeature, u"XML namespace declarations are not allowed in XMPP."_s };
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
            Q_ASSERT(depth > 0);
            if (depth == 0) {
                return Error { InvalidState, u"Invalid state: Received element end instead of element start."_s };
            }

            currentElement = currentElement.parentNode().toElement();
            depth--;
            // if top-level element is complete: return
            if (depth == 0) {
                return doc.documentElement();
            }
            break;
        case QXmlStreamReader::Characters:
            // DOM reader must only be used on element start: characters on level 0 are not allowed
            Q_ASSERT(depth > 0);
            if (depth == 0) {
                return Error { InvalidState, u"Invalid state: Received top-level character data instead of element begin."_s };
            }

            currentElement.appendChild(doc.createTextNode(r.text().toString()));
            break;
        case QXmlStreamReader::NoToken:
            // skip
            break;
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
            Q_ASSERT_X(false, "DomReader", "Received document begin or end.");
            return Error { InvalidState, u"Invalid state: Received document begin or end."_s };
            break;
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            return Error { UnsupportedXmlFeature, restrictedXmlErrorText(r.tokenType()) };
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
        info(u"Socket connected to %1 %2"_s
                 .arg(m_socket->peerAddress().toString(),
                      QString::number(m_socket->peerPort())));

        // do not emit started() with direct TLS (this happens in encrypted())
        if (!m_directTls) {
            m_reader.clear();
            m_streamReceived = false;
            Q_EMIT started();
        }
    });
    QObject::connect(socket, &QAbstractSocket::disconnected, this, [this]() {
        // reset error state
        m_errorOccurred = false;
    });
    QObject::connect(socket, &QSslSocket::encrypted, this, [this]() {
        debug(u"Socket encrypted"_s);
        // this happens with direct TLS or STARTTLS
        m_reader.clear();
        m_streamReceived = false;
        Q_EMIT started();
    });
    QObject::connect(socket, &QSslSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        warning(u"Socket error: "_s + m_socket->errorString());
    });
    QObject::connect(socket, &QSslSocket::readyRead, this, [this]() {
        processData(QString::fromUtf8(m_socket->readAll()));
    });
}

bool XmppSocket::isConnected() const
{
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

void XmppSocket::connectToHost(const ServerAddress &address)
{
    m_directTls = address.type == ServerAddress::Tls;

    // connect to host
    switch (address.type) {
    case ServerAddress::Tcp:
        info(u"Connecting to %1:%2 (TCP)"_s.arg(address.host, QString::number(address.port)));
        m_socket->connectToHost(address.host, address.port);
        break;
    case ServerAddress::Tls:
        info(u"Connecting to %1:%2 (TLS)"_s.arg(address.host, QString::number(address.port)));
        Q_ASSERT(QSslSocket::supportsSsl());
        m_socket->connectToHostEncrypted(address.host, address.port);
        break;
    }
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

void XmppSocket::throwStreamError(const StreamErrorElement &error)
{
    Q_ASSERT(!m_errorOccurred);
    m_errorOccurred = true;

    sendData(serializeXml(error));
    m_socket->disconnectFromHost();
    Q_EMIT streamErrorSent(error);
}

void XmppSocket::processData(const QString &data)
{
    // stop parsing after an error has occurred
    if (m_errorOccurred) {
        return;
    }

    // Check for whitespace pings
    if (data.isEmpty()) {
        logReceived({});
        Q_EMIT stanzaReceived(QDomElement());
        return;
    }

    // log data received and process
    logReceived(data);
    m_reader.addData(data);

    // 'm_reader' parses the XML stream and 'm_domReader' creates DOM elements with the data from
    // 'm_reader'. 'm_domReader' lives as long as one stanza element is parsed.

    auto readDomElement = [this]() {
        return std::visit(
            overloaded {
                [this](const QDomElement &element) {
                    m_domReader.reset();
                    Q_EMIT stanzaReceived(element);
                    return true;
                },
                [](DomReader::Unfinished) {
                    return false;
                },
                [this](const DomReader::Error &error) {
                    switch (error.type) {
                    case DomReader::InvalidState:
                        throwStreamError({
                            StreamError::InternalServerError,
                            u"Experienced internal error while parsing XML."_s,
                        });
                        break;
                    case DomReader::NotWellFormed:
                        throwStreamError({
                            StreamError::NotWellFormed,
                            u"Not well-formed: "_s + error.text,
                        });
                        break;
                    case DomReader::UnsupportedXmlFeature:
                        throwStreamError({ StreamError::RestrictedXml, error.text });
                        break;
                    }
                    return false;
                },
            },
            m_domReader->process(m_reader));
    };

    // we're still reading a previously started top-level element
    if (m_domReader) {
        m_reader.readNext();
        if (!readDomElement()) {
            return;
        }
    }

    do {
        switch (m_reader.readNext()) {
        case QXmlStreamReader::Invalid:
            // error received
            if (m_reader.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
                return throwStreamError({ StreamError::NotWellFormed, m_reader.errorString() });
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
                // check for 'stream:stream' (this is required by the spec)
                if (m_reader.prefix() != u"stream") {
                    throwStreamError({
                        StreamError::BadNamespacePrefix,
                        u"Top-level stream element must have a namespace prefix of 'stream'."_s,
                    });
                    return;
                }

                m_streamReceived = true;
                Q_EMIT streamReceived(StreamOpen::fromXml(m_reader));
            } else if (!m_streamReceived) {
                throwStreamError({
                    StreamError::BadFormat,
                    u"Invalid element received. Expected 'stream' element qualified by 'http://etherx.jabber.org/streams' namespace."_s,
                });
                return;
            } else {
                // parse top-level stream element
                m_domReader = DomReader();
                if (!readDomElement()) {
                    return;
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
                throwStreamError({
                    StreamError::BadFormat,
                    u"Top-level, non-whitespace character data is not allowed in XMPP."_s,
                });
                return;
            }
            break;
        case QXmlStreamReader::NoToken:
            // skip
            break;
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            throwStreamError({ StreamError::RestrictedXml, restrictedXmlErrorText(m_reader.tokenType()) });
            return;
        }
    } while (!m_reader.hasError());
}

}  // namespace QXmpp::Private
