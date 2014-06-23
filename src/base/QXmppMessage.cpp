/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
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
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QPair>

#include "QXmppConstants.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

static const char* chat_states[] = {
    "",
    "active",
    "inactive",
    "gone",
    "composing",
    "paused",
};

static const char* message_types[] = {
    "error",
    "normal",
    "chat",
    "groupchat",
    "headline"
};

static const char* marker_types[] = {
    "",
    "received",
    "displayed",
    "acknowledged"
};

static const char *ns_xhtml = "http://www.w3.org/1999/xhtml";

enum StampType
{
    LegacyDelayedDelivery,  // XEP-0091: Legacy Delayed Delivery
    DelayedDelivery,        // XEP-0203: Delayed Delivery
};

class QXmppMessagePrivate : public QSharedData
{
public:
    QXmppMessage::Type type;
    QDateTime stamp;
    StampType stampType;
    QXmppMessage::State state;

    bool attentionRequested;
    QString body;
    QString subject;
    QString thread;

    // XEP-0071: XHTML-IM
    QString xhtml;

    // Request message receipt as per XEP-0184.
    QString receiptId;
    bool receiptRequested;

    // XEP-0249: Direct MUC Invitations
    QString mucInvitationJid;
    QString mucInvitationPassword;
    QString mucInvitationReason;

    // XEP-0333: Chat Markers
    bool markable;
    QXmppMessage::Marker marker;
    QString markedId;
    QString markedThread;
};

/// Constructs a QXmppMessage.
///
/// \param from
/// \param to
/// \param body
/// \param thread

QXmppMessage::QXmppMessage(const QString& from, const QString& to, const
                         QString& body, const QString& thread)
    : QXmppStanza(from, to)
    , d(new QXmppMessagePrivate)
{
    d->type = Chat;
    d->stampType = DelayedDelivery;
    d->state = None;
    d->attentionRequested = false;
    d->body = body;
    d->thread = thread;
    d->receiptRequested = false;

    d->markable = false;
    d->marker = NoMarker;
}

/// Constructs a copy of \a other.

QXmppMessage::QXmppMessage(const QXmppMessage &other)
    : QXmppStanza(other)
    , d(other.d)
{
}

QXmppMessage::~QXmppMessage()
{

}

/// Assigns \a other to this message.

QXmppMessage& QXmppMessage::operator=(const QXmppMessage &other)
{
    QXmppStanza::operator=(other);
    d = other.d;
    return *this;
}

/// Returns the message's body.
///

QString QXmppMessage::body() const
{
    return d->body;
}

/// Sets the message's body.
///
/// \param body

void QXmppMessage::setBody(const QString& body)
{
    d->body = body;
}

/// Returns true if the user's attention is requested, as defined
/// by XEP-0224: Attention.

bool QXmppMessage::isAttentionRequested() const
{
    return d->attentionRequested;
}

/// Sets whether the user's attention is requested, as defined
/// by XEP-0224: Attention.
///
/// \a param requested

void QXmppMessage::setAttentionRequested(bool requested)
{
    d->attentionRequested = requested;
}

/// Returns true if a delivery receipt is requested, as defined
/// by XEP-0184: Message Delivery Receipts.

bool QXmppMessage::isReceiptRequested() const
{
    return d->receiptRequested;
}

/// Sets whether a delivery receipt is requested, as defined
/// by XEP-0184: Message Delivery Receipts.
///
/// \a param requested

void QXmppMessage::setReceiptRequested(bool requested)
{
    d->receiptRequested = requested;
    if (requested && id().isEmpty())
        generateAndSetNextId();
}

/// If this message is a delivery receipt, returns the ID of the
/// original message.

QString QXmppMessage::receiptId() const
{
    return d->receiptId;
}

/// Make this message a delivery receipt for the message with
/// the given \a id.

void QXmppMessage::setReceiptId(const QString &id)
{
    d->receiptId = id;
}

/// Returns the JID for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

QString QXmppMessage::mucInvitationJid() const
{
    return d->mucInvitationJid;
}

/// Sets the JID for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

void QXmppMessage::setMucInvitationJid(const QString &jid)
{
    d->mucInvitationJid = jid;
}

/// Returns the password for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

QString QXmppMessage::mucInvitationPassword() const
{
    return d->mucInvitationPassword;
}

/// Sets the \a password for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

void QXmppMessage::setMucInvitationPassword(const QString &password)
{
    d->mucInvitationPassword = password;
}

/// Returns the reason for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

