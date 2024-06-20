// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConfiguration.h"
#include "QXmppConstants_p.h"
#include "QXmppSasl2UserAgent.h"
#include "QXmppSaslManager_p.h"
#include "QXmppSasl_p.h"
#include "QXmppUtils_p.h"

#include "XmppSocket.h"
#include "util.h"

#include <QObject>

using namespace QXmpp::Private;

struct TestSocket : SendDataInterface {
    std::vector<QByteArray> sent;
    bool sendData(const QByteArray &data)
    {
        sent.push_back(data);
        return true;
    }
};

struct SaslManagerTest {
    std::unique_ptr<QXmppLoggable> loggable = std::make_unique<QXmppLoggable>();
    TestSocket socket;
    SaslManager manager = SaslManager { &socket };
};

struct Sasl2ManagerTest {
    std::unique_ptr<QXmppLoggable> loggable = std::make_unique<QXmppLoggable>();
    TestSocket socket;
    Sasl2Manager manager = Sasl2Manager { &socket };
};

class tst_QXmppSasl : public QObject
{
    Q_OBJECT

private:
    // SASL 1 parsing
    Q_SLOT void testParsing();
    Q_SLOT void testAuth_data();
    Q_SLOT void testAuth();
    Q_SLOT void testChallenge_data();
    Q_SLOT void testChallenge();
    Q_SLOT void testFailure();
    Q_SLOT void testResponse_data();
    Q_SLOT void testResponse();
    Q_SLOT void testSuccess();

    // SASL 2 parsing
    Q_SLOT void sasl2StreamFeature();
    Q_SLOT void sasl2UserAgent();
    Q_SLOT void sasl2Authenticate();
    Q_SLOT void sasl2Challenge();
    Q_SLOT void sasl2Response();
    Q_SLOT void sasl2Success();
    Q_SLOT void sasl2Failure();
    Q_SLOT void sasl2ContinueElement();
    Q_SLOT void sasl2Abort();

    Q_SLOT void htAlgorithmParsing();

    // client
    Q_SLOT void testClientAvailableMechanisms();
    Q_SLOT void testClientBadMechanism();
    Q_SLOT void testClientAnonymous();
    Q_SLOT void testClientDigestMd5();
    Q_SLOT void testClientDigestMd5_data();
    Q_SLOT void testDigestMd5ParseMessage();
    Q_SLOT void testClientFacebook();
    Q_SLOT void testClientGoogle();
    Q_SLOT void testClientPlain();
    Q_SLOT void testClientScramSha1();
    Q_SLOT void testClientScramSha1_bad();
    Q_SLOT void testClientScramSha256();
    Q_SLOT void testClientWindowsLive();
    Q_SLOT void clientHtSha256();

    // server
    Q_SLOT void testServerBadMechanism();
    Q_SLOT void testServerAnonymous();
    Q_SLOT void testServerDigestMd5();
    Q_SLOT void testServerPlain();
    Q_SLOT void testServerPlainChallenge();

    // SASL 1 client manager
    Q_SLOT void saslManagerNoMechanisms();

    // SASL 2 client manager
    Q_SLOT void sasl2ManagerPlain();
    Q_SLOT void sasl2ManagerFailure();
    Q_SLOT void sasl2ManagerUnsupportedTasks();

    // SASL 2 + FAST
    Q_SLOT void sasl2Fast();
};

void tst_QXmppSasl::testParsing()
{
    // empty
    QMap<QByteArray, QByteArray> empty = QXmppSaslDigestMd5::parseMessage(QByteArray());
    QCOMPARE(empty.size(), 0);
    QCOMPARE(QXmppSaslDigestMd5::serializeMessage(empty), QByteArray());

    // non-empty
    const QByteArray bytes("number=12345,quoted_plain=\"quoted string\",quoted_quote=\"quoted\\\\slash\\\"quote\",string=string");

    QMap<QByteArray, QByteArray> map = QXmppSaslDigestMd5::parseMessage(bytes);
    QCOMPARE(map.size(), 4);
    QCOMPARE(map["number"], QByteArray("12345"));
    QCOMPARE(map["quoted_plain"], QByteArray("quoted string"));
    QCOMPARE(map["quoted_quote"], QByteArray("quoted\\slash\"quote"));
    QCOMPARE(map["string"], QByteArray("string"));
    QCOMPARE(QXmppSaslDigestMd5::serializeMessage(map), bytes);
}

void tst_QXmppSasl::testAuth_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QString>("mechanism");
    QTest::addColumn<QByteArray>("value");

    QTest::newRow("plain")
        << QByteArray("<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"PLAIN\">AGZvbwBiYXI=</auth>")
        << "PLAIN" << QByteArray("\0foo\0bar", 8);

    QTest::newRow("digest-md5")
        << QByteArray("<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"DIGEST-MD5\"/>")
        << "DIGEST-MD5" << QByteArray();
}

void tst_QXmppSasl::testAuth()
{
    QFETCH(QByteArray, xml);
    QFETCH(QString, mechanism);
    QFETCH(QByteArray, value);

    // no condition
    auto auth = Sasl::Auth::fromDom(xmlToDom(xml));
    QVERIFY(auth);
    QCOMPARE(auth->mechanism, mechanism);
    QCOMPARE(auth->value, value);
    serializePacket(*auth, xml);
}

void tst_QXmppSasl::testChallenge_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QByteArray>("value");

    QTest::newRow("empty")
        << QByteArray("<challenge xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"/>")
        << QByteArray();

    QTest::newRow("value")
        << QByteArray("<challenge xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\">AGZvbwBiYXI=</challenge>")
        << QByteArray("\0foo\0bar", 8);
}

