/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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


#include "QXmppPresence.h"
#include "QXmppUtils.h"
#include <QtDebug>
#include <QDomElement>
#include <QXmlStreamWriter>
#include "QXmppConstants.h"

QXmppPresence::QXmppPresence(QXmppPresence::Type type,
                             const QXmppPresence::Status& status)
    : QXmppStanza(), m_type(type), m_status(status)
{

}

QXmppPresence::~QXmppPresence()
{

}

QXmppPresence::Type QXmppPresence::type() const
{
    return m_type;
}

void QXmppPresence::setType(QXmppPresence::Type type)
{
    m_type = type;
}

const QXmppPresence::Status& QXmppPresence::status() const
{
    return m_status;
}

QXmppPresence::Status& QXmppPresence::status()
{
    return m_status;
}

void QXmppPresence::setStatus(const QXmppPresence::Status& status)
{
    m_status = status;
}

void QXmppPresence::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    setTypeFromStr(element.attribute("type"));
    m_status.parse(element);

    QXmppElementList extensions;
    QDomElement xElement = element.firstChildElement();
    m_vCardUpdateType = VCardUpdateNone;
    while(!xElement.isNull())
    {
        // XEP-0153: vCard-Based Avatars
        if(xElement.namespaceURI() == ns_vcard_update)
        {
            QDomElement photoElement = xElement.firstChildElement("photo");
            if(!photoElement.isNull())
            {
                m_photoHash = photoElement.text().toUtf8();
                if(m_photoHash.isEmpty())
                    m_vCardUpdateType = PhotoNotAdvertized;
                else
                    m_vCardUpdateType = PhotoAdvertised;
            }
            else
            {
                m_photoHash = "";
                m_vCardUpdateType = PhotoNotReady;
            }
        }
        // XEP-0115: Entity Capabilities
        else if(xElement.tagName() == "c" && xElement.namespaceURI() == ns_capabilities)
        {
            m_capabilityNode = xElement.attribute("node");
            m_capabilityVer = xElement.attribute("ver");
            m_capabilityHash = xElement.attribute("hash");
            m_capabilityExt = xElement.attribute("ext").split(" ", QString::SkipEmptyParts);
        }
        else if (xElement.tagName() == "error")
        {
        }
        else if (xElement.tagName() == "show")
        {
        }
        else if (xElement.tagName() == "status")
        {
        }
        else if (xElement.tagName() == "priority")
        {
        }
        else
        {
            // other extensions
            extensions << QXmppElement(xElement);
        }
        xElement = xElement.nextSiblingElement();
    }
    setExtensions(extensions);
}

void QXmppPresence::toXml(QXmlStreamWriter *xmlWriter) const
{
    xmlWriter->writeStartElement("presence");
    helperToXmlAddAttribute(xmlWriter,"xml:lang", lang());
    helperToXmlAddAttribute(xmlWriter,"id", id());
    helperToXmlAddAttribute(xmlWriter,"to", to());
    helperToXmlAddAttribute(xmlWriter,"from", from());
    helperToXmlAddAttribute(xmlWriter,"type", getTypeStr());
    m_status.toXml(xmlWriter);

    error().toXml(xmlWriter);

    // XEP-0153: vCard-Based Avatars
    if(m_vCardUpdateType != VCardUpdateNone)
    {
        xmlWriter->writeStartElement("x");
        helperToXmlAddAttribute(xmlWriter, "xmlns", ns_vcard_update);
        switch(m_vCardUpdateType)
        {
        case PhotoNotAdvertized:
            helperToXmlAddTextElement(xmlWriter, "photo", "");
            break;
        case PhotoAdvertised:
            helperToXmlAddTextElement(xmlWriter, "photo", m_photoHash);
            break;
        case PhotoNotReady:
            break;
        default:
            break;
        }
        xmlWriter->writeEndElement();
    }

    if(!m_capabilityNode.isEmpty() && !m_capabilityVer.isEmpty()
        && !m_capabilityHash.isEmpty())
    {
        xmlWriter->writeStartElement("c");
        helperToXmlAddAttribute(xmlWriter, "xmlns", ns_capabilities);
        helperToXmlAddAttribute(xmlWriter, "hash", m_capabilityHash);
        helperToXmlAddAttribute(xmlWriter, "node", m_capabilityNode);
        helperToXmlAddAttribute(xmlWriter, "ver", m_capabilityVer);
        xmlWriter->writeEndElement();
    }

    foreach (const QXmppElement &extension, extensions())
        extension.toXml(xmlWriter);

    xmlWriter->writeEndElement();
}

