// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppByteStreamIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include <QDomElement>

using namespace QXmpp::Private;

///
/// \enum QXmppByteStreamIq::Mode
///
/// Used to select the transport layer protocol (TCP or UDP).
///

///
/// \class QXmppByteStreamIq::StreamHost
///
/// StreamHost represents information about a specific SOCKS5 bytestreams host.
///

///
/// Returns the host address of the stream host.
///
QString QXmppByteStreamIq::StreamHost::host() const
{
    return m_host;
}

///
/// Sets the host address of the stream host.
///
void QXmppByteStreamIq::StreamHost::setHost(const QString &host)
{
    m_host = host;
}

///
/// Returns the JID of the stream host.
///
QString QXmppByteStreamIq::StreamHost::jid() const
{
    return m_jid;
}

///
/// Sets the JID of the stream host.
///
void QXmppByteStreamIq::StreamHost::setJid(const QString &jid)
{
    m_jid = jid;
}

///
/// Returns the port of the stream host.
///
quint16 QXmppByteStreamIq::StreamHost::port() const
{
    return m_port;
}

///
/// Sets the port of the stream host.
///
void QXmppByteStreamIq::StreamHost::setPort(quint16 port)
{
    m_port = port;
}

///
/// Returns the zero-configuration service available for bytestreaming.
///
QString QXmppByteStreamIq::StreamHost::zeroconf() const
{
    return m_zeroconf;
}

///
/// Sets the zero-configuration service available for bytestreaming.
///
void QXmppByteStreamIq::StreamHost::setZeroconf(const QString &zeroconf)
{
    m_zeroconf = zeroconf;
}

///
/// \class QXmppByteStreamIq
///
/// QXmppByteStreamIq represents a SOCKS5 bytestreams negoatiation IQ as defined
/// by \xep{0065, SOCKS5 Bytestreams}.
///

///
/// Returns the protocol type (UDP or TCP).
///
QXmppByteStreamIq::Mode QXmppByteStreamIq::mode() const
{
    return m_mode;
}

///
/// Sets the protocol type (UDP or TCP).
///
void QXmppByteStreamIq::setMode(QXmppByteStreamIq::Mode mode)
{
    m_mode = mode;
}

///
/// Returns the bytestream stream ID.
///
QString QXmppByteStreamIq::sid() const
{
    return m_sid;
}

///
/// Sets the bytestream stream ID.
///
void QXmppByteStreamIq::setSid(const QString &sid)
{
    m_sid = sid;
}

///
/// Returns the jid of the target.
///
QString QXmppByteStreamIq::activate() const
{
    return m_activate;
}

///
/// Sets the jid of the target.
///
void QXmppByteStreamIq::setActivate(const QString &activate)
{
    m_activate = activate;
}

///
/// Returns available SOCKS5 stream hosts.
///
QList<QXmppByteStreamIq::StreamHost> QXmppByteStreamIq::streamHosts() const
{
    return m_streamHosts;
}

///
/// Sets available SOCKS5 stream hosts.
///
void QXmppByteStreamIq::setStreamHosts(const QList<QXmppByteStreamIq::StreamHost> &streamHosts)
{
    m_streamHosts = streamHosts;
}

///
/// Returns the JID of the used stream host.
///
QString QXmppByteStreamIq::streamHostUsed() const
{
    return m_streamHostUsed;
}

///
/// Sets the JID of the used stream host.
///
void QXmppByteStreamIq::setStreamHostUsed(const QString &jid)
{
    m_streamHostUsed = jid;
}

///
/// Returns whether \a element is an IQ element with a bytestream query.
///
bool QXmppByteStreamIq::isByteStreamIq(const QDomElement &element)
{
    return isIqType(element, u"query", ns_bytestreams);
}

/// \cond
void QXmppByteStreamIq::parseElementFromChild(const QDomElement &element)
{
    auto queryElement = firstChildElement(element, u"query", ns_bytestreams);
    m_sid = queryElement.attribute(QStringLiteral("sid"));
    const auto modeStr = queryElement.attribute(QStringLiteral("mode"));
    if (modeStr == QStringLiteral("tcp")) {
        m_mode = Tcp;
    } else if (modeStr == QStringLiteral("udp")) {
        m_mode = Udp;
    } else {
        m_mode = None;
    }

    QDomElement hostElement = firstChildElement(queryElement, u"streamhost");
    while (!hostElement.isNull()) {
        StreamHost streamHost;
        streamHost.setHost(hostElement.attribute(QStringLiteral("host")));
        streamHost.setJid(hostElement.attribute(QStringLiteral("jid")));
        streamHost.setPort(hostElement.attribute(QStringLiteral("port")).toInt());
        streamHost.setZeroconf(hostElement.attribute(QStringLiteral("zeroconf")));
        m_streamHosts.append(streamHost);

        hostElement = hostElement.nextSiblingElement(QStringLiteral("streamhost"));
    }
    m_activate = firstChildElement(queryElement, u"activate").text();
    m_streamHostUsed = firstChildElement(queryElement, u"streamhost-used").attribute(QStringLiteral("jid"));
}

void QXmppByteStreamIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("query"));
    writer->writeDefaultNamespace(toString65(ns_bytestreams));
    writeOptionalXmlAttribute(writer, u"sid", m_sid);
    QString modeStr;
    if (m_mode == Tcp) {
        modeStr = QStringLiteral("tcp");
    } else if (m_mode == Udp) {
        modeStr = QStringLiteral("udp");
    }
    writeOptionalXmlAttribute(writer, u"mode", modeStr);
    for (const auto &streamHost : m_streamHosts) {
        writer->writeStartElement(QStringLiteral("streamhost"));
        writeOptionalXmlAttribute(writer, u"host", streamHost.host());
        writeOptionalXmlAttribute(writer, u"jid", streamHost.jid());
        writeOptionalXmlAttribute(writer, u"port", QString::number(streamHost.port()));
        writeOptionalXmlAttribute(writer, u"zeroconf", streamHost.zeroconf());
        writer->writeEndElement();
    }
    if (!m_activate.isEmpty()) {
        writeXmlTextElement(writer, u"activate", m_activate);
    }
    if (!m_streamHostUsed.isEmpty()) {
        writer->writeStartElement(QStringLiteral("streamhost-used"));
        writeOptionalXmlAttribute(writer, u"jid", m_streamHostUsed);
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond
