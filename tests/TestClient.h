// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef CLIENTTESTING_H
#define CLIENTTESTING_H

#include "QXmppClient.h"
#include "QXmppClientExtension.h"  // needed for qDeleteAll(d->extensions)
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

    ~TestClient() override = default;

    QXmppOutgoingClient *stream() const { return d->stream; }
    QXmppOutgoingClientPrivate *streamPrivate() const { return d->stream->d.get(); }

    template<typename String>
    void inject(const String &xml)
    {
        d->stream->handleIqResponse(xmlToDom(xml));
        QCoreApplication::processEvents();
        resetIdCount();
    }

    void expect(QString &&packet)
    {
        QVERIFY2(!m_sentPackets.empty(), "No packet was sent!");
        QCOMPARE(m_sentPackets.takeFirst().replace(u'\'', u'"'), packet.replace(u'\'', u'"'));
        resetIdCount();
    }
    QString takePacket()
    {
        [this]() { QVERIFY(!m_sentPackets.isEmpty()); }();
        return m_sentPackets.takeFirst();
    }
    QString takeLastPacket()
    {
        [this]() { QVERIFY(!m_sentPackets.isEmpty()); }();
        return m_sentPackets.takeLast();
    }
    void expectNoPacket() const
    {
        if (!m_sentPackets.empty()) {
            qDebug() << "Unexpected:";
            for (const auto &packet : m_sentPackets) {
                qDebug().noquote() << " *" << packet;
            }
        }
        VERIFY2(m_sentPackets.empty(), "Unexpected packet sent!");
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

    void setStreamManagementState(QXmppClient::StreamManagementState state)
    {
        switch (state) {
        case QXmppClient::StreamManagementState::NoStreamManagement:
            d->stream->c2sStreamManager().setEnabled(false);
            break;
        case QXmppClient::StreamManagementState::NewStream:
            d->stream->c2sStreamManager().setEnabled(true);
            d->stream->c2sStreamManager().setResumed(false);
            break;
        case QXmppClient::StreamManagementState::ResumedStream:
            d->stream->c2sStreamManager().setEnabled(true);
            d->stream->c2sStreamManager().setResumed(true);
            break;
        }
    }

private:
    void onLoggerMessage(QXmppLogger::MessageType type, const QString &text)
    {
        if (type != QXmppLogger::SentMessage ||
            text == u"<r xmlns=\"urn:xmpp:sm:3\"/>") {
            return;
        }

        if (debugEnabled) {
            qDebug().noquote() << "LOG:" << text;
        }

        m_sentPackets << text;
    }

    bool debugEnabled;
    QList<QString> m_sentPackets;
};

#endif  // CLIENTTESTING_H