void tst_QXmppSasl::testChallenge()
{
    QFETCH(QByteArray, xml);
    QFETCH(QByteArray, value);

    // no condition
    auto challenge = Sasl::Challenge::fromDom(xmlToDom(xml));
    QVERIFY(challenge);
    QCOMPARE(challenge->value, value);
    serializePacket(*challenge, xml);
}

void tst_QXmppSasl::testFailure()
{
    // no condition
    const QByteArray xml = "<failure xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"/>";
    auto failure = Sasl::Failure::fromDom(xmlToDom(xml));
    QVERIFY(!failure->condition.has_value());
    QVERIFY(failure->text.isEmpty());
    serializePacket(*failure, xml);

    // not authorized
    const QByteArray xml2 = "<failure xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"><not-authorized/></failure>";
    auto failure2 = Sasl::Failure::fromDom(xmlToDom(xml2));
    QVERIFY(failure2);
    QCOMPARE(failure2->condition, Sasl::ErrorCondition::NotAuthorized);
    serializePacket(*failure2, xml2);

    // email verification required
    const QByteArray xml3 = "<failure xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\">"
                            "<account-disabled/>"
                            "<text xml:lang=\"en\">Your account has not been activated yet. Please check your email inbox for an activation link</text>"
                            "</failure>";

    auto failure3 = Sasl::Failure::fromDom(xmlToDom(xml3));
    QVERIFY(failure3);
    QCOMPARE(failure3->condition, Sasl::ErrorCondition::AccountDisabled);
    QCOMPARE(failure3->text, "Your account has not been activated yet. Please check your email inbox for an activation link");
    serializePacket(*failure3, xml3);
}

void tst_QXmppSasl::testResponse_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QByteArray>("value");

    QTest::newRow("empty")
        << QByteArray("<response xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"/>")
        << QByteArray();

    QTest::newRow("value")
        << QByteArray("<response xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\">AGZvbwBiYXI=</response>")
        << QByteArray("\0foo\0bar", 8);
}

void tst_QXmppSasl::testResponse()
{
    QFETCH(QByteArray, xml);
    QFETCH(QByteArray, value);

    // no condition
    auto response = Sasl::Response::fromDom(xmlToDom(xml));
    QVERIFY(response);
    QCOMPARE(response->value, value);
    serializePacket(*response, xml);
}

void tst_QXmppSasl::testSuccess()
{
    const QByteArray xml = "<success xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"/>";
    QVERIFY(Sasl::Success::fromDom(xmlToDom(xml)));
    Sasl::Success success;
    serializePacket(success, xml);
}

void tst_QXmppSasl::sasl2StreamFeature()
{
    auto xml =
        "<authentication xmlns='urn:xmpp:sasl:2'>"
        "<mechanism>SCRAM-SHA-1</mechanism>"
        "<mechanism>SCRAM-SHA-1-PLUS</mechanism>"
        "<inline>"
        "<bind xmlns='urn:xmpp:bind:0'>"
        "<inline>"
        "<feature var='urn:xmpp:carbons:2'/>"
        "<feature var='urn:xmpp:csi:0'/>"
        "<feature var='urn:xmpp:sm:3'/>"
        "</inline>"
        "</bind>"
        "<sm xmlns='urn:xmpp:sm:3'/>"
        "</inline>"
        "</authentication>";

    auto feature = Sasl2::StreamFeature::fromDom(xmlToDom(xml));
    QVERIFY(feature.has_value());
    QCOMPARE(feature->mechanisms.size(), 2);
    QCOMPARE(feature->mechanisms, (QList<QString> { "SCRAM-SHA-1", "SCRAM-SHA-1-PLUS" }));
    QCOMPARE(feature->streamResumptionAvailable, true);
    QVERIFY(feature->bind2Feature.has_value());
    QCOMPARE(feature->bind2Feature->features, (std::vector<QString> { "urn:xmpp:carbons:2", "urn:xmpp:csi:0", "urn:xmpp:sm:3" }));
    serializePacket(*feature, xml);
}

void tst_QXmppSasl::sasl2UserAgent()
{
    auto xml =
        "<user-agent id='d4565fa7-4d72-4749-b3d3-740edbf87770'>"
        "<software>AwesomeXMPP</software>"
        "<device>Kiva&apos;s Phone</device>"
        "</user-agent>";
    auto namespaceWrapper = u"<authenticate xmlns='%1'>%2</authenticate>"_s;

    auto userAgentDom = xmlToDom(namespaceWrapper.arg(ns_sasl_2, xml)).firstChildElement();
    auto userAgent = Sasl2::UserAgent::fromDom(userAgentDom);
    QVERIFY(userAgent.has_value());
    QCOMPARE(userAgent->id, QUuid::fromString(u"d4565fa7-4d72-4749-b3d3-740edbf87770"_s));
    QVERIFY(!userAgent->id.isNull());
    QCOMPARE(userAgent->software, "AwesomeXMPP");
    QCOMPARE(userAgent->device, "Kiva's Phone");

    serializePacket(*userAgent, xml);
}

