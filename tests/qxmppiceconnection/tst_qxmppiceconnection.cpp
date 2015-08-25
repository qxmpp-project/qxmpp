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

#include "QXmppStun.h"
#include "util.h"

class tst_QXmppIceConnection : public QObject
{
    Q_OBJECT

private slots:
    void testConnect();
};

void tst_QXmppIceConnection::testConnect()
{
    const int component = 1024;

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);

    QXmppIceConnection clientL;
    connect(&clientL, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
            &logger, SLOT(log(QXmppLogger::MessageType,QString)));
    clientL.setIceControlling(true);
    clientL.addComponent(component);
    clientL.bind(QXmppIceComponent::discoverAddresses());

    QXmppIceConnection clientR;
    connect(&clientR, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
            &logger, SLOT(log(QXmppLogger::MessageType,QString)));
    clientR.setIceControlling(false);
    clientR.addComponent(component);
    clientR.bind(QXmppIceComponent::discoverAddresses());

    // exchange credentials
    clientL.setRemoteUser(clientR.localUser());
    clientL.setRemotePassword(clientR.localPassword());
    clientR.setRemoteUser(clientL.localUser());
    clientR.setRemotePassword(clientL.localPassword());

    // exchange candidates
    foreach (const QXmppJingleCandidate &candidate, clientR.localCandidates())
        clientL.addRemoteCandidate(candidate);
    foreach (const QXmppJingleCandidate &candidate, clientL.localCandidates())
        clientR.addRemoteCandidate(candidate);

    // start ICE
    QEventLoop loop;
    connect(&clientL, SIGNAL(connected()), &loop, SLOT(quit()));
    connect(&clientR, SIGNAL(connected()), &loop, SLOT(quit()));

    clientL.connectToHost();
    clientR.connectToHost();

    // check both clients are connected
    loop.exec();
    loop.exec();
    QVERIFY(clientL.isConnected());
    QVERIFY(clientR.isConnected());
}

QTEST_MAIN(tst_QXmppIceConnection)
#include "tst_qxmppiceconnection.moc"
