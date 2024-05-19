// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppE2eeMetadata.h"
#include "QXmppStanza.h"

#include "util.h"

#include <QDateTime>
#include <QObject>

class QXmppStanzaStub : public QXmppStanza
{
public:
    void toXml(QXmlStreamWriter *) const override {};
};

class tst_QXmppStanza : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testExtendedAddress_data();
    Q_SLOT void testExtendedAddress();

    Q_SLOT void testErrorCases_data();
    Q_SLOT void testErrorCases();
    Q_SLOT void testErrorFileTooLarge();
    Q_SLOT void testErrorRetry();
    Q_SLOT void testErrorEnums();

    Q_SLOT void testEncryption();
    Q_SLOT void testSenderKey();
    Q_SLOT void testSceTimestamp();
};

void tst_QXmppStanza::testExtendedAddress_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("delivered");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("type");

    QTest::newRow("simple")
        << QByteArray(R"(<address jid="foo@example.com/QXmpp" type="bcc"/>)")
        << false
        << QString()
        << u"foo@example.com/QXmpp"_s
        << u"bcc"_s;

    QTest::newRow("full")
        << QByteArray(R"(<address delivered="true" desc="some description" jid="foo@example.com/QXmpp" type="bcc"/>)")
        << true
        << u"some description"_s
        << u"foo@example.com/QXmpp"_s
        << u"bcc"_s;
}

void tst_QXmppStanza::testExtendedAddress()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, delivered);
    QFETCH(QString, description);
    QFETCH(QString, jid);
    QFETCH(QString, type);

    QXmppExtendedAddress address;
    parsePacket(address, xml);
    QCOMPARE(address.isDelivered(), delivered);
    QCOMPARE(address.description(), description);
    QCOMPARE(address.jid(), jid);
    QCOMPARE(address.type(), type);
    serializePacket(address, xml);
}

void tst_QXmppStanza::testErrorCases_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QXmppStanza::Error::Type>("type");
    QTest::addColumn<QXmppStanza::Error::Condition>("condition");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("redirectionUri");
    QTest::addColumn<QString>("by");

#define ROW(name, xml, type, condition, text, redirect, by) \
    QTest::newRow(QT_STRINGIFY(name))                       \
        << QByteArrayLiteral(xml)                           \
        << QXmppStanza::Error::type                         \
        << QXmppStanza::Error::condition                    \
        << text                                             \
        << redirect                                         \
        << by