void tst_QXmppSasl::sasl2Authenticate()
{
    auto xml =
        "<authenticate xmlns='urn:xmpp:sasl:2' mechanism='SCRAM-SHA-1-PLUS'>"
        "<initial-response>cD10bHMtZXhwb3J0ZXIsLG49dXNlcixyPTEyQzRDRDVDLUUzOEUtNEE5OC04RjZELTE1QzM4RjUxQ0NDNg==</initial-response>"
        "<user-agent id='d4565fa7-4d72-4749-b3d3-740edbf87770'>"
        "<software>AwesomeXMPP</software>"
        "<device>Kiva&apos;s Phone</device>"
        "</user-agent>"
        "<bind xmlns='urn:xmpp:bind:0'>"
        "<tag>AwesomeXMPP</tag>"
        "</bind>"
        "</authenticate>";

    auto auth = Sasl2::Authenticate::fromDom(xmlToDom(xml));
    QVERIFY(auth);
    QCOMPARE(auth->mechanism, "SCRAM-SHA-1-PLUS");
    QCOMPARE(auth->initialResponse, "p=tls-exporter,,n=user,r=12C4CD5C-E38E-4A98-8F6D-15C38F51CCC6");
    QVERIFY(auth->userAgent);
    QCOMPARE(auth->userAgent->id, QUuid::fromString(u"d4565fa7-4d72-4749-b3d3-740edbf87770"_s));
    QCOMPARE(auth->userAgent->software, "AwesomeXMPP");
    QCOMPARE(auth->userAgent->device, "Kiva's Phone");
    QVERIFY(auth->bindRequest);
    QCOMPARE(auth->bindRequest->tag, "AwesomeXMPP");
    serializePacket(*auth, xml);
}

void tst_QXmppSasl::sasl2Challenge()
{
    auto xml =
        "<challenge xmlns='urn:xmpp:sasl:2'>"
        "cj0xMkM0Q0Q1Qy1FMzhFLTRBOTgtOEY2RC0xNUMzOEY1MUNDQzZhMDkxMTdhNi1hYzUwLTRmMmYtOTNmMS05Mzc5OWMyYmRkZjYscz1RU1hDUitRNnNlazhiZjkyLGk9NDA5Ng=="
        "</challenge>";

    auto challenge = Sasl2::Challenge::fromDom(xmlToDom(xml));
    QVERIFY(challenge.has_value());
    QCOMPARE(challenge->data, "r=12C4CD5C-E38E-4A98-8F6D-15C38F51CCC6a09117a6-ac50-4f2f-93f1-93799c2bddf6,s=QSXCR+Q6sek8bf92,i=4096");
    serializePacket(*challenge, xml);
}

void tst_QXmppSasl::sasl2Response()
{
    auto xml =
        "<response xmlns='urn:xmpp:sasl:2'>"
        "Yz1jRDEwYkhNdFpYaHdiM0owWlhJc0xNY29Rdk9kQkRlUGQ0T3N3bG1BV1YzZGcxYTFXaDF0WVBUQndWaWQxMFZVLHI9MTJDNENENUMtRTM4RS00QTk4LThGNkQtMTVDMzhGNTFDQ0M2YTA5MTE3YTYtYWM1MC00ZjJmLTkzZjEtOTM3OTljMmJkZGY2LHA9VUFwbzd4bzZQYTlKK1ZhZWpmei9kRzdCb21VPQ=="
        "</response>";

    auto response = Sasl2::Response::fromDom(xmlToDom(xml));
    QVERIFY(response);
    QCOMPARE(response->data, "c=cD10bHMtZXhwb3J0ZXIsLMcoQvOdBDePd4OswlmAWV3dg1a1Wh1tYPTBwVid10VU,r=12C4CD5C-E38E-4A98-8F6D-15C38F51CCC6a09117a6-ac50-4f2f-93f1-93799c2bddf6,p=UApo7xo6Pa9J+Vaejfz/dG7BomU=");
    serializePacket(*response, xml);
}

void tst_QXmppSasl::sasl2Success()
{
    auto xml =
        "<success xmlns='urn:xmpp:sasl:2'>"
        "<additional-data>"
        "dj1tc1ZIcy9CeklPSERxWGVWSDdFbW1EdTlpZDg9"
        "</additional-data>"
        "<authorization-identifier>user@example.org/abc</authorization-identifier>"
        "<bound xmlns='urn:xmpp:bind:0'/>"
        "</success>";

    auto success = Sasl2::Success::fromDom(xmlToDom(xml));
    QVERIFY(success.has_value());
    QCOMPARE(success->additionalData, "v=msVHs/BzIOHDqXeVH7EmmDu9id8=");
    QCOMPARE(success->authorizationIdentifier, "user@example.org/abc");
    QVERIFY(success->bound);
    serializePacket(*success, xml);
}

void tst_QXmppSasl::sasl2Failure()
{
    auto xml =
        "<failure xmlns='urn:xmpp:sasl:2'>"
        "<aborted xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>"
        "<text>This is a terrible example.</text>"
        "</failure>";

    auto failure = Sasl2::Failure::fromDom(xmlToDom(xml));
    QVERIFY(failure.has_value());
    QCOMPARE(failure->condition, Sasl::ErrorCondition::Aborted);
    QCOMPARE(failure->text, "This is a terrible example.");
    serializePacket(*failure, xml);
}

