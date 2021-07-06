/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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

#ifndef CLIENTTESTING_H
#define CLIENTTESTING_H

#include "QXmppClient.h"
#include "QXmppClient_p.h"
#include "QXmppOutgoingClient.h"
#include "util.h"

class TestClient : public QXmppClient
{
    Q_OBJECT
public:
    TestClient(bool enableDebug = false)
        : QXmppClient(),
          debugEnabled(enableDebug)
    {
        // clear extensions
        qDeleteAll(d->extensions);
        d->extensions.clear();
        // enable stream management (so IQ requests are not stopped)
        d->stream->enableStreamManagement(true);
        // setup logging (for expect())
        logger()->setLoggingType(QXmppLogger::SignalLogging);
        connect(logger(), &QXmppLogger::message, this, &TestClient::onLoggerMessage);
        resetIdCount();
    }

    ~TestClient() override
    {
    }

    void inject(const QByteArray &xml)
    {
        d->stream->handleIqResponse(xmlToDom(xml));
        QCoreApplication::processEvents();
        resetIdCount();
    }

    void expect(QString &&packet)
    {
        QVERIFY2(!m_sentPackets.empty(), "No packet was sent!");
        QCOMPARE(m_sentPackets.takeFirst(), packet.replace(u'\'', u'"'));
        resetIdCount();
    }
    void ignore()
    {
        m_sentPackets.takeFirst();
        resetIdCount();
    }

    void resetIdCount()
    {
        QXmppStanza::s_uniqeIdNo = 0;
    }

private:
    void onLoggerMessage(QXmppLogger::MessageType type, const QString &text)
    {
        if (type != QXmppLogger::SentMessage ||
                text == QLatin1String("<r xmlns=\"urn:xmpp:sm:3\"/>")) {
            return;
        }

        if (debugEnabled) {
            qDebug() << "LOG" << text;
        }

        m_sentPackets << text;
    }

    bool debugEnabled;
    QList<QString> m_sentPackets;
};

#endif // CLIENTTESTING_H