#define BASIC(xml, type, condition) \
    ROW(condition, xml, type, condition, QString(), QString(), QString())

    ROW(
        empty - text,
        "<error type=\"modify\">"
        "<bad-request xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Modify,
        BadRequest,
        QString(),
        QString(),
        QString());
    ROW(
        redirection - uri - gone,
        "<error by=\"example.net\" type=\"cancel\">"
        "<gone xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\">"
        "xmpp:romeo@afterlife.example.net"
        "</gone>"
        "</error>",
        Cancel,
        Gone,
        QString(),
        "xmpp:romeo@afterlife.example.net",
        "example.net");
    ROW(
        redirection - uri - redirect,
        "<error type=\"cancel\">"
        "<redirect xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\">"
        "xmpp:rms@afterlife.example.net"
        "</redirect>"
        "</error>",
        Cancel,
        Redirect,
        QString(),
        "xmpp:rms@afterlife.example.net",
        QString());
    ROW(
        redirection - uri - empty,
        "<error type=\"cancel\">"
        "<redirect xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Cancel,
        Redirect,
        QString(),
        QString(),
        QString());
    ROW(
        policy - violation - text,
        "<error by=\"example.net\" type=\"modify\">"
        "<policy-violation xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "<text xml:lang=\"en\" xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\">The used words are not allowed on this server.</text>"
        "</error>",
        Modify,
        PolicyViolation,
        "The used words are not allowed on this server.",
        QString(),
        "example.net");
    ROW(
        jid - malformed - with - by,
        "<error by=\"muc.example.com\" type=\"modify\">"
        "<jid-malformed xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Modify,
        JidMalformed,
        QString(),
        QString(),
        "muc.example.com");

    BASIC(
        "<error type=\"modify\">"
        "<bad-request xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Modify,
        BadRequest);
    BASIC(
        "<error type=\"cancel\">"
        "<conflict xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Cancel,
        Conflict);
    BASIC(
        "<error type=\"cancel\">"
        "<feature-not-implemented xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Cancel,
        FeatureNotImplemented);
    BASIC(
        "<error type=\"auth\">"
        "<forbidden xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Auth,
        Forbidden);
    BASIC(
        "<error type=\"cancel\">"
        "<gone xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Cancel,
        Gone);
    BASIC(
        "<error type=\"cancel\">"
        "<internal-server-error xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Cancel,
        InternalServerError);
    BASIC(
        "<error type=\"cancel\">"
        "<item-not-found xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Cancel,
        ItemNotFound);
    BASIC(
        "<error type=\"modify\">"
        "<jid-malformed xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Modify,
        JidMalformed);
    BASIC(
        "<error type=\"modify\">"
        "<not-acceptable xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Modify,
        NotAcceptable);
    BASIC(
        "<error type=\"cancel\">"
        "<not-allowed xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Cancel,
        NotAllowed);
    BASIC(
        "<error type=\"auth\">"
        "<not-authorized xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Auth,
        NotAuthorized);
    BASIC(
        "<error type=\"modify\">"
        "<policy-violation xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Modify,
        PolicyViolation);
    BASIC(
        "<error type=\"wait\">"
        "<recipient-unavailable xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Wait,
        RecipientUnavailable);
    BASIC(
        "<error type=\"modify\">"
        "<redirect xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Modify,
        Redirect);
    BASIC(
        "<error type=\"auth\">"
        "<registration-required xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Auth,
        RegistrationRequired);
    BASIC(
        "<error type=\"cancel\">"
        "<remote-server-not-found xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Cancel,
        RemoteServerNotFound);
    BASIC(
        "<error type=\"wait\">"
        "<remote-server-timeout xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Wait,
        RemoteServerTimeout);
    BASIC(
        "<error type=\"wait\">"
        "<resource-constraint xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Wait,
        ResourceConstraint);
    BASIC(
        "<error type=\"cancel\">"
        "<service-unavailable xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Cancel,
        ServiceUnavailable);
    BASIC(
        "<error type=\"auth\">"
        "<subscription-required xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Auth,
        SubscriptionRequired);
    BASIC(
        "<error type=\"modify\">"
        "<undefined-condition xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "</error>",
        Modify,
        UndefinedCondition);

#undef BASIC
#undef ROW
}

void tst_QXmppStanza::testErrorCases()
{
    QFETCH(QByteArray, xml);
    QFETCH(QXmppStanza::Error::Type, type);
    QFETCH(QXmppStanza::Error::Condition, condition);
    QFETCH(QString, text);
    QFETCH(QString, redirectionUri);
    QFETCH(QString, by);

    // parsing
    QXmppStanza::Error error;
    parsePacket(error, xml);
    QCOMPARE(error.type(), type);
    QCOMPARE(error.condition(), condition);
    QCOMPARE(error.text(), text);
    QCOMPARE(error.redirectionUri(), redirectionUri);
    QCOMPARE(error.by(), by);
    // check parsed error results in the same xml
    serializePacket(error, xml);

    // serialization from setters
    error = QXmppStanza::Error();
    error.setType(type);
    error.setCondition(condition);
    error.setText(text);
    error.setRedirectionUri(redirectionUri);
    error.setBy(by);
    serializePacket(error, xml);
}

void tst_QXmppStanza::testErrorFileTooLarge()
{
    const QByteArray xml(
        "<error type=\"modify\">"
        "<not-acceptable xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "<text xml:lang=\"en\" "
        "xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\">"
        "File too large. The maximum file size is 20000 bytes"
        "</text>"
        "<file-too-large xmlns=\"urn:xmpp:http:upload:0\">"
        "<max-file-size>20000</max-file-size>"
        "</file-too-large>"
        "</error>");

    QXmppStanza::Error error;
    parsePacket(error, xml);
    QCOMPARE(error.type(), QXmppStanza::Error::Modify);
    QCOMPARE(error.text(), QStringLiteral("File too large. The maximum file size is "
                                          "20000 bytes"));
    QCOMPARE(error.condition(), QXmppStanza::Error::NotAcceptable);
    QVERIFY(error.fileTooLarge());
    QCOMPARE(error.maxFileSize(), 20000);
    serializePacket(error, xml);

    // test setters
    error.setMaxFileSize(60000);
    QCOMPARE(error.maxFileSize(), 60000);
    error.setFileTooLarge(false);
    QVERIFY(!error.fileTooLarge());

    QXmppStanza::Error e2;
    e2.setMaxFileSize(123000);
    QVERIFY(e2.fileTooLarge());
}