void tst_QXmppSasl::sasl2ContinueElement()
{
    auto xml =
        "<continue xmlns='urn:xmpp:sasl:2'>"
        "<additional-data>"
        "SSdtIGJvcmVkIG5vdy4="
        "</additional-data>"
        "<tasks>"
        "<task>HOTP-EXAMPLE</task>"
        "<task>TOTP-EXAMPLE</task>"
        "</tasks>"
        "<text>This account requires 2FA</text>"
        "</continue>";

    auto cont = Sasl2::Continue::fromDom(xmlToDom(xml));
    QVERIFY(cont.has_value());
    QCOMPARE(cont->additionalData, "I'm bored now.");
    QCOMPARE(cont->tasks, (std::vector<QString> { "HOTP-EXAMPLE", "TOTP-EXAMPLE" }));
    QCOMPARE(cont->text, "This account requires 2FA");
    serializePacket(*cont, xml);
}

void tst_QXmppSasl::sasl2Abort()
{
    auto xml = "<abort xmlns='urn:xmpp:sasl:2'><text>I changed my mind.</text></abort>";

    auto abort = Sasl2::Abort::fromDom(xmlToDom(xml));
    QVERIFY(abort);
    QCOMPARE(abort->text, "I changed my mind.");
    serializePacket(*abort, xml);
}

void tst_QXmppSasl::htAlgorithmParsing()
{
    constexpr auto testValues = to_array<std::tuple<QStringView, SaslHtMechanism>>({
        { u"HT-SHA-256-ENDP", { IanaHashAlgorithm::Sha256, SaslHtMechanism::TlsServerEndpoint } },
        { u"HT-SHA-256-EXPR", { IanaHashAlgorithm::Sha256, SaslHtMechanism::TlsExporter } },
        { u"HT-SHA-256-UNIQ", { IanaHashAlgorithm::Sha256, SaslHtMechanism::TlsUnique } },
        { u"HT-SHA-256-NONE", { IanaHashAlgorithm::Sha256, SaslHtMechanism::None } },
        { u"HT-SHA3-256-ENDP", { IanaHashAlgorithm::Sha3_256, SaslHtMechanism::TlsServerEndpoint } },
        { u"HT-SHA3-512-EXPR", { IanaHashAlgorithm::Sha3_512, SaslHtMechanism::TlsExporter } },
        { u"HT-SHA-512-UNIQ", { IanaHashAlgorithm::Sha512, SaslHtMechanism::TlsUnique } },
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        { u"HT-BLAKE2B-256-NONE", { IanaHashAlgorithm::Blake2b_256, SaslHtMechanism::None } },
#endif
    });

    for (const auto &[string, htAlg] : testValues) {
        QCOMPARE(htAlg.toString(), string);
        QCOMPARE(unwrap(SaslHtMechanism::fromString(string)), htAlg);
    }
}

void tst_QXmppSasl::testClientAvailableMechanisms()
{
    QObject context;
    const QStringList expectedMechanisms = {
        "SCRAM-SHA3-512",
        "SCRAM-SHA-512",
        "SCRAM-SHA-256",
        "SCRAM-SHA-1",
        "DIGEST-MD5",
        "PLAIN",
        "ANONYMOUS",
        "X-FACEBOOK-PLATFORM",
        "X-MESSENGER-OAUTH2",
        "X-OAUTH2"
    };

    for (const auto &mechanism : expectedMechanisms) {
        auto parsed = SaslMechanism::fromString(mechanism);
        QVERIFY(parsed);
        QVERIFY(QXmppSaslClient::create(*parsed, &context) != nullptr);
    }
}

void tst_QXmppSasl::testClientBadMechanism()
{
    QVERIFY(!QXmppSaslClient::create("BAD-MECH"));
}

void tst_QXmppSasl::testClientAnonymous()
{
    auto client = QXmppSaslClient::create("ANONYMOUS");
    QVERIFY(client);
    QCOMPARE(client->mechanism().toString(), u"ANONYMOUS");

    // initial step returns nothing
    QCOMPARE(client->respond(QByteArray()), QByteArray());

    // any further step is an error
    QVERIFY(!client->respond(QByteArray()));
}

void tst_QXmppSasl::testClientDigestMd5_data()
{
    QTest::addColumn<QByteArray>("qop");
    QTest::newRow("qop-none") << QByteArray();
    QTest::newRow("qop-auth") << QByteArray(",qop=\"auth\"");
    QTest::newRow("qop-multi") << QByteArray(",qop=\"auth,auth-int\"");
}

void tst_QXmppSasl::testDigestMd5ParseMessage()
{
    auto result = QXmppSaslDigestMd5::parseMessage("charset=utf-8,digest-uri=\"xmpp/0.0.0.0\",nc=00000001,qop=auth,realm=0.0.0.0,response=9c3ee0a919d714c9d72853ff51c0a4f3,username=");
    QCOMPARE(result["username"], QByteArray());

    result = QXmppSaslDigestMd5::parseMessage("nc=00000001,username=,qop=auth,realm=0.0.0.0,response=9c3ee0a919d714c9d72853ff51c0a4f3");
    QCOMPARE(result["username"], QByteArray());
}

void tst_QXmppSasl::testClientDigestMd5()
{
    QFETCH(QByteArray, qop);

    QXmppSaslDigestMd5::setNonce("AMzVG8Oibf+sVUCPPlWLR8lZQvbbJtJB9vJd+u3c6dw=");

    auto client = QXmppSaslClient::create("DIGEST-MD5");
    QVERIFY(client);
    QCOMPARE(client->mechanism().toString(), "DIGEST-MD5");

    client->setUsername("qxmpp1");
    client->setCredentials(Credentials { .password = "qxmpp123" });
    client->setHost("jabber.ru");
    client->setServiceType("xmpp");

    // initial step returns nothing
    QCOMPARE(client->respond(QByteArray()), QByteArray());

    QCOMPARE(client->respond(QByteArray("nonce=\"2530347127\"") + qop + QByteArray("charset=utf-8,algorithm=md5-sess")),
             QByteArray("charset=utf-8,cnonce=\"AMzVG8Oibf+sVUCPPlWLR8lZQvbbJtJB9vJd+u3c6dw=\",digest-uri=\"xmpp/jabber.ru\",nc=00000001,nonce=2530347127,qop=auth,response=a61fbf4320577d74038b71a8546bc7ae,username=qxmpp1"));

    QCOMPARE(client->respond(QByteArray("rspauth=d92bf7f4331700c24799cbab364a14b7")), QByteArray());

    // any further step is an error
    QVERIFY(!client->respond(QByteArray()));
}

