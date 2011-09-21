/*
 * Copyright (C) 2008-2011 The QXmpp developers
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

/// Constructs a QXmppPresence.
///
/// \param type
/// \param status

QXmppPresence::QXmppPresence(QXmppPresence::Type type,
                             const QXmppPresence::Status& status)
    : QXmppStanza(),
    m_type(type),
    m_status(status),
    m_vCardUpdateType(VCardUpdateNone)
{

}

/// Destroys a QXmppPresence.

QXmppPresence::~QXmppPresence()
{

}

/// Returns the presence type.
///
/// You can use this method to determine the action which needs to be
/// taken in response to receiving the presence. For instance, if the type is
/// QXmppPresence::Available or QXmppPresence::Unavailable, you could update
/// the icon representing a contact's availability.

QXmppPresence::Type QXmppPresence::type() const
{
    return m_type;
}

/// Sets the presence type.
///
/// \param type

void QXmppPresence::setType(QXmppPresence::Type type)
{
    m_type = type;
}

/// Returns the presence status.

const QXmppPresence::Status& QXmppPresence::status() const
{
    return m_status;
}

/// Returns a reference to the presence status, allowing you to change it.

QXmppPresence::Status& QXmppPresence::status()
{
    return m_status;
}

/// Sets the presence status.
///
/// \param status

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
        // XEP-0045: Multi-User Chat
        if(xElement.namespaceURI() == ns_muc_user)
        {
            QDomElement itemElement = xElement.firstChildElement("item");
            m_mucItem.parse(itemElement);
            QDomElement statusElement = xElement.firstChildElement("status");
            m_mucStatusCodes.clear();
            while (!statusElement.isNull()) {
                m_mucStatusCodes << statusElement.attribute("code").toInt();
                statusElement = statusElement.nextSiblingElement("status");
            }
        }
        // XEP-0153: vCard-Based Avatars
        else if(xElement.namespaceURI() == ns_vcard_update)
        {
            QDomElement photoElement = xElement.firstChildElement("photo");
            if(!photoElement.isNull())
            {
                m_photoHash = QByteArray::fromHex(photoElement.text().toAscii());
                if(m_photoHash.isEmpty())
                    m_vCardUpdateType = VCardUpdateNoPhoto;
                else
                    m_vCardUpdateType = VCardUpdateValidPhoto;
            }
            else
            {
                m_photoHash = QByteArray();
                m_vCardUpdateType = VCardUpdateNotReady;
            }
        }
        // XEP-0115: Entity Capabilities
        else if(xElement.tagName() == "c" && xElement.namespaceURI() == ns_capabilities)
        {
            m_capabilityNode = xElement.attribute("node");
            m_capabilityVer = QByteArray::fromBase64(xElement.attribute("ver").toAscii());
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

    // XEP-0045: Multi-User Chat
    if(!m_mucItem.isNull() || !m_mucStatusCodes.isEmpty())
    {
        xmlWriter->writeStartElement("x");
        xmlWriter->writeAttribute("xmlns", ns_muc_user);
        if (!m_mucItem.isNull())
            m_mucItem.toXml(xmlWriter);
        foreach (int code, m_mucStatusCodes) {
            xmlWriter->writeStartElement("status");
            xmlWriter->writeAttribute("code", QString::number(code));
            xmlWriter->writeEndElement();
        }
        xmlWriter->writeEndElement();
    }

    // XEP-0153: vCard-Based Avatars
    if(m_vCardUpdateType != VCardUpdateNone)
    {
        xmlWriter->writeStartElement("x");
        xmlWriter->writeAttribute("xmlns", ns_vcard_update);
        switch(m_vCardUpdateType)
        {
        case VCardUpdateNoPhoto:
            helperToXmlAddTextElement(xmlWriter, "photo", "");
            break;
        case VCardUpdateValidPhoto:
            helperToXmlAddTextElement(xmlWriter, "photo", m_photoHash.toHex());
            break;
        case VCardUpdateNotReady:
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
        xmlWriter->writeAttribute("xmlns", ns_capabilities);
        helperToXmlAddAttribute(xmlWriter, "hash", m_capabilityHash);
        helperToXmlAddAttribute(xmlWriter, "node", m_capabilityNode);
        helperToXmlAddAttribute(xmlWriter, "ver", m_capabilityVer.toBase64());
        xmlWriter->writeEndElement();
    }

    foreach (const QXmppElement &extension, extensions())
        extension.toXml(xmlWriter);

    xmlWriter->writeEndElement();
}

QString QXmppPresence::getTypeStr() const
{
    switch(m_type) {
    case QXmppPresence::Error:
        return "error";
    case QXmppPresence::Available:
        return "";
    case QXmppPresence::Unavailable:
        return "unavailable";
    case QXmppPresence::Subscribe:
        return "subscribe";
    case QXmppPresence::Subscribed:
        return "subscribed";
    case QXmppPresence::Unsubscribe:
        return "unsubscribe";
    case QXmppPresence::Unsubscribed:
        return "unsubscribed";
    case QXmppPresence::Probe:
        return "probe";
    default:
        qWarning("QXmppPresence::getTypeStr() invalid type %d", (int)m_type);
        return "";
    }
}

void QXmppPresence::setTypeFromStr(const QString& str)
{
    if(str == "error")
        m_type = QXmppPresence::Error;
    else if(str == "")
        m_type = QXmppPresence::Available;
    else if(str == "unavailable")
        m_type = QXmppPresence::Unavailable;
    else if(str == "subscribe")
        m_type = QXmppPresence::Subscribe;
    else if(str == "subscribed")
        m_type = QXmppPresence::Subscribed;
    else if(str == "unsubscribe")
        m_type = QXmppPresence::Unsubscribe;
    else if(str == "unsubscribed")
        m_type = QXmppPresence::Unsubscribed;
    else if(str == "probe")
        m_type = QXmppPresence::Probe;
    else {
        qWarning("QXmppPresence::setTypeFromStr() invalid input string type: %s",
                 qPrintable(str));
        m_type = QXmppPresence::Error;
    }
}

/// Constructs a presence status.

QXmppPresence::Status::Status(QXmppPresence::Status::Type type,
                             const QString statusText, int priority) :
                                m_type(type),
                                m_statusText(statusText), m_priority(priority)
{
}

/// Returns the status type, for instance busy or away.

QXmppPresence::Status::Type QXmppPresence::Status::type() const
{
    return m_type;
}

/// Sets the status type.

void QXmppPresence::Status::setType(QXmppPresence::Status::Type type)
{
    m_type = type;
}

void QXmppPresence::Status::setTypeFromStr(const QString& str)
{
    // FIXME: there is no keyword for Offline
    if(str == "")
        m_type = QXmppPresence::Status::Online;
    else if(str == "away")
        m_type = QXmppPresence::Status::Away;
    else if(str == "chat")
        m_type = QXmppPresence::Status::Chat;
    else if(str == "dnd")
        m_type = QXmppPresence::Status::DND;
    else if(str == "xa")
        m_type = QXmppPresence::Status::XA;
    else {
        qWarning("QXmppPresence::Status::setTypeFromStr() invalid input string type %s",
            qPrintable(str));
        m_type = QXmppPresence::Status::Online;
    }
}

QString QXmppPresence::Status::getTypeStr() const
{
    switch(m_type) {
    case QXmppPresence::Status::Online:
        return "";
    case QXmppPresence::Status::Offline:
        // FIXME: there is no keyword for Offline
        return "";
    case QXmppPresence::Status::Away:
        return "away";
    case QXmppPresence::Status::XA:
        return "xa";
    case QXmppPresence::Status::DND:
        return "dnd";
    case QXmppPresence::Status::Chat:
        return "chat";
    default:
        qWarning("QXmppPresence::Status::getTypeStr() invalid type %d",
                 (int)m_type);
        return "";
    }
}

/// Returns the status text, a textual description of the user's status.

QString QXmppPresence::Status::statusText() const
{
    return m_statusText;
}

/// Sets the status text, a textual description of the user's status.
///
/// \param str The status text, for example "Gone fishing".

void QXmppPresence::Status::setStatusText(const QString& str)
{
    m_statusText = str;
}

/// Returns the priority level of the resource.

int QXmppPresence::Status::priority() const
{
    return m_priority;
}

/// Sets the priority level of the resource.
///
/// \param priority

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
        helperToXmlAddTextElement(xmlWriter, "priority", QString::number(m_priority));
}

/// Returns the photo-hash of the VCardUpdate.
///
/// \return QByteArray

QByteArray QXmppPresence::photoHash() const
{
    return m_photoHash;
}

/// Sets the photo-hash of the VCardUpdate.
///
/// \param photoHash as QByteArray

void QXmppPresence::setPhotoHash(const QByteArray& photoHash)
{
    m_photoHash = photoHash;
}

/// Returns the type of VCardUpdate
///
/// \return VCardUpdateType

QXmppPresence::VCardUpdateType QXmppPresence::vCardUpdateType() const
{
    return m_vCardUpdateType;
}

/// Sets the type of VCardUpdate
///
/// \param type VCardUpdateType

void QXmppPresence::setVCardUpdateType(VCardUpdateType type)
{
    m_vCardUpdateType = type;
}

/// XEP-0115: Entity Capabilities
QString QXmppPresence::capabilityHash() const
{
    return m_capabilityHash;
}

/// XEP-0115: Entity Capabilities
void QXmppPresence::setCapabilityHash(const QString& hash)
{
    m_capabilityHash = hash;
}

/// XEP-0115: Entity Capabilities
QString QXmppPresence::capabilityNode() const
{
    return m_capabilityNode;
}

/// XEP-0115: Entity Capabilities
void QXmppPresence::setCapabilityNode(const QString& node)
{
    m_capabilityNode = node;
}

/// XEP-0115: Entity Capabilities
QByteArray QXmppPresence::capabilityVer() const
{
    return m_capabilityVer;
}

/// XEP-0115: Entity Capabilities
void QXmppPresence::setCapabilityVer(const QByteArray& ver)
{
    m_capabilityVer = ver;
}

/// Legacy XEP-0115: Entity Capabilities
QStringList QXmppPresence::capabilityExt() const
{
    return m_capabilityExt;
}

/// Returns the MUC item.

QXmppMucItem QXmppPresence::mucItem() const
{
    return m_mucItem;
}

/// Sets the MUC item.
///
/// \param item

void QXmppPresence::setMucItem(const QXmppMucItem &item)
{
    m_mucItem = item;
}

/// Returns the MUC status codes.

QList<int> QXmppPresence::mucStatusCodes() const
{
    return m_mucStatusCodes;
}

/// Sets the MUC status codes.
///
/// \param codes

void QXmppPresence::setMucStatusCodes(const QList<int> &codes)
{
    m_mucStatusCodes = codes;
}