QString QXmppMessage::mucInvitationReason() const
{
    return d->mucInvitationReason;
}

/// Sets the \a reason for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

void QXmppMessage::setMucInvitationReason(const QString &reason)
{
    d->mucInvitationReason = reason;
}

/// Returns the message's type.
///

QXmppMessage::Type QXmppMessage::type() const
{
    return d->type;
}

/// Sets the message's type.
///
/// \param type

void QXmppMessage::setType(QXmppMessage::Type type)
{
    d->type = type;
}

/// Returns the message's timestamp (if any).

QDateTime QXmppMessage::stamp() const
{
    return d->stamp;
}

/// Sets the message's timestamp.
///
/// \param stamp

void QXmppMessage::setStamp(const QDateTime &stamp)
{
    d->stamp = stamp;
}

/// Returns the message's chat state.
///

QXmppMessage::State QXmppMessage::state() const
{
    return d->state;
}

/// Sets the message's chat state.
///
/// \param state

void QXmppMessage::setState(QXmppMessage::State state)
{
    d->state = state;
}

/// Returns the message's subject.
///

QString QXmppMessage::subject() const
{
    return d->subject;
}

/// Sets the message's subject.
///
/// \param subject

void QXmppMessage::setSubject(const QString& subject)
{
    d->subject = subject;
}

/// Returns the message's thread.

QString QXmppMessage::thread() const
{
    return d->thread;
}

/// Sets the message's thread.
///
/// \param thread

void QXmppMessage::setThread(const QString& thread)
{
    d->thread = thread;
}

/// Returns the message's XHTML body as defined by
/// XEP-0071: XHTML-IM.

QString QXmppMessage::xhtml() const
{
    return d->xhtml;
}

/// Sets the message's XHTML body as defined by
/// XEP-0071: XHTML-IM.

void QXmppMessage::setXhtml(const QString &xhtml)
{
    d->xhtml = xhtml;
}

namespace
{
    static QList<QPair<QString, QString> > knownMessageSubelems()
    {
        QList<QPair<QString, QString> > result;
        result << qMakePair(QString("body"), QString())
               << qMakePair(QString("subject"), QString())
               << qMakePair(QString("thread"), QString())
               << qMakePair(QString("html"), QString())
               << qMakePair(QString("received"), QString(ns_message_receipts))
               << qMakePair(QString("request"), QString())
               << qMakePair(QString("delay"), QString())
               << qMakePair(QString("attention"), QString())
               << qMakePair(QString("addresses"), QString());
        for (int i = QXmppMessage::Active; i <= QXmppMessage::Paused; i++)
            result << qMakePair(QString(chat_states[i]), QString());
        return result;
    }
}

/// Returns true if a message is markable, as defined
/// XEP-0333: Chat Markers.

bool QXmppMessage::isMarkable() const
{
    return d->markable;
}

/// Sets if the message is markable, as defined
/// XEP-0333: Chat Markers.

void QXmppMessage::setMarkable(const bool markable)
{
    d->markable = markable;
}

/// Returns the message's marker id, as defined
/// XEP-0333: Chat Markers.

QString QXmppMessage::markedId() const
{
    return d->markedId;
}

/// Sets the message's marker id, as defined
/// XEP-0333: Chat Markers.

void QXmppMessage::setMarkerId(const QString &markerId)
{
    d->markedId = markerId;
}

/// Returns the message's marker thread, as defined
/// XEP-0333: Chat Markers.

QString QXmppMessage::markedThread() const
{
    return d->markedThread;
}

/// Sets the message's marked thread, as defined
/// XEP-0333: Chat Markers.

void QXmppMessage::setMarkedThread(const QString &markedThread)
{
    d->markedThread = markedThread;
}

/// Returns the message's marker, as defined
/// XEP-0333: Chat Markers.

QXmppMessage::Marker QXmppMessage::marker() const
{
    return d->marker;
}

/// Sets the message's marker, as defined
/// XEP-0333: Chat Markers

void QXmppMessage::setMarker(const Marker marker)
{
    d->marker = marker;
}