void tst_QXmppSasl::testClientFacebook()
{
    auto client = QXmppSaslClient::create("X-FACEBOOK-PLATFORM");
    QVERIFY(client);
    QCOMPARE(client->mechanism().toString(), u"X-FACEBOOK-PLATFORM");

    client->setCredentials(Credentials {
        .facebookAccessToken = "abcdefghijlkmno",
        .facebookAppId = "123456789012345",
    });

    // initial step returns nothing
    QCOMPARE(client->respond(QByteArray()), QByteArray());

    // challenge response
    QCOMPARE(client->respond(QByteArray("version=1&method=auth.xmpp_login&nonce=AA4EFEE16F2AB64B131EEFFE6EACDDB8")),
             QByteArray("access_token=abcdefghijlkmno&api_key=123456789012345&call_id&method=auth.xmpp_login&nonce=AA4EFEE16F2AB64B131EEFFE6EACDDB8&v=1.0"));

    // any further step is an error
    QVERIFY(!client->respond(QByteArray()));
}

void tst_QXmppSasl::testClientGoogle()
{
    auto client = QXmppSaslClient::create("X-OAUTH2");
    QVERIFY(client);
    QCOMPARE(client->mechanism().toString(), u"X-OAUTH2");

    client->setUsername("foo");
    client->setCredentials(Credentials { .googleAccessToken = "bar" });

    // initial step returns data
    QCOMPARE(client->respond(QByteArray()), QByteArray("\0foo\0bar", 8));

    // any further step is an error
    QVERIFY(!client->respond(QByteArray()));
}

void tst_QXmppSasl::testClientPlain()
{
    auto client = QXmppSaslClient::create("PLAIN");
    QVERIFY(client);
    QCOMPARE(client->mechanism().toString(), u"PLAIN");

    client->setUsername("foo");
    client->setCredentials(Credentials { .password = "bar" });

    // initial step returns data
    QCOMPARE(client->respond(QByteArray()), QByteArray("\0foo\0bar", 8));

    // any further step is an error
    QVERIFY(!client->respond(QByteArray()));
}

void tst_QXmppSasl::testClientScramSha1()
{
    QXmppSaslDigestMd5::setNonce("fyko+d2lbbFgONRv9qkxdawL");

    auto client = QXmppSaslClient::create("SCRAM-SHA-1");
    QCOMPARE(client->mechanism().toString(), u"SCRAM-SHA-1");

    client->setUsername("user");
    client->setCredentials(Credentials { .password = "pencil" });

    // first step
    QCOMPARE(client->respond(QByteArray()), QByteArray("n,,n=user,r=fyko+d2lbbFgONRv9qkxdawL"));

    // second step
    QCOMPARE(client->respond(QByteArray("r=fyko+d2lbbFgONRv9qkxdawL3rfcNHYJY1ZVvWVs7j,s=QSXCR+Q6sek8bf92,i=4096")),
             QByteArray("c=biws,r=fyko+d2lbbFgONRv9qkxdawL3rfcNHYJY1ZVvWVs7j,p=v0X8v3Bz2T0CJGbJQyF0X+HI4Ts="));

    // third step
    QCOMPARE(client->respond(QByteArray("v=rmF9pqV8S7suAoZWja4dJRkFsKQ")), QByteArray());

    // any further step is an error
    QVERIFY(!client->respond(QByteArray()));
}

void tst_QXmppSasl::testClientScramSha1_bad()
{
    QXmppSaslDigestMd5::setNonce("fyko+d2lbbFgONRv9qkxdawL");

    auto client = QXmppSaslClient::create("SCRAM-SHA-1");
    QCOMPARE(client->mechanism().toString(), u"SCRAM-SHA-1");

    client->setUsername("user");
    client->setCredentials(Credentials { .password = "pencil" });

    // first step
    QCOMPARE(client->respond(QByteArray()), QByteArray("n,,n=user,r=fyko+d2lbbFgONRv9qkxdawL"));

    // no nonce
    QVERIFY(!client->respond(QByteArray("s=QSXCR+Q6sek8bf92,i=4096")));

    // no salt
    QVERIFY(!client->respond(QByteArray("r=fyko+d2lbbFgONRv9qkxdawL3rfcNHYJY1ZVvWVs7j,i=4096")));

    // no iterations
    QVERIFY(!client->respond(QByteArray("r=fyko+d2lbbFgONRv9qkxdawL3rfcNHYJY1ZVvWVs7j,s=QSXCR+Q6sek8bf92")));
}

