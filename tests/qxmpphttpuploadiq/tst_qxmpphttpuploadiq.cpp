// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppHttpUploadIq.h"

#include "util.h"

#include <QObject>

class tst_QXmppHttpUploadIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testRequest();
    Q_SLOT void testIsRequest_data();
    Q_SLOT void testIsRequest();
    Q_SLOT void testSlot();
    Q_SLOT void testIsSlot_data();
    Q_SLOT void testIsSlot();
};

void tst_QXmppHttpUploadIq::testRequest()
{
    const QByteArray xml(
        "<iq id=\"step_03\" "
        "to=\"upload.montague.tld\" "
        "from=\"romeo@montague.tld/garden\" "
        "type=\"get\">"
        "<request xmlns=\"urn:xmpp:http:upload:0\" "
        "filename=\"très cool.jpg\" "
        "size=\"23456\" "
        "content-type=\"image/jpeg\"/>"
        "</iq>");

    QXmppHttpUploadRequestIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.fileName(), u"très cool.jpg"_s);
    QCOMPARE(iq.size(), 23456);
    QCOMPARE(iq.contentType().name(), u"image/jpeg"_s);
    serializePacket(iq, xml);

    // test setters
    iq.setFileName("icon.png");
    QCOMPARE(iq.fileName(), u"icon.png"_s);
    iq.setSize(23421337);
    QCOMPARE(iq.size(), 23421337);
    iq.setContentType(QMimeDatabase().mimeTypeForName("image/png"));
    QCOMPARE(iq.contentType().name(), u"image/png"_s);
}

void tst_QXmppHttpUploadIq::testIsRequest_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isRequest");

    QTest::newRow("empty-iq")
        << QByteArray("<iq/>")
        << false;
    QTest::newRow("wrong-ns")
        << QByteArray("<iq><request xmlns=\"some:other:request\"/></iq>")
        << false;
    QTest::newRow("correct")
        << QByteArray("<iq><request xmlns=\"urn:xmpp:http:upload:0\"/></iq>")
        << true;
}

void tst_QXmppHttpUploadIq::testIsRequest()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isRequest);

    QDomDocument doc;
    QVERIFY(doc.setContent(xml, true));
    QCOMPARE(QXmppHttpUploadRequestIq::isHttpUploadRequestIq(doc.documentElement()), isRequest);
}

void tst_QXmppHttpUploadIq::testSlot()
{
    const QByteArray xml(
        "<iq id=\"step_03\" "
        "to=\"romeo@montague.tld/garden\" "
        "from=\"upload.montague.tld\" "
        "type=\"result\">"
        "<slot xmlns=\"urn:xmpp:http:upload:0\">"
        "<put url=\"https://upload.montague.tld/4a771ac1-f0b2-4a4a-970"
        "0-f2a26fa2bb67/tr%C3%A8s%20cool.jpg\">"
        "<header name=\"Authorization\">Basic Base64String==</header>"
        "<header name=\"Cookie\">foo=bar; user=romeo</header>"
        "</put>"
        "<get url=\"https://download.montague.tld/4a771ac1-f0b2-4a4a-9"
        "700-f2a26fa2bb67/tr%C3%A8s%20cool.jpg\"/>"
        "</slot>"
        "</iq>");

    QXmppHttpUploadSlotIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.putUrl(), QUrl("https://upload.montague.tld/4a771ac1-f0b2-4a4a"
                               "-9700-f2a26fa2bb67/tr%C3%A8s%20cool.jpg"));
    QCOMPARE(iq.getUrl(), QUrl("https://download.montague.tld/4a771ac1-f0b2-4a"
                               "4a-9700-f2a26fa2bb67/tr%C3%A8s%20cool.jpg"));
    QMap<QString, QString> headers;
    headers["Authorization"] = "Basic Base64String==";
    headers["Cookie"] = "foo=bar; user=romeo";
    QCOMPARE(iq.putHeaders(), headers);
    serializePacket(iq, xml);

    // test setters
    iq.setGetUrl(QUrl("https://dl.example.org/user/file"));
    QCOMPARE(iq.getUrl(), QUrl("https://dl.example.org/user/file"));
    iq.setPutUrl(QUrl("https://ul.example.org/user/file"));
    QCOMPARE(iq.putUrl(), QUrl("https://ul.example.org/user/file"));
    QMap<QString, QString> emptyMap;
    iq.setPutHeaders(emptyMap);
    QCOMPARE(iq.putHeaders(), emptyMap);
}

void tst_QXmppHttpUploadIq::testIsSlot_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isSlot");

    QTest::newRow("empty-iq")
        << QByteArray("<iq/>")
        << false;
    QTest::newRow("wrong-ns")
        << QByteArray("<iq><slot xmlns=\"some:other:slot\"/></iq>")
        << false;
    QTest::newRow("correct")
        << QByteArray("<iq><slot xmlns=\"urn:xmpp:http:upload:0\"/></iq>")
        << true;
}

void tst_QXmppHttpUploadIq::testIsSlot()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isSlot);

    QDomDocument doc;
    QVERIFY(doc.setContent(xml, true));
    QCOMPARE(QXmppHttpUploadSlotIq::isHttpUploadSlotIq(doc.documentElement()), isSlot);
}

QTEST_MAIN(tst_QXmppHttpUploadIq)
#include "tst_qxmpphttpuploadiq.moc"