QString QXmppPresence::getTypeStr() const
{
    QString text;
    switch(m_type)
    {
    case QXmppPresence::Error:
        text = "error"; 
        break;
    case QXmppPresence::Available:
        // no type-attribute if available
        text = ""; 
        break;
    case QXmppPresence::Unavailable:
        text = "unavailable"; 
        break;
    case QXmppPresence::Subscribe:
        text = "subscribe"; 
        break;
    case QXmppPresence::Subscribed:
        text = "subscribed"; 
        break;
    case QXmppPresence::Unsubscribe:
        text = "unsubscribe"; 
        break;
    case QXmppPresence::Unsubscribed:
        text = "unsubscribed"; 
        break;
    case QXmppPresence::Probe:
        text = "probe"; 
        break;
    default:
        qWarning("QXmppPresence::getTypeStr() invalid type %d", (int)m_type);
        break;
    }
    return text;
}

void QXmppPresence::setTypeFromStr(const QString& str)
{
    QXmppPresence::Type type;
    if(str == "error")
    {
        type = QXmppPresence::Error;
        setType(type);
        return;
    }
    else if(str == "unavailable")
    {
        type = QXmppPresence::Unavailable;
        setType(type);
        return;
    }
    else if(str == "subscribe")
    {
        type = QXmppPresence::Subscribe;
        setType(type);
        return;
    }
    else if(str == "subscribed")
    {
        type = QXmppPresence::Subscribed;
        setType(type);
        return;
    }
    else if(str == "unsubscribe")
    {
        type = QXmppPresence::Unsubscribe;
        setType(type);
        return;
    }
    else if(str == "unsubscribed")
    {
        type = QXmppPresence::Unsubscribed;
        setType(type);
        return;
    }
    else if(str == "probe")
    {
        type = QXmppPresence::Probe;
        setType(type);
        return;
    }
    else if(str == "")
    {
        type = QXmppPresence::Available;
        setType(type);
        return;
    }
    else
    {
        type = static_cast<QXmppPresence::Type>(-1);
        qWarning("QXmppPresence::setTypeFromStr() invalid input string type: %s",
                 qPrintable(str));
        setType(type);
        return;
    }
}

QXmppPresence::Status::Status(QXmppPresence::Status::Type type,
                             const QString statusText, int priority) :
                                m_type(type),
                                m_statusText(statusText), m_priority(priority)
{
}

QXmppPresence::Status::Type QXmppPresence::Status::type() const
{
    return m_type;
}

void QXmppPresence::Status::setType(QXmppPresence::Status::Type type)
{
    m_type = type;
}

void QXmppPresence::Status::setTypeFromStr(const QString& str)
{
    // there is no keyword for Offline

    QXmppPresence::Status::Type type;
    if(str == "")   // not type-attribute means online
    {
        type = QXmppPresence::Status::Online;
        setType(type);
        return;
    }
    else if(str == "away")
    {
        type = QXmppPresence::Status::Away;
        setType(type);
        return;
    }
    else if(str == "xa")
    {
        type = QXmppPresence::Status::XA;
        setType(type);
        return;
    }
    else if(str == "dnd")
    {
        type = QXmppPresence::Status::DND;
        setType(type);
        return;
    }
    else if(str == "chat")
    {
        type = QXmppPresence::Status::Chat;
        setType(type);
        return;
    }
    else
    {
        type = static_cast<QXmppPresence::Status::Type>(-1);
        qWarning("QXmppPresence::Status::setTypeFromStr() invalid input string type %s", 
            qPrintable(str));
        setType(type);
    }
}