void tst_QXmppSasl::testClientScramSha256()
{
    QXmppSaslDigestMd5::setNonce("rOprNGfwEbeRWgbNEkqO");

    auto client = QXmppSaslClient::create("SCRAM-SHA-256");
    QVERIFY(client != 0);
    QCOMPARE(client->mechanism().toString(), u"SCRAM-SHA-256");

    client->setUsername("user");
    client->setCredentials(Credentials { .password = "pencil" });

    // first step
    QCOMPARE(client->respond(QByteArray()), QByteArray("n,,n=user,r=rOprNGfwEbeRWgbNEkqO"));

    // second step
    QCOMPARE(client->respond(QByteArray("r=rOprNGfwEbeRWgbNEkqO%hvYDpWUa2RaTCAfuxFIlj)hNlF$k0,s=W22ZaJ0SNY7soEsUEjb6gQ==,i=4096")),
             QByteArray("c=biws,r=rOprNGfwEbeRWgbNEkqO%hvYDpWUa2RaTCAfuxFIlj)hNlF$k0,p=dHzbZapWIk4jUhN+Ute9ytag9zjfMHgsqmmiz7AndVQ="));

    // third step
    QCOMPARE(client->respond(QByteArray("v=6rriTRBi23WpRR/wtup+mMhUZUn/dB5nLTJRsjl95G4=")), QByteArray());

    // any further step is an error
    QVERIFY(!client->respond(QByteArray()));
}

void tst_QXmppSasl::testClientWindowsLive()
{
    auto client = QXmppSaslClient::create("X-MESSENGER-OAUTH2");
    QVERIFY(client != 0);
    QCOMPARE(client->mechanism().toString(), u"X-MESSENGER-OAUTH2");

    client->setCredentials(Credentials {
        .windowsLiveAccessToken = QByteArray("footoken").toBase64(),
    });

    // initial step returns data
    QCOMPARE(client->respond(QByteArray()), QByteArray("footoken", 8));

    // any further step is an error
    QVERIFY(!client->respond(QByteArray()));
}

void tst_QXmppSasl::clientHtSha256()
{
    auto client = QXmppSaslClient::create({ SaslHtMechanism(IanaHashAlgorithm::Sha256, SaslHtMechanism::None) });
    QVERIFY(client != nullptr);
    QCOMPARE(client->mechanism().toString(), u"HT-SHA-256-NONE"_s);

    client->setUsername(u"lnj"_s);
    client->setCredentials(Credentials {
        .htToken = HtToken {
            SaslHtMechanism(IanaHashAlgorithm::Sha256, SaslHtMechanism::None),
            u"secret-token:fast-Oeie4nmlUoLHXca_YhkjwkEBgCEKKHKCArT8"_s,
            QDateTime(),
        },
    });

    auto response = client->respond({});
    QVERIFY(response.has_value());
    QCOMPARE(response->toBase64(), "bG5qAKq/BuI7mZiZ6fByiqP1ARkYUI/WyFSh7tsYik1uUiB5");

    // any further step is an error
    QVERIFY(!client->respond({}));
}

void tst_QXmppSasl::testServerBadMechanism()
{
    QVERIFY(!QXmppSaslServer::create("BAD-MECH"));
}

void tst_QXmppSasl::testServerAnonymous()
{
    auto server = QXmppSaslServer::create("ANONYMOUS");
    QVERIFY(server);
    QCOMPARE(server->mechanism(), "ANONYMOUS");

    // initial step returns success
    QByteArray response;
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Succeeded);
    QCOMPARE(response, QByteArray());

    // any further step is an error
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Failed);
}

void tst_QXmppSasl::testServerDigestMd5()
{
    QXmppSaslDigestMd5::setNonce("OI08/m+QRm6Ma+fKOjuqVXtz40sR5u9/u5GN6sSW0rs=");

    auto server = QXmppSaslServer::create("DIGEST-MD5");
    QVERIFY(server);
    QCOMPARE(server->mechanism(), "DIGEST-MD5");

    // initial step returns challenge
    QByteArray response;
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Challenge);
    QCOMPARE(response, QByteArray("algorithm=md5-sess,charset=utf-8,nonce=\"OI08/m+QRm6Ma+fKOjuqVXtz40sR5u9/u5GN6sSW0rs=\",qop=auth"));

    // password needed
    const QByteArray request = QByteArray("charset=utf-8,cnonce=\"AMzVG8Oibf+sVUCPPlWLR8lZQvbbJtJB9vJd+u3c6dw=\",digest-uri=\"xmpp/jabber.ru\",nc=00000001,nonce=\"OI08/m+QRm6Ma+fKOjuqVXtz40sR5u9/u5GN6sSW0rs=\",qop=auth,response=70e9063257ee2bf6bfd108975b917410,username=qxmpp1");
    QCOMPARE(server->respond(request, response), QXmppSaslServer::InputNeeded);
    QCOMPARE(server->username(), QLatin1String("qxmpp1"));
    server->setPassword("qxmpp123");

    // second challenge
    QCOMPARE(server->respond(request, response), QXmppSaslServer::Challenge);
    QCOMPARE(response, QByteArray("rspauth=2821a3add271b9ae02b813bed57ec878"));

    // success
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Succeeded);
    QCOMPARE(response, QByteArray());

    // any further step is an error
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Failed);
}

void tst_QXmppSasl::testServerPlain()
{
    auto server = QXmppSaslServer::create("PLAIN");
    QVERIFY(server);
    QCOMPARE(server->mechanism(), "PLAIN");

    // initial step returns success
    QByteArray response;
    QCOMPARE(server->respond(QByteArray("\0foo\0bar", 8), response), QXmppSaslServer::InputNeeded);
    QCOMPARE(response, QByteArray());
    QCOMPARE(server->username(), QLatin1String("foo"));
    QCOMPARE(server->password(), QLatin1String("bar"));

    // any further step is an error
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Failed);
}

