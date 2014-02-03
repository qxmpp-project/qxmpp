/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Authors:
 *  Juan Aragon
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

#include <QObject>
#include "util.h"
#include "QXmppConstants.h"
#include "QXmppStreamManagement.h"

#include <QDebug>

class tst_QXmppStreamManagement : public QObject
{
    Q_OBJECT
public slots:
   void messageACKReceived(const QXmppMessage& message, bool ack);
   void iqACKReceived(const QXmppIq& iq, bool ack);
   void presenceACKReceived(const QXmppPresence& presence,  bool ack);

private slots:
    void initStreamManagement();
    void testEnableStreamManagement();
    void testRequestStreamManagement();
    void testAckStreamManagement();

private:
    QXmppStreamManagement *streamManagement;
    
};

void tst_QXmppStreamManagement::initStreamManagement()
{
    streamManagement = new QXmppStreamManagement(this);
    bool connected = false;
    Q_UNUSED(connected);
    connected = connect(streamManagement, SIGNAL(messageAcknowledged(QXmppMessage, bool)), this, SLOT(messageACKReceived(QXmppMessage, bool)));
    Q_ASSERT(connected);
    connected = connect(streamManagement, SIGNAL(iqAcknowledged(QXmppIq, bool)), this, SLOT(iqACKReceived(QXmppIq, bool)));
    Q_ASSERT(connected);
    connected = connect(streamManagement, SIGNAL(presenceAcknowledged(QXmppPresence, bool)), this, SLOT(presenceACKReceived(QXmppPresence, bool)));
    Q_ASSERT(connected);
}

void tst_QXmppStreamManagement::testEnableStreamManagement()
{
    //test
    const QByteArray xml("<enable xmlns=\"urn:xmpp:sm:3\"/>");
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    streamManagement->enableToXml(&writer,false);
    qDebug() << "expect " << xml;
    qDebug() << "writing" << buffer.data();
    QCOMPARE(buffer.data(), xml);
}


void tst_QXmppStreamManagement::testRequestStreamManagement ()
{
    const QByteArray xml("<a xmlns=\"urn:xmpp:sm:3\" h=\"0\"/>");
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    streamManagement->ackToXml(&writer);
    qDebug() << "expect " << xml;
    qDebug() << "writing" << buffer.data();
    QCOMPARE(buffer.data(), xml);
}

void tst_QXmppStreamManagement::testAckStreamManagement()
{
    const QByteArray xml("<r xmlns=\"urn:xmpp:sm:3\"/>");
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    streamManagement->requestToXml(&writer);
    qDebug() << "expect " << xml;
    qDebug() << "writing" << buffer.data();
    QCOMPARE(buffer.data(), xml);
}

void tst_QXmppStreamManagement::messageACKReceived(const QXmppMessage& message, bool ack)
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    message.toXml(&writer);
    qDebug() << "expect " << buffer.data();
}

void tst_QXmppStreamManagement::iqACKReceived(const QXmppIq& iq, bool ack)
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    iq.toXml(&writer);
    qDebug() << "expect " << buffer.data();
}

void tst_QXmppStreamManagement::presenceACKReceived(const QXmppPresence& presence, bool ack)
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    presence.toXml(&writer);
    qDebug() << "expect " << buffer.data();
}



QTEST_MAIN(tst_QXmppStreamManagement)
#include "tst_qxmppstreammanagement.moc"
