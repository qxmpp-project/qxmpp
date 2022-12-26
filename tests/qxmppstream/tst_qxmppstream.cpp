// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppStream.h"

#include "util.h"

Q_DECLARE_METATYPE(QDomElement)

class TestStream : public QXmppStream
{
    Q_OBJECT

public:
    TestStream(QObject *parent)
        : QXmppStream(parent)
    {
    }

    void handleStart() override
    {
        QXmppStream::handleStart();
        Q_EMIT started();
    }

    void handleStream(const QDomElement &element) override
    {
        Q_EMIT streamReceived(element);
    }

    void handleStanza(const QDomElement &element) override
    {
        Q_EMIT stanzaReceived(element);
    }

    Q_SIGNAL void started();
    Q_SIGNAL void streamReceived(const QDomElement &element);
    Q_SIGNAL void stanzaReceived(const QDomElement &element);
};

class tst_QXmppStream : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();
    Q_SLOT void testProcessData();
};

void tst_QXmppStream::initTestCase()
{
    qRegisterMetaType<QDomElement>();
}

void tst_QXmppStream::testProcessData()
{
    TestStream stream(this);

    QSignalSpy onStarted(&stream, &TestStream::started);
    QSignalSpy onStreamReceived(&stream, &TestStream::streamReceived);
    QSignalSpy onStanzaReceived(&stream, &TestStream::stanzaReceived);

    stream.processData(R"(<?xml version="1.0" encoding="UTF-8"?>)");
    stream.processData(R"(
        <stream:stream from='juliet@im.example.com'
                       to='im.example.com'
                       version='1.0'
                       xml:lang='en'
                       xmlns='jabber:client'
                       xmlns:stream='http://etherx.jabber.org/streams'>)");

    // check stream was found
    QCOMPARE(onStreamReceived.size(), 1);
    QCOMPARE(onStanzaReceived.size(), 0);
    QCOMPARE(onStarted.size(), 0);

    // check stream information
    const auto streamElement = onStreamReceived[0][0].value<QDomElement>();
    QCOMPARE(streamElement.tagName(), QStringLiteral("stream"));
    QCOMPARE(streamElement.namespaceURI(), QStringLiteral("http://etherx.jabber.org/streams"));
    QCOMPARE(streamElement.attribute("from"), QStringLiteral("juliet@im.example.com"));
    QCOMPARE(streamElement.attribute("to"), QStringLiteral("im.example.com"));
    QCOMPARE(streamElement.attribute("version"), QStringLiteral("1.0"));
    QCOMPARE(streamElement.attribute("lang"), QStringLiteral("en"));

    stream.processData(R"(
        <stream:features>
            <starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'>
                <required/>
            </starttls>
        </stream:features>)");

    QCOMPARE(onStreamReceived.size(), 1);
    QCOMPARE(onStanzaReceived.size(), 1);
    QCOMPARE(onStarted.size(), 0);

    const auto features = onStanzaReceived[0][0].value<QDomElement>();
    QCOMPARE(features.tagName(), QStringLiteral("features"));
    QCOMPARE(features.namespaceURI(), QStringLiteral("http://etherx.jabber.org/streams"));

    // test partial data
    stream.processData(R"(<message from="juliet@im.example.co)");
    QCOMPARE(onStreamReceived.size(), 1);
    QCOMPARE(onStanzaReceived.size(), 1);
    QCOMPARE(onStarted.size(), 0);
    stream.processData(R"(m" to="stpeter@im.example.com">)");
    stream.processData(R"(<body>Moin</body>)");
    stream.processData(R"(</message>)");
    QCOMPARE(onStreamReceived.size(), 1);
    QCOMPARE(onStanzaReceived.size(), 2);
    QCOMPARE(onStarted.size(), 0);

    const auto message = onStanzaReceived[1][0].value<QDomElement>();
    QCOMPARE(message.tagName(), QStringLiteral("message"));
    QCOMPARE(message.namespaceURI(), QStringLiteral("jabber:client"));

    stream.processData(R"(</stream:stream>)");
}

QTEST_MAIN(tst_QXmppStream)
#include "tst_qxmppstream.moc"
