/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  James Turner (james.turner@kdab.com)
 *  Truphone Labs (labs@truphone.com)
 *
 * Source:
 *  https://github.com/trulabs/qxmpp/
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

#ifndef QXMPPSTREAMMANAGEMENT_H
#define QXMPPSTREAMMANAGEMENT_H

#include <qxmlstream.h>
#include "QXmppGlobal.h"
#include "QXmppLogger.h"
#include "QXmppMessage.h"
#include "QXmppPresence.h"
#include "QXmppIbbIq.h"
#include "QXmppStanza.h"
#include "QList"

class QXmppStreamManagementPrivate;


/// \brief The QXmppStreamManagement class manages Stream Management
/// as defined by XEP-0198: Stream Management.
/// \note Under development

class QXMPP_EXPORT QXmppStreamManagement : public QXmppLoggable
{
    Q_OBJECT
public:
    QXmppStreamManagement(QObject* parent = 0);

    void enableSent();
    void enabledReceived(const QDomElement &element);

    void disable();

    void stanzaSent(const QXmppStanza& stanza);
    void ackReceived(const QDomElement &element);

    void resumeSent();
    void resumedReceived();
    void failedReceived(const QDomElement &element, QXmppStanza::Error::Condition &condition);

    void stanzaHandled();

    void enableToXml(QXmlStreamWriter *xmlStream, const bool resume);
    void ackToXml(QXmlStreamWriter *xmlStream);
    void requestToXml(QXmlStreamWriter *xmlStream);
    void resumeToXml(QXmlStreamWriter *xmlStream);

    bool isEnabled() const;
    bool isOutboundEnabled() const;
    bool isInboundEnabled() const;
    bool isResumeEnabled() const;
    bool isResumming() const;

    int  outboundCounter() const;
    int  inboudCounter() const;

    QString resumeId() const;
    QString resumeLocation() const;

    QList<QXmppStanza *> outBoundBuffer() const;

    void socketDisconnected();

signals:
    /// This singal is emitted when the server AKC/NACK a message
    void messageAcknowledged(const QXmppMessage&, const bool);

    /// This singal is emitted when the server AKC/NACK a presence
    void presenceAcknowledged(const QXmppPresence&, const bool);

    /// This singal is emitted when the server AKC a iq
    void iqAcknowledged(const QXmppIq&, const bool);

private:
    QXmppStreamManagementPrivate * const d;

};

#endif // QXMPPSTREAMMANAGEMENT_H
