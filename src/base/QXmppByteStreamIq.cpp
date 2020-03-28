/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
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

#include "QXmppByteStreamIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>

QString QXmppByteStreamIq::StreamHost::host() const
{
    return m_host;
}

void QXmppByteStreamIq::StreamHost::setHost(const QString &host)
{
    m_host = host;
}

QString QXmppByteStreamIq::StreamHost::jid() const
{
    return m_jid;
}

void QXmppByteStreamIq::StreamHost::setJid(const QString &jid)
{
    m_jid = jid;
}

quint16 QXmppByteStreamIq::StreamHost::port() const
{
    return m_port;
}

void QXmppByteStreamIq::StreamHost::setPort(quint16 port)
{
    m_port = port;
}

QString QXmppByteStreamIq::StreamHost::zeroconf() const
{
    return m_zeroconf;
}

void QXmppByteStreamIq::StreamHost::setZeroconf(const QString &zeroconf)
{
    m_zeroconf = zeroconf;
}

QXmppByteStreamIq::Mode QXmppByteStreamIq::mode() const
{
    return m_mode;
}

void QXmppByteStreamIq::setMode(QXmppByteStreamIq::Mode mode)
{
    m_mode = mode;
}

QString QXmppByteStreamIq::sid() const
{
    return m_sid;
}

void QXmppByteStreamIq::setSid(const QString &sid)
{
    m_sid = sid;
}

QString QXmppByteStreamIq::activate() const
{
    return m_activate;
}

void QXmppByteStreamIq::setActivate(const QString &activate)
{
    m_activate = activate;
}

QList<QXmppByteStreamIq::StreamHost> QXmppByteStreamIq::streamHosts() const
{
    return m_streamHosts;
}

void QXmppByteStreamIq::setStreamHosts(const QList<QXmppByteStreamIq::StreamHost> &streamHosts)
{
    m_streamHosts = streamHosts;
}

QString QXmppByteStreamIq::streamHostUsed() const
{
    return m_streamHostUsed;
}

void QXmppByteStreamIq::setStreamHostUsed(const QString &jid)
{
    m_streamHostUsed = jid;
}

/// \cond
bool QXmppByteStreamIq::isByteStreamIq(const QDomElement &element)
{
    return element.firstChildElement(QStringLiteral("query")).namespaceURI() == ns_bytestreams;
}

void QXmppByteStreamIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(QStringLiteral("query"));
    m_sid = queryElement.attribute(QStringLiteral("sid"));
    const QString modeStr = queryElement.attribute(QStringLiteral("mode"));
    if (modeStr == QStringLiteral("tcp"))
        m_mode = Tcp;
    else if (modeStr == QStringLiteral("udp"))
        m_mode = Udp;
    else
        m_mode = None;

    QDomElement hostElement = queryElement.firstChildElement(QStringLiteral("streamhost"));
    while (!hostElement.isNull()) {
        StreamHost streamHost;
        streamHost.setHost(hostElement.attribute(QStringLiteral("host")));
        streamHost.setJid(hostElement.attribute(QStringLiteral("jid")));
        streamHost.setPort(hostElement.attribute(QStringLiteral("port")).toInt());
        streamHost.setZeroconf(hostElement.attribute(QStringLiteral("zeroconf")));
        m_streamHosts.append(streamHost);

        hostElement = hostElement.nextSiblingElement(QStringLiteral("streamhost"));
    }
    m_activate = queryElement.firstChildElement(QStringLiteral("activate")).text();
    m_streamHostUsed = queryElement.firstChildElement(QStringLiteral("streamhost-used")).attribute(QStringLiteral("jid"));
}

void QXmppByteStreamIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("query"));
    writer->writeDefaultNamespace(ns_bytestreams);
    helperToXmlAddAttribute(writer, QStringLiteral("sid"), m_sid);
    QString modeStr;
    if (m_mode == Tcp)
        modeStr = QStringLiteral("tcp");
    else if (m_mode == Udp)
        modeStr = QStringLiteral("udp");
    helperToXmlAddAttribute(writer, QStringLiteral("mode"), modeStr);
    for (const auto &streamHost : m_streamHosts) {
        writer->writeStartElement(QStringLiteral("streamhost"));
        helperToXmlAddAttribute(writer, QStringLiteral("host"), streamHost.host());
        helperToXmlAddAttribute(writer, QStringLiteral("jid"), streamHost.jid());
        helperToXmlAddAttribute(writer, QStringLiteral("port"), QString::number(streamHost.port()));
        helperToXmlAddAttribute(writer, QStringLiteral("zeroconf"), streamHost.zeroconf());
        writer->writeEndElement();
    }
    if (!m_activate.isEmpty())
        helperToXmlAddTextElement(writer, QStringLiteral("activate"), m_activate);
    if (!m_streamHostUsed.isEmpty()) {
        writer->writeStartElement(QStringLiteral("streamhost-used"));
        helperToXmlAddAttribute(writer, QStringLiteral("jid"), m_streamHostUsed);
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond
