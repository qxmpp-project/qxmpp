/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *	Ian Reinhart Geiser
 *  Jeremy Lainé
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
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

#include <cstdlib>
#include <cstdio>

#include <QAudioInput>
#include <QAudioOutput>
#include <QCoreApplication>
#include <QDebug>
#include <QHostInfo>

#include "QXmppCallManager.h"
#include "QXmppJingleIq.h"
#include "QXmppRtpChannel.h"
#include "QXmppUtils.h"

#include "example_4_callHandling.h"

xmppClient::xmppClient(QObject *parent)
    : QXmppClient(parent)
    , m_turnPort(0)
    , m_turnFinished(false)
{
    bool check;
    Q_UNUSED(check);

    // add QXmppCallManager extension
    callManager = new QXmppCallManager;
    addExtension(callManager);

    check = connect(this, SIGNAL(connected()),
                    this, SLOT(slotConnected()));
    Q_ASSERT(check);

    check = connect(this, SIGNAL(presenceReceived(QXmppPresence)),
                    this, SLOT(slotPresenceReceived(QXmppPresence)));
    Q_ASSERT(check);

    check = connect(callManager, SIGNAL(callReceived(QXmppCall*)),
                    this, SLOT(slotCallReceived(QXmppCall*)));
    Q_ASSERT(check);

    check = connect(&m_dns, SIGNAL(finished()),
                    this, SLOT(slotDnsLookupFinished()));
    Q_ASSERT(check);
}

void xmppClient::setRecipient(const QString &recipient)
{
    m_recipient = recipient;
}

/// The audio mode of a call changed.

void xmppClient::slotAudioModeChanged(QIODevice::OpenMode mode)
{
    QXmppCall *call = qobject_cast<QXmppCall*>(sender());
    Q_ASSERT(call);
    QXmppRtpAudioChannel *channel = call->audioChannel();

    // prepare audio format
    QAudioFormat format;
    format.setFrequency(channel->payloadType().clockrate());
    format.setChannels(channel->payloadType().channels());
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    // the size in bytes of the audio buffers to/from sound devices
    // 160 ms seems to be the minimum to work consistently on Linux/Mac/Windows
    const int bufferSize = (format.frequency() * format.channels() * (format.sampleSize() / 8) * 160) / 1000;

    if (mode & QIODevice::ReadOnly) {
        // initialise audio output
        QAudioOutput *audioOutput = new QAudioOutput(format, this);
        audioOutput->setBufferSize(bufferSize);
        audioOutput->start(channel);
    }

    if (mode & QIODevice::WriteOnly) {
        // initialise audio input
        QAudioInput *audioInput = new QAudioInput(format, this);
        audioInput->setBufferSize(bufferSize);
        audioInput->start(channel);
    }
}


/// A call was received.

void xmppClient::slotCallReceived(QXmppCall *call)
{
    bool check;
    Q_UNUSED(check);

    qDebug() << "Got call from:" << call->jid();

    check = connect(call, SIGNAL(stateChanged(QXmppCall::State)),
                    this, SLOT(slotCallStateChanged(QXmppCall::State)));
    Q_ASSERT(check);

    check = connect(call, SIGNAL(audioModeChanged(QIODevice::OpenMode)),
                    this, SLOT(slotAudioModeChanged(QIODevice::OpenMode)));
    Q_ASSERT(check);

    // accept call
    call->accept();
}

/// A call changed state.

void xmppClient::slotCallStateChanged(QXmppCall::State state)
{
    if (state == QXmppCall::ActiveState)
        qDebug("Call active");
    else if (state == QXmppCall::DisconnectingState)
        qDebug("Call disconnecting");
    else if (state == QXmppCall::FinishedState)
        qDebug("Call finished");
}

void xmppClient::slotConnected()
{
    // lookup TURN server
    const QString domain = configuration().domain();
    debug(QString("Looking up TURN server for domain %1").arg(domain));
    m_dns.setType(QDnsLookup::SRV);
    m_dns.setName("_turn._udp." + domain);
    m_dns.lookup();
}

/// The DNS SRV lookup for TURN completed.

void xmppClient::slotDnsLookupFinished()
{
    QString serverName;

    if (m_dns.error() == QDnsLookup::NoError && !m_dns.serviceRecords().isEmpty()) {
        m_turnPort = m_dns.serviceRecords().first().port();
        QHostInfo::lookupHost(m_dns.serviceRecords().first().target(),
                              this, SLOT(slotHostInfoFinished(QHostInfo)));
    } else {
        warning("Could not find STUN server for domain " + configuration().domain());
        m_turnFinished = true;
        startCall();
    }
}

void xmppClient::slotHostInfoFinished(const QHostInfo &hostInfo)
{
    if (!hostInfo.addresses().isEmpty()) {
        info(QString("Found TURN server %1 port %2 for domain %3").arg(hostInfo.addresses().first().toString(), QString::number(m_turnPort), configuration().domain()));
        callManager->setTurnServer(hostInfo.addresses().first(), m_turnPort);
        callManager->setTurnUser(configuration().user());
        callManager->setTurnPassword(configuration().password());
    }
    m_turnFinished = true;
    startCall();
}

/// A presence was received.

void xmppClient::slotPresenceReceived(const QXmppPresence &presence)
{
    // if we don't have a recipient, or if the presence is not from the recipient,
    // do nothing
    if (m_recipient.isEmpty() ||
        QXmppUtils::jidToBareJid(presence.from()) != m_recipient ||
        presence.type() != QXmppPresence::Available)
        return;

    // start the call and connect to the its signals
    m_recipientFullJid = presence.from();
    startCall();
}

void xmppClient::startCall()
{
    bool check;
    Q_UNUSED(check);

    if (!m_turnFinished || m_recipientFullJid.isEmpty())
        return;

    // start the call and connect to the its signals
    QXmppCall *call = callManager->call(m_recipientFullJid);

    check = connect(call, SIGNAL(stateChanged(QXmppCall::State)),
                    this, SLOT(slotCallStateChanged(QXmppCall::State)));
    Q_ASSERT(check);

    check = connect(call, SIGNAL(audioModeChanged(QIODevice::OpenMode)),
                    this, SLOT(slotAudioModeChanged(QIODevice::OpenMode)));
    Q_ASSERT(check);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // we want one argument : "send" or "receive"
    if (argc != 2 || (strcmp(argv[1], "send") && strcmp(argv[1], "receive")))
    {
        fprintf(stderr, "Usage: %s send|receive\n", argv[0]);
        return EXIT_FAILURE;
    }

    xmppClient client;
    client.logger()->setLoggingType(QXmppLogger::StdoutLogging);
    if (!strcmp(argv[1], "send")) {
        client.setRecipient("qxmpp.test2@qxmpp.org");
        client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");
    } else {
        client.connectToServer("qxmpp.test2@qxmpp.org", "qxmpp456");
    }

    return a.exec();
}
