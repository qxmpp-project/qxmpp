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

namespace QXmpp::Private {

void StreamOpen::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartDocument();
    writer->writeStartElement(QSL65("stream:stream"));
    if (!from.isEmpty()) {
        writer->writeAttribute(QSL65("from"), from);
    }
    writer->writeAttribute(QSL65("to"), to);
    writer->writeAttribute(QSL65("version"), QSL65("1.0"));
    writer->writeDefaultNamespace(toString65(xmlns));
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
            m_dataBuffer.clear();
            m_streamOpenElement.clear();
            Q_EMIT started();
        }
    });
    QObject::connect(socket, &QSslSocket::encrypted, this, [this]() {
        debug(u"Socket encrypted"_s);
        // this happens with direct TLS or STARTTLS
        m_dataBuffer.clear();
        m_streamOpenElement.clear();
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
    static const QRegularExpression streamStartRegex(uR"(^(<\?xml.*\?>)?\s*<stream:stream[^>]*>)"_s);
    static const QRegularExpression streamEndRegex(u"</stream:stream>$"_s);

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
        wrappedStanzas.append(u"</stream:stream>"_s);
    }

    //
    // Try to parse the wrapped XML
    //
    QDomDocument doc;
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (!doc.setContent(wrappedStanzas, QDomDocument::ParseOption::UseNamespaceProcessing)) {
#else
    if (!doc.setContent(wrappedStanzas, true)) {
#endif
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

}  // namespace QXmpp::Private
