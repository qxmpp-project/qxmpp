/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Authors:
 *  Jeremy Lainé
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

#include <QObject>

class TestPackets : public QObject
{
    Q_OBJECT

private slots:
    void testArchiveList_data();
    void testArchiveList();
    void testArchiveChat_data();
    void testArchiveChat();
    void testArchiveRemove();
    void testArchiveRetrieve_data();
    void testArchiveRetrieve();
    void testBindNoResource();
    void testBindResource();
    void testBindResult();
    void testDiscovery();
    void testDiscoveryWithForm();
    void testNonSaslAuth();
    void testSession();
    void testStreamFeatures();
    void testVersionGet();
    void testVersionResult();
    void testEntityTimeGet();
    void testEntityTimeResult();
};

class TestPubSub : public QObject
{
    Q_OBJECT

private slots:
    void testItems();
    void testItemsResponse();
    void testPublish();
    void testSubscribe();
    void testSubscription();
    void testSubscriptions();
};

class TestServer : public QObject
{
    Q_OBJECT

private slots:
    void testConnect_data();
    void testConnect();
};