void tst_QXmppSasl::testServerPlainChallenge()
{
    auto server = QXmppSaslServer::create("PLAIN");
    QVERIFY(server);
    QCOMPARE(server->mechanism(), "PLAIN");

    // initial step returns challenge
    QByteArray response;
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Challenge);
    QCOMPARE(response, QByteArray());

    // initial step returns success
    QCOMPARE(server->respond(QByteArray("\0foo\0bar", 8), response), QXmppSaslServer::InputNeeded);
    QCOMPARE(response, QByteArray());
    QCOMPARE(server->username(), QLatin1String("foo"));
    QCOMPARE(server->password(), QLatin1String("bar"));

    // any further step is an error
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Failed);
}

void tst_QXmppSasl::saslManagerNoMechanisms()
{
    SaslManagerTest test;
    auto &sent = test.socket.sent;

    QXmppConfiguration config;
    config.setUser("marc");
    config.setPassword("1234");
    config.setDisabledSaslMechanisms({ "SCRAM-SHA-1" });

    QVERIFY(QXmppSaslClient::isMechanismAvailable({ SaslScramMechanism(SaslScramMechanism::Sha1) }, config.credentialData()));

    auto task = test.manager.authenticate(config, { "SCRAM-SHA-1" }, test.loggable.get());

    auto [text, error] = expectFutureVariant<SaslManager::AuthError>(task);
    QCOMPARE(error.type, QXmpp::AuthenticationError::MechanismMismatch);
}

void tst_QXmppSasl::sasl2ManagerPlain()
{
    Sasl2ManagerTest test;
    auto &sent = test.socket.sent;

    QXmppConfiguration config;
    config.setUser("bowman");
    config.setPassword("1234");
    // use PLAIN
    config.setSaslAuthMechanism("PLAIN");
    config.setDisabledSaslMechanisms({});
    config.setSasl2UserAgent(QXmppSasl2UserAgent {
        QUuid::fromString(u"d4565fa7-4d72-4749-b3d3-740edbf87770"_s),
        "QXmpp",
        "HAL 9000",
    });

    auto task = test.manager.authenticate(
        Sasl2::Authenticate(),
        config,
        Sasl2::StreamFeature { { "PLAIN", "SCRAM-SHA-1" }, {}, {}, false },
        test.loggable.get());

    QVERIFY(!task.isFinished());
    QCOMPARE(sent.size(), 1);
    QCOMPARE(sent.at(0), "<authenticate xmlns=\"urn:xmpp:sasl:2\" mechanism=\"PLAIN\"><initial-response>AGJvd21hbgAxMjM0</initial-response><user-agent id=\"d4565fa7-4d72-4749-b3d3-740edbf87770\"><software>QXmpp</software><device>HAL 9000</device></user-agent></authenticate>");

    test.manager.handleElement(xmlToDom("<success xmlns='urn:xmpp:sasl:2'><authorization-identifier>bowman@example.org</authorization-identifier></success>"));

    QVERIFY(task.isFinished());
    auto success = expectFutureVariant<Sasl2::Success>(task);

    QCOMPARE(success.additionalData, std::nullopt);
    QCOMPARE(success.authorizationIdentifier, "bowman@example.org");
}

void tst_QXmppSasl::sasl2ManagerFailure()
{
    Sasl2ManagerTest test;
    auto &sent = test.socket.sent;

    QXmppConfiguration config;
    config.setUser("bowman");
    config.setPassword("1234");

    auto task = test.manager.authenticate(
        Sasl2::Authenticate(),
        config,
        Sasl2::StreamFeature { { "SCRAM-SHA-1" }, {}, {}, false },
        test.loggable.get());

    QVERIFY(!task.isFinished());
    QCOMPARE(sent.size(), 1);
    QCOMPARE(sent.at(0), "<authenticate xmlns=\"urn:xmpp:sasl:2\" mechanism=\"SCRAM-SHA-1\"><initial-response>biwsbj1ib3dtYW4scj1PSTA4L20rUVJtNk1hK2ZLT2p1cVZYdHo0MHNSNXU5L3U1R042c1NXMHJzPQ==</initial-response></authenticate>");

    auto handled = test.manager.handleElement(xmlToDom(
        "<failure xmlns='urn:xmpp:sasl:2'>"
        "<aborted xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>"
        "<optional-application-specific xmlns='urn:something:else'/>"
        "<text>This is a terrible example.</text>"
        "</failure>"));
    QCOMPARE(handled, Finished);

    auto [text, err] = expectFutureVariant<Sasl2Manager::AuthError>(task);
    QCOMPARE(err.type, QXmpp::AuthenticationError::NotAuthorized);
    QCOMPARE(err.text, "This is a terrible example.");
}