QString QXmppPresence::Status::getTypeStr() const
{
    QString text;
    switch(m_type)
    {
    case QXmppPresence::Status::Online:
        // no type-attribute if available
        text = ""; 
        break;
    case QXmppPresence::Status::Offline:
        text = ""; 
        break;
    case QXmppPresence::Status::Away:
        text = "away"; 
        break;
    case QXmppPresence::Status::XA:
        text = "xa"; 
        break;
    case QXmppPresence::Status::DND:
        text = "dnd"; 
        break;
    case QXmppPresence::Status::Chat:
        text = "chat"; 
        break;
    default:
        qWarning("QXmppPresence::Status::getTypeStr() invalid type %d",
                 (int)m_type);
        break;
    }
    return text;
}

QString QXmppPresence::Status::statusText() const
{
    return m_statusText;
}

void QXmppPresence::Status::setStatusText(const QString& str)
{
    m_statusText = str;
}

int QXmppPresence::Status::priority() const
{
    return m_priority;
}

void QXmppPresence::Status::setPriority(int priority)
{
    m_priority = priority;
}

void QXmppPresence::Status::parse(const QDomElement &element)
{
    setTypeFromStr(element.firstChildElement("show").text());
    m_statusText = element.firstChildElement("status").text();
    m_priority = element.firstChildElement("priority").text().toInt();
}

void QXmppPresence::Status::toXml(QXmlStreamWriter *xmlWriter) const
{
    const QString show = getTypeStr();
    if (!show.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "show", getTypeStr());
    if (!m_statusText.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "status", m_statusText);
    if (m_priority != 0)
        helperToXmlAddNumberElement(xmlWriter, "priority", m_priority);
}

QByteArray QXmppPresence::photoHash() const
{
    return m_photoHash;
}

void QXmppPresence::setPhotoHash(const QByteArray& photoHash)
{
    m_photoHash = photoHash;
}

QXmppPresence::VCardUpdateType QXmppPresence::vCardUpdateType()
{
    return m_vCardUpdateType;
}

void QXmppPresence::setVCardUpdateType(VCardUpdateType type)
{
    m_vCardUpdateType = type;
}

/// XEP-0115: Entity Capabilities
QString QXmppPresence::capabilityHash()
{
    return m_capabilityHash;
}

/// XEP-0115: Entity Capabilities
void QXmppPresence::setCapabilityHash(const QString& hash)
{
    m_capabilityHash = hash;
}

/// XEP-0115: Entity Capabilities
QString QXmppPresence::capabilityNode()
{
    return m_capabilityNode;
}

/// XEP-0115: Entity Capabilities
void QXmppPresence::setCapabilityNode(const QString& node)
{
    m_capabilityNode = node;
}

/// XEP-0115: Entity Capabilities
QString QXmppPresence::capabilityVer()
{
    return m_capabilityVer;
}

/// XEP-0115: Entity Capabilities
void QXmppPresence::setCapabilityVer(const QString& ver)
{
    m_capabilityVer = ver;
}

QStringList QXmppPresence::capabilityExt()
{
    return m_capabilityExt;
}

/// \cond

QXmppPresence::Type QXmppPresence::getType() const
{
    return m_type;
}

const QXmppPresence::Status& QXmppPresence::getStatus() const
{
    return m_status;
}

QXmppPresence::Status& QXmppPresence::getStatus()
{
    return m_status;
}

QXmppPresence::Status::Type QXmppPresence::Status::getType() const
{
    return m_type;
}

QString QXmppPresence::Status::getStatusText() const
{
    return m_statusText;
}

int QXmppPresence::Status::getPriority() const
{
    return m_priority;
}

/// \endcond