/// \cond
void QXmppMessage::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    const QString type = element.attribute("type");
    d->type = Normal;
    for (int i = Error; i <= Headline; i++) {
        if (type == message_types[i]) {
            d->type = static_cast<Type>(i);
            break;
        }
    }

    d->body = element.firstChildElement("body").text();
    d->subject = element.firstChildElement("subject").text();
    d->thread = element.firstChildElement("thread").text();

    // chat states
    for (int i = Active; i <= Paused; i++)
    {
        QDomElement stateElement = element.firstChildElement(chat_states[i]);
        if (!stateElement.isNull() &&
            stateElement.namespaceURI() == ns_chat_states)
        {
            d->state = static_cast<QXmppMessage::State>(i);
            break;
        }
    }

    // XEP-0071: XHTML-IM
    QDomElement htmlElement = element.firstChildElement("html");
    if (!htmlElement.isNull() && htmlElement.namespaceURI() == ns_xhtml_im) {
        QDomElement bodyElement = htmlElement.firstChildElement("body");
        if (!bodyElement.isNull() && bodyElement.namespaceURI() == ns_xhtml) {
            QTextStream stream(&d->xhtml, QIODevice::WriteOnly);
            bodyElement.save(stream, 0);

            d->xhtml = d->xhtml.mid(d->xhtml.indexOf('>') + 1);
            d->xhtml.replace(" xmlns=\"http://www.w3.org/1999/xhtml\"", "");
            d->xhtml.replace("</body>", "");
            d->xhtml = d->xhtml.trimmed();
        }
    }

    // XEP-0184: Message Delivery Receipts
    QDomElement receivedElement = element.firstChildElement("received");
    if (!receivedElement.isNull() && receivedElement.namespaceURI() == ns_message_receipts) {
        d->receiptId = receivedElement.attribute("id");

        // compatibility with old-style XEP
        if (d->receiptId.isEmpty())
            d->receiptId = id();
    } else {
        d->receiptId = QString();
    }
    d->receiptRequested = element.firstChildElement("request").namespaceURI() == ns_message_receipts;

    // XEP-0203: Delayed Delivery
    QDomElement delayElement = element.firstChildElement("delay");
    if (!delayElement.isNull() && delayElement.namespaceURI() == ns_delayed_delivery)
    {
        const QString str = delayElement.attribute("stamp");
        d->stamp = QXmppUtils::datetimeFromString(str);
        d->stampType = DelayedDelivery;
    }

    // XEP-0224: Attention
    d->attentionRequested = element.firstChildElement("attention").namespaceURI() == ns_attention;

    // XEP-0333: Chat Markers
    QDomElement markableElement = element.firstChildElement("markable");
    if (!markableElement.isNull())
    {
        d->markable = true;
    }
    // check for all the marker types
    QDomElement chatStateElement;
    QXmppMessage::Marker marker = QXmppMessage::NoMarker;
    for (int i = Received; i <= Acknowledged; i++)
    {
        chatStateElement = element.firstChildElement(marker_types[i]);
        if (!chatStateElement.isNull() &&
            chatStateElement.namespaceURI() == ns_chat_markers)
        {
            marker = static_cast<QXmppMessage::Marker>(i);
            break;
        }
    }
    // if marker is present, check it's the right ns
    if (!chatStateElement.isNull())
    {
        if (chatStateElement.namespaceURI() == ns_chat_markers)
        {
            d->marker = marker;
            d->markedId = chatStateElement.attribute("id", QString());
            d->markedThread = chatStateElement.attribute("thread", QString());
        }
    }

    const QList<QPair<QString, QString> > &knownElems = knownMessageSubelems();

    QXmppElementList extensions;
    QDomElement xElement = element.firstChildElement();
    while (!xElement.isNull())
    {
        if (xElement.tagName() == "x")
        {
            if (xElement.namespaceURI() == ns_legacy_delayed_delivery)
            {
                // XEP-0091: Legacy Delayed Delivery
                const QString str = xElement.attribute("stamp");
                d->stamp = QDateTime::fromString(str, "yyyyMMddThh:mm:ss");
                d->stamp.setTimeSpec(Qt::UTC);
                d->stampType = LegacyDelayedDelivery;
            } else if (xElement.namespaceURI() == ns_conference) {
                // XEP-0249: Direct MUC Invitations
                d->mucInvitationJid = xElement.attribute("jid");
                d->mucInvitationPassword = xElement.attribute("password");
                d->mucInvitationReason = xElement.attribute("reason");
            }
            else {
                extensions << QXmppElement(xElement);
            }
        } else if (!knownElems.contains(qMakePair(xElement.tagName(), xElement.namespaceURI())) &&
                   !knownElems.contains(qMakePair(xElement.tagName(), QString()))) {
            // other extensions
            extensions << QXmppElement(xElement);
        }
        xElement = xElement.nextSiblingElement();
    }
    setExtensions(extensions);
}