void tst_QXmppStanza::testErrorRetry()
{
    const QByteArray xml(
        "<error type=\"wait\">"
        "<resource-constraint "
        "xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
        "<text xml:lang=\"en\" "
        "xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\">"
        "Quota reached. You can only upload 5 files in 5 minutes"
        "</text>"
        "<retry xmlns=\"urn:xmpp:http:upload:0\" "
        "stamp=\"2017-12-03T23:42:05Z\"/>"
        "</error>");

    QXmppStanza::Error error;
    parsePacket(error, xml);
    QCOMPARE(error.type(), QXmppStanza::Error::Wait);
    QCOMPARE(error.text(), QStringLiteral("Quota reached. You can only upload 5 "
                                          "files in 5 minutes"));
    QCOMPARE(error.condition(), QXmppStanza::Error::ResourceConstraint);
    QCOMPARE(error.retryDate(), QDateTime(QDate(2017, 12, 03), QTime(23, 42, 05), Qt::UTC));
    serializePacket(error, xml);

    // test setter
    error.setRetryDate(QDateTime(QDate(1985, 10, 26), QTime(1, 35)));
    QCOMPARE(error.retryDate(), QDateTime(QDate(1985, 10, 26), QTime(1, 35)));
}

void tst_QXmppStanza::testErrorEnums()
{
    QXmppStanza::Error err;
    QCOMPARE(err.condition(), QXmppStanza::Error::NoCondition);
    QCOMPARE(err.type(), QXmppStanza::Error::NoType);

    err.setCondition(QXmppStanza::Error::BadRequest);
    err.setType(QXmppStanza::Error::Cancel);

    err.setCondition(QXmppStanza::Error::Condition(-1));
    err.setType(QXmppStanza::Error::Type(-1));

    QCOMPARE(err.condition(), QXmppStanza::Error::NoCondition);
    QCOMPARE(err.type(), QXmppStanza::Error::NoType);
}

void tst_QXmppStanza::testEncryption()
{
    QXmppStanzaStub stanza;
    QVERIFY(!stanza.e2eeMetadata().has_value());
    QXmppE2eeMetadata e2eeMetadata;
    e2eeMetadata.setEncryption(QXmpp::Omemo2);
    stanza.setE2eeMetadata(e2eeMetadata);
    QCOMPARE(stanza.e2eeMetadata()->encryption(), QXmpp::Omemo2);
}

void tst_QXmppStanza::testSenderKey()
{
    QXmppStanzaStub stanza;
    QVERIFY(!stanza.e2eeMetadata().has_value());
    QXmppE2eeMetadata e2eeMetadata;
    e2eeMetadata.setSenderKey(QByteArray::fromBase64(QByteArrayLiteral("aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=")));
    stanza.setE2eeMetadata(e2eeMetadata);
    QCOMPARE(stanza.e2eeMetadata()->senderKey(), QByteArray::fromBase64(QByteArrayLiteral("aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=")));
}

void tst_QXmppStanza::testSceTimestamp()
{
    QXmppStanzaStub stanza;
    QVERIFY(!stanza.e2eeMetadata().has_value());
    QXmppE2eeMetadata e2eeMetadata;
    QVERIFY(e2eeMetadata.senderKey().isNull());
    QVERIFY(e2eeMetadata.sceTimestamp().isNull());
    e2eeMetadata.setSceTimestamp(QDateTime(QDate(2022, 01, 01), QTime()));
    stanza.setE2eeMetadata(e2eeMetadata);
    QCOMPARE(stanza.e2eeMetadata()->sceTimestamp(), QDateTime(QDate(2022, 01, 01), QTime()));
}

QTEST_MAIN(tst_QXmppStanza)
#include "tst_qxmppstanza.moc"