void tst_QXmppSasl::sasl2ManagerUnsupportedTasks()
{
    Sasl2ManagerTest test;
    auto &sent = test.socket.sent;

    QXmppConfiguration config;
    config.setUser("bowman");
    config.setPassword("1234");

    auto task = test.manager.authenticate(
        Sasl2::Authenticate(),
        config,
        Sasl2::StreamFeature { { "SCRAM-SHA-1" }, {}, {}, false },
        test.loggable.get());

    auto handled = test.manager.handleElement(xmlToDom(
        "<continue xmlns='urn:xmpp:sasl:2'>"
        "<additional-data>SSdtIGJvcmVkIG5vdy4=</additional-data>"
        "<tasks>"
        "<task>HOTP-EXAMPLE</task>"
        "<task>TOTP-EXAMPLE</task>"
        "</tasks>"
        "<text>This account requires 2FA</text>"
        "</continue>"));
    QCOMPARE(handled, Accepted);

    QCOMPARE(sent.size(), 2);
    QCOMPARE(sent.at(1), "<abort xmlns=\"urn:xmpp:sasl:2\"><text>SASL 2 tasks are not supported.</text></abort>");

    handled = test.manager.handleElement(xmlToDom(
        "<failure xmlns='urn:xmpp:sasl:2'>"
        "<aborted xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>"
        "<text>Aborted as you said</text>"
        "</failure>"));
    QCOMPARE(handled, Finished);

    auto [text, err] = expectFutureVariant<Sasl2Manager::AuthError>(task);
    QCOMPARE(err.type, QXmpp::AuthenticationError::RequiredTasks);
    QCOMPARE(err.text, "This account requires 2FA");
}

void tst_QXmppSasl::sasl2Fast()
{
    Sasl2ManagerTest test;
    auto &sent = test.socket.sent;

    QXmppConfiguration config;
    config.setUser("bowman");
    config.setPassword("1234");
    config.setDisabledSaslMechanisms({});
    config.setSasl2UserAgent(QXmppSasl2UserAgent {
        QUuid::fromString(u"d4565fa7-4d72-4749-b3d3-740edbf87770"_s),
        "QXmpp",
        "HAL 9000",
    });

    Sasl2::StreamFeature sasl2Feature {
        { "PLAIN" },
        {},
        FastFeature { { "HT-SHA-256-NONE", "HT-SHA3-512-NONE" }, false },
        false
    };

    Sasl2::Authenticate auth;

    FastTokenManager fast(config);
    fast.onSasl2Authenticate(auth, sasl2Feature);

    // first: authenticate without fast, but request token
    auto task = test.manager.authenticate(std::move(auth), config, sasl2Feature, test.loggable.get());

    QVERIFY(!task.isFinished());
    QCOMPARE(sent.size(), 1);
    QByteArray authenticateXml =
        "<authenticate xmlns=\"urn:xmpp:sasl:2\" mechanism=\"PLAIN\">"
        "<initial-response>AGJvd21hbgAxMjM0</initial-response>"
        "<user-agent id=\"d4565fa7-4d72-4749-b3d3-740edbf87770\"><software>QXmpp</software><device>HAL 9000</device></user-agent>"
        "<request-token xmlns=\"urn:xmpp:fast:0\" mechanism=\"HT-SHA3-512-NONE\"/>"
        "</authenticate>";
    QCOMPARE(sent.at(0), authenticateXml);

    test.manager.handleElement(xmlToDom("<success xmlns='urn:xmpp:sasl:2'><authorization-identifier>bowman@example.org</authorization-identifier><token xmlns='urn:xmpp:fast:0' token='s3cr3tt0k3n' expiry='2024-07-11T14:00:00Z'/></success>"));

    QVERIFY(task.isFinished());
    auto success = expectFutureVariant<Sasl2::Success>(task);
    fast.onSasl2Success(success);
    QVERIFY(fast.tokenChanged());

    QVERIFY(config.credentialData().htToken.has_value());
    auto token = unwrap(config.credentialData().htToken);
    QCOMPARE(token.secret, u"s3cr3tt0k3n");
    QCOMPARE(token.mechanism, SaslHtMechanism(IanaHashAlgorithm::Sha3_512, SaslHtMechanism::None));

    // Now authenticate with FAST token
    auth = Sasl2::Authenticate();
    fast.onSasl2Authenticate(auth, sasl2Feature);
    task = test.manager.authenticate(std::move(auth), config, sasl2Feature, test.loggable.get());
    QVERIFY(!task.isFinished());
    QCOMPARE(sent.size(), 2);
    authenticateXml =
        "<authenticate xmlns=\"urn:xmpp:sasl:2\" mechanism=\"HT-SHA3-512-NONE\">"
        "<initial-response>Ym93bWFuAJvHQZJynTMTHwKpXP0AYsGYWSIJMiQn/esiN1G6daGDry+2Fruyr11JLvyWPEmP1VxEZ6qBdNd/es7G1pRpmDg=</initial-response>"
        "<user-agent id=\"d4565fa7-4d72-4749-b3d3-740edbf87770\"><software>QXmpp</software><device>HAL 9000</device></user-agent>"
        "<fast xmlns=\"urn:xmpp:fast:0\"/>"
        "</authenticate>";
    QCOMPARE(sent.at(1), authenticateXml);
    test.manager.handleElement(xmlToDom("<success xmlns='urn:xmpp:sasl:2'><authorization-identifier>bowman@example.org</authorization-identifier><token xmlns='urn:xmpp:fast:0' token='t0k3n-rotation-token' expiry='2024-07-30T14:00:00Z'/></success>"));

    QVERIFY(task.isFinished());
    success = expectFutureVariant<Sasl2::Success>(task);
    fast.onSasl2Success(success);
    QVERIFY(fast.tokenChanged());
    token = unwrap(config.credentialData().htToken);
    QCOMPARE(token.secret, u"t0k3n-rotation-token");
    QCOMPARE(token.mechanism, SaslHtMechanism(IanaHashAlgorithm::Sha3_512, SaslHtMechanism::None));
}

QTEST_MAIN(tst_QXmppSasl)
#include "tst_qxmppsasl.moc"
