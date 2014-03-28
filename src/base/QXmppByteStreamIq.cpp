/*
 * Copyright (C) 2008-2014 The QXmpp developers
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

#include <QDomElement>

#include "QXmppByteStreamIq.h"
#include "QXmppConstants.h"
#include "QXmppUtils.h"

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

void  QXmppByteStreamIq::StreamHost::setZeroconf(const QString &zeroconf)
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
    return element.firstChildElement("query").namespaceURI() == ns_bytestreams;
}

void QXmppByteStreamIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    m_sid = queryElement.attribute("sid");
    const QString modeStr = queryElement.attribute("mode");
    if (modeStr == "tcp")
        m_mode = Tcp;
    else if (modeStr == "udp")
        m_mode = Udp;
    else
        m_mode = None;

    QDomElement hostElement = queryElement.firstChildElement("streamhost");
    while (!hostElement.isNull())
    {
        StreamHost streamHost;
        streamHost.setHost(hostElement.attribute("host"));
        streamHost.setJid(hostElement.attribute("jid"));
        streamHost.setPort(hostElement.attribute("port").toInt());
        streamHost.setZeroconf(hostElement.attribute("zeroconf"));
        m_streamHosts.append(streamHost);

        hostElement = hostElement.nextSiblingElement("streamhost");
    }
    m_activate = queryElement.firstChildElement("activate").text();
    m_streamHostUsed = queryElement.firstChildElement("streamhost-used").attribute("jid");
}

void QXmppByteStreamIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns", ns_bytestreams);
    helperToXmlAddAttribute(writer, "sid", m_sid);
    QString modeStr;
    if (m_mode == Tcp)
        modeStr = "tcp";
    else if (m_mode == Udp)
        modeStr = "udp";
    helperToXmlAddAttribute(writer, "mode", modeStr);
    foreach (const StreamHost& streamHost, m_streamHosts)
    {
        writer->writeStartElement("streamhost");
        helperToXmlAddAttribute(writer, "host", streamHost.host());
        helperToXmlAddAttribute(writer, "jid", streamHost.jid());
        helperToXmlAddAttribute(writer, "port", QString::number(streamHost.port()));
        helperToXmlAddAttribute(writer, "zeroconf", streamHost.zeroconf());
        writer->writeEndElement();
    }
    if (!m_activate.isEmpty())
        helperToXmlAddTextElement(writer, "activate", m_activate);
    if (!m_streamHostUsed.isEmpty())
    {
        writer->writeStartElement("streamhost-used");
        helperToXmlAddAttribute(writer, "jid", m_streamHostUsed);
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond
