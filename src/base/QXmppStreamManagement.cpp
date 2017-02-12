/*
 * Copyright (C) 2008-2013 The QXmpp developers
 *
 * Authors:
 *  Niels Ole Salscheider
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

#include "QXmppStreamManagement_p.h"
#include "QXmppStanza_p.h"

#include "QXmppConstants_p.h"

QXmppStreamManagementEnable::QXmppStreamManagementEnable(const bool resume, const unsigned max)
    : m_resume(resume), m_max(max)
{
}

bool QXmppStreamManagementEnable::resume() const
{
    return m_resume;
}

void QXmppStreamManagementEnable::setResume(bool resume)
{
    m_resume = resume;
}

unsigned QXmppStreamManagementEnable::max() const
{
    return m_max;
}

void QXmppStreamManagementEnable::setMax(const unsigned max)
{
    m_max = max;
}

bool QXmppStreamManagementEnable::isStreamManagementEnable(const QDomElement &element)
{
    return element.tagName() == QLatin1String("enable") &&
           element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementEnable::parse(const QDomElement &element)
{
    QString resume = element.attribute("resume");
    m_resume = resume == QString("true") || resume == QString("1");
    m_max = element.attribute("max").toUInt();
}

void QXmppStreamManagementEnable::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("enable");
    writer->writeAttribute("xmlns", ns_stream_management);
    if (m_resume)
        writer->writeAttribute("resume", "true");
    if (m_max > 0)
        writer->writeAttribute("max", QString::number(m_max));
    writer->writeEndElement();
}

QXmppStreamManagementEnabled::QXmppStreamManagementEnabled(const bool resume, const QString id, const unsigned max, const QString location)
    : m_resume(resume), m_id(id), m_max(max), m_location(location)
{
}

bool QXmppStreamManagementEnabled::resume() const
{
    return m_resume;
}

void QXmppStreamManagementEnabled::setResume(const bool resume)
{
    m_resume = resume;
}

QString QXmppStreamManagementEnabled::id() const
{
    return m_id;
}

void QXmppStreamManagementEnabled::setId(const QString id)
{
    m_id = id;
}

unsigned QXmppStreamManagementEnabled::max() const
{
    return m_max;
}

void QXmppStreamManagementEnabled::setMax(const unsigned max)
{
    m_max = max;
}

QString QXmppStreamManagementEnabled::location() const
{
    return m_location;
}

void QXmppStreamManagementEnabled::setLocation(const QString location)
{
    m_location = location;
}

bool QXmppStreamManagementEnabled::isStreamManagementEnabled(const QDomElement &element)
{
    return element.tagName() == QLatin1String("enabled") &&
           element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementEnabled::parse(const QDomElement &element)
{
    QString resume = element.attribute("resume");
    m_resume = resume == QString("true") || resume == QString("1");
    m_max = element.attribute("max").toUInt();
    m_location = element.attribute("location");
}

void QXmppStreamManagementEnabled::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("enable");
    writer->writeAttribute("xmlns", ns_stream_management);
    if (m_resume)
        writer->writeAttribute("resume", "true");
    if (m_max > 0)
        writer->writeAttribute("max", QString::number(m_max));
    if (!m_location.isEmpty())
        writer->writeAttribute("location", m_location);
    writer->writeEndElement();
}

QXmppStreamManagementResume::QXmppStreamManagementResume(const unsigned h, const QString& previd)
    : m_h(h), m_previd(previd)
{
}

unsigned QXmppStreamManagementResume::h() const
{
    return m_h;
}

void QXmppStreamManagementResume::setH(const unsigned h)
{
    m_h = h;
}

QString QXmppStreamManagementResume::prevId() const
{
    return m_previd;
}

void QXmppStreamManagementResume::setPrevId(const QString& previd)
{
    m_previd = previd;
}

bool QXmppStreamManagementResume::isStreamManagementResume(const QDomElement &element)
{
    return element.tagName() == QLatin1String("resume") &&
           element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementResume::parse(const QDomElement &element)
{
    m_h = element.attribute("h").toUInt();
    m_previd = element.attribute("previd");
}

void QXmppStreamManagementResume::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("resume");
    writer->writeAttribute("h", QString::number(m_h));
    writer->writeAttribute("previd", m_previd);
    writer->writeEndElement();
}

QXmppStreamManagementResumed::QXmppStreamManagementResumed(const unsigned h, const QString& previd)
    : m_h(h), m_previd(previd)
{
}

unsigned QXmppStreamManagementResumed::h() const
{
    return m_h;
}

void QXmppStreamManagementResumed::setH(const unsigned h)
{
    m_h = h;
}

QString QXmppStreamManagementResumed::prevId() const
{
    return m_previd;
}

void QXmppStreamManagementResumed::setPrevId(const QString& previd)
{
    m_previd = previd;
}

bool QXmppStreamManagementResumed::isStreamManagementResumed(const QDomElement &element)
{
    return element.tagName() == QLatin1String("resumed") &&
           element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementResumed::parse(const QDomElement &element)
{
    m_h = element.attribute("h").toUInt();
    m_previd = element.attribute("previd");
}

void QXmppStreamManagementResumed::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("resumed");
    writer->writeAttribute("h", QString::number(m_h));
    writer->writeAttribute("previd", m_previd);
    writer->writeEndElement();
}

QXmppStreamManagementFailed::QXmppStreamManagementFailed(const QXmppStanza::Error::Condition error)
    : m_error(error)
{
}

QXmppStanza::Error::Condition QXmppStreamManagementFailed::error() const
{
    return m_error;
}

void QXmppStreamManagementFailed::setError(const QXmppStanza::Error::Condition error)
{
    m_error = error;
}

bool QXmppStreamManagementFailed::isStreamManagementFailed(const QDomElement &element)
{
    return element.tagName() == QLatin1String("failed") &&
           element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementFailed::parse(const QDomElement &element)
{
    QDomElement childElement = element.firstChildElement();
    if(!childElement.isNull() && childElement.namespaceURI() == ns_stanza) {
        m_error = conditionFromStr(childElement.tagName());
    }
}

void QXmppStreamManagementFailed::toXml(QXmlStreamWriter *writer) const
{
    QString errorString = strFromCondition(m_error);

    writer->writeStartElement("failed");
    writer->writeAttribute("xmlns", ns_stream_management);
    writer->writeStartElement(errorString, ns_stanza);
    writer->writeEndElement();
    writer->writeEndElement();
}

QXmppStreamManagementAck::QXmppStreamManagementAck(const unsigned seqNo)
    : m_seqNo(seqNo)
{
}

unsigned QXmppStreamManagementAck::seqNo() const
{
    return m_seqNo;
}

void QXmppStreamManagementAck::setSeqNo(const unsigned seqNo)
{
    m_seqNo = seqNo;
}

void QXmppStreamManagementAck::parse(const QDomElement &element)
{
    m_seqNo = element.attribute("h").toUInt();
}

void QXmppStreamManagementAck::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("a");
    writer->writeAttribute("xmlns", ns_stream_management);
    writer->writeAttribute("h", QString::number(m_seqNo));
    writer->writeEndElement();
}

bool QXmppStreamManagementAck::isStreamManagementAck(const QDomElement &element)
{
    return element.tagName() == QLatin1String("a") &&
           element.namespaceURI() == ns_stream_management;
}

bool QXmppStreamManagementReq::isStreamManagementReq(const QDomElement &element)
{
    return element.tagName() == QLatin1String("r") &&
           element.namespaceURI() == ns_stream_management;
}

void QXmppStreamManagementReq::toXml(QXmlStreamWriter *writer)
{
    writer->writeStartElement("r");
    writer->writeAttribute("xmlns", ns_stream_management);
    writer->writeEndElement();
}