void QXmppMessage::toXml(QXmlStreamWriter *xmlWriter) const
{
    xmlWriter->writeStartElement("message");
    helperToXmlAddAttribute(xmlWriter, "xml:lang", lang());
    helperToXmlAddAttribute(xmlWriter, "id", id());
    helperToXmlAddAttribute(xmlWriter, "to", to());
    helperToXmlAddAttribute(xmlWriter, "from", from());
    helperToXmlAddAttribute(xmlWriter, "type", message_types[d->type]);
    if (!d->subject.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "subject", d->subject);
    if (!d->body.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "body", d->body);
    if (!d->thread.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "thread", d->thread);
    error().toXml(xmlWriter);

    // chat states
    if (d->state > None && d->state <= Paused)
    {
        xmlWriter->writeStartElement(chat_states[d->state]);
        xmlWriter->writeAttribute("xmlns", ns_chat_states);
        xmlWriter->writeEndElement();
    }

    // XEP-0071: XHTML-IM
    if (!d->xhtml.isEmpty()) {
        xmlWriter->writeStartElement("html");
        xmlWriter->writeAttribute("xmlns", ns_xhtml_im);
        xmlWriter->writeStartElement("body");
        xmlWriter->writeAttribute("xmlns", ns_xhtml);
        xmlWriter->writeCharacters("");
        xmlWriter->device()->write(d->xhtml.toUtf8());
        xmlWriter->writeEndElement();
        xmlWriter->writeEndElement();
    }

    // time stamp
    if (d->stamp.isValid())
    {
        QDateTime utcStamp = d->stamp.toUTC();
        if (d->stampType == DelayedDelivery)
        {
            // XEP-0203: Delayed Delivery
            xmlWriter->writeStartElement("delay");
            xmlWriter->writeAttribute("xmlns", ns_delayed_delivery);
            helperToXmlAddAttribute(xmlWriter, "stamp", QXmppUtils::datetimeToString(utcStamp));
            xmlWriter->writeEndElement();
        } else {
            // XEP-0091: Legacy Delayed Delivery
            xmlWriter->writeStartElement("x");
            xmlWriter->writeAttribute("xmlns", ns_legacy_delayed_delivery);
            helperToXmlAddAttribute(xmlWriter, "stamp", utcStamp.toString("yyyyMMddThh:mm:ss"));
            xmlWriter->writeEndElement();
        }
    }

    // XEP-0184: Message Delivery Receipts
    if (!d->receiptId.isEmpty()) {
        xmlWriter->writeStartElement("received");
        xmlWriter->writeAttribute("xmlns", ns_message_receipts);
        xmlWriter->writeAttribute("id", d->receiptId);
        xmlWriter->writeEndElement();
    }
    if (d->receiptRequested) {
        xmlWriter->writeStartElement("request");
        xmlWriter->writeAttribute("xmlns", ns_message_receipts);
        xmlWriter->writeEndElement();
    }

    // XEP-0224: Attention
    if (d->attentionRequested) {
        xmlWriter->writeStartElement("attention");
        xmlWriter->writeAttribute("xmlns", ns_attention);
        xmlWriter->writeEndElement();
    }

    // XEP-0249: Direct MUC Invitations
    if (!d->mucInvitationJid.isEmpty()) {
        xmlWriter->writeStartElement("x");
        xmlWriter->writeAttribute("xmlns", ns_conference);
        xmlWriter->writeAttribute("jid", d->mucInvitationJid);
        if (!d->mucInvitationPassword.isEmpty())
            xmlWriter->writeAttribute("password", d->mucInvitationPassword);
        if (!d->mucInvitationReason.isEmpty())
            xmlWriter->writeAttribute("reason", d->mucInvitationReason);
        xmlWriter->writeEndElement();
    }

    // XEP-0333: Chat Markers
    if (d->markable) {
        xmlWriter->writeStartElement("markable");
        xmlWriter->writeAttribute("xmlns", ns_chat_markers);
        xmlWriter->writeEndElement();
    }
    if (d->marker != NoMarker) {
        xmlWriter->writeStartElement(marker_types[d->marker]);
        xmlWriter->writeAttribute("xmlns", ns_chat_markers);
        xmlWriter->writeAttribute("id", d->markedId);
        if (!d->markedThread.isNull() && !d->markedThread.isEmpty()) {
            xmlWriter->writeAttribute("thread", d->markedThread);
        }
        xmlWriter->writeEndElement();
    }

    // other extensions
    QXmppStanza::extensionsToXml(xmlWriter);

    xmlWriter->writeEndElement();
}
/// \endcond
