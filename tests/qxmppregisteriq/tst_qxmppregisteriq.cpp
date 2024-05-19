// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBitsOfBinaryContentId.h"
#include "QXmppBitsOfBinaryData.h"
#include "QXmppBitsOfBinaryDataList.h"
#include "QXmppRegisterIq.h"

#include "util.h"

#include <QObject>

class tst_QXmppRegisterIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testGet();
    Q_SLOT void testResult();
    Q_SLOT void testResultWithForm();
    Q_SLOT void testResultWithRedirection();
    Q_SLOT void testResultWithFormAndRedirection();
    Q_SLOT void testSet();
    Q_SLOT void testSetWithForm();
    Q_SLOT void testBobData();
    Q_SLOT void testRegistered();
    Q_SLOT void testRemove();
    Q_SLOT void testChangePassword();
    Q_SLOT void testUnregistration();
};

void tst_QXmppRegisterIq::testGet()
{
    const QByteArray xml(
        "<iq id=\"reg1\" to=\"shakespeare.lit\" type=\"get\">"
        "<query xmlns=\"jabber:iq:register\"/>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg1"));
    QCOMPARE(iq.to(), QLatin1String("shakespeare.lit"));
    QCOMPARE(iq.from(), QString());
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.instructions(), QString());
    QVERIFY(!iq.isRegistered());
    QVERIFY(!iq.isRemove());
    QVERIFY(iq.username().isNull());
    QVERIFY(iq.password().isNull());
    QVERIFY(iq.email().isNull());
    QVERIFY(iq.form().isNull());
    QVERIFY(iq.outOfBandUrl().isNull());
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testResult()
{
    const QByteArray xml(
        "<iq id=\"reg1\" type=\"result\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<instructions>Choose a username and password for use with this service. Please also provide your email address.</instructions>"
        "<username/>"
        "<password/>"
        "<email/>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg1"));
    QCOMPARE(iq.to(), QString());
    QCOMPARE(iq.from(), QString());
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.instructions(), QLatin1String("Choose a username and password for use with this service. Please also provide your email address."));
    QVERIFY(!iq.username().isNull());
    QVERIFY(iq.username().isEmpty());
    QVERIFY(!iq.password().isNull());
    QVERIFY(iq.password().isEmpty());
    QVERIFY(!iq.email().isNull());
    QVERIFY(iq.email().isEmpty());
    QVERIFY(iq.form().isNull());
    QVERIFY(iq.outOfBandUrl().isNull());
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testResultWithForm()
{
    const QByteArray xml(
        "<iq id=\"reg3\" to=\"juliet@capulet.com/balcony\" from=\"contests.shakespeare.lit\" type=\"result\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<instructions>Use the enclosed form to register. If your Jabber client does not support Data Forms, visit http://www.shakespeare.lit/contests.php</instructions>"
        "<x xmlns=\"jabber:x:data\" type=\"form\">"
        "<title>Contest Registration</title>"
        "<instructions>"
        "Please provide the following information"
        "to sign up for our special contests!"
        "</instructions>"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>jabber:iq:register</value>"
        "</field>"
        "<field type=\"text-single\" label=\"Given Name\" var=\"first\">"
        "<required/>"
        "</field>"
        "<field type=\"text-single\" label=\"Family Name\" var=\"last\">"
        "<required/>"
        "</field>"
        "<field type=\"text-single\" label=\"Email Address\" var=\"email\">"
        "<required/>"
        "</field>"
        "<field type=\"list-single\" label=\"Gender\" var=\"x-gender\">"
        "<option label=\"Male\"><value>M</value></option>"
        "<option label=\"Female\"><value>F</value></option>"
        "</field>"
        "</x>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg3"));
    QCOMPARE(iq.to(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(iq.from(), QLatin1String("contests.shakespeare.lit"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.instructions(), QLatin1String("Use the enclosed form to register. If your Jabber client does not support Data Forms, visit http://www.shakespeare.lit/contests.php"));
    QVERIFY(iq.username().isNull());
    QVERIFY(iq.password().isNull());
    QVERIFY(iq.email().isNull());
    QVERIFY(!iq.form().isNull());
    QCOMPARE(iq.form().title(), QLatin1String("Contest Registration"));
    QVERIFY(iq.outOfBandUrl().isNull());
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testResultWithRedirection()
{
    const QByteArray xml(
        "<iq id=\"reg3\" type=\"result\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<instructions>"
        "To register, visit http://www.shakespeare.lit/contests.php"
        "</instructions>"
        "<x xmlns=\"jabber:x:oob\">"
        "<url>http://www.shakespeare.lit/contests.php</url>"
        "</x>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg3"));
    QCOMPARE(iq.to(), QString());
    QCOMPARE(iq.from(), QString());
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.instructions(), QLatin1String("To register, visit http://www.shakespeare.lit/contests.php"));
    QVERIFY(iq.username().isNull());
    QVERIFY(iq.password().isNull());
    QVERIFY(iq.email().isNull());
    QVERIFY(iq.form().isNull());
    QCOMPARE(iq.outOfBandUrl(), QLatin1String("http://www.shakespeare.lit/contests.php"));
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testResultWithFormAndRedirection()
{
    const QByteArray xml(
        "<iq id=\"reg3\" to=\"juliet@capulet.com/balcony\" from=\"contests.shakespeare.lit\" type=\"result\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<instructions>Use the enclosed form to register. If your Jabber client does not support Data Forms, visit http://www.shakespeare.lit/contests.php</instructions>"
        "<x xmlns=\"jabber:x:data\" type=\"form\">"
        "<title>Contest Registration</title>"
        "<instructions>"
        "Please provide the following information"
        "to sign up for our special contests!"
        "</instructions>"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>jabber:iq:register</value>"
        "</field>"
        "<field type=\"text-single\" label=\"Given Name\" var=\"first\">"
        "<required/>"
        "</field>"
        "<field type=\"text-single\" label=\"Family Name\" var=\"last\">"
        "<required/>"
        "</field>"
        "<field type=\"text-single\" label=\"Email Address\" var=\"email\">"
        "<required/>"
        "</field>"
        "<field type=\"list-single\" label=\"Gender\" var=\"x-gender\">"
        "<option label=\"Male\"><value>M</value></option>"
        "<option label=\"Female\"><value>F</value></option>"
        "</field>"
        "</x>"
        "<x xmlns=\"jabber:x:oob\">"
        "<url>http://www.shakespeare.lit/contests.php</url>"
        "</x>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg3"));
    QCOMPARE(iq.to(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(iq.from(), QLatin1String("contests.shakespeare.lit"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.instructions(), QLatin1String("Use the enclosed form to register. If your Jabber client does not support Data Forms, visit http://www.shakespeare.lit/contests.php"));
    QVERIFY(iq.username().isNull());
    QVERIFY(iq.password().isNull());
    QVERIFY(iq.email().isNull());
    QVERIFY(!iq.form().isNull());
    QCOMPARE(iq.form().title(), QLatin1String("Contest Registration"));
    QCOMPARE(iq.outOfBandUrl(), QLatin1String("http://www.shakespeare.lit/contests.php"));
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testSet()
{
    const QByteArray xml(
        "<iq id=\"reg2\" type=\"set\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<username>bill</username>"
        "<password>Calliope</password>"
        "<email>bard@shakespeare.lit</email>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg2"));
    QCOMPARE(iq.to(), QString());
    QCOMPARE(iq.from(), QString());
    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.username(), QLatin1String("bill"));
    QCOMPARE(iq.password(), QLatin1String("Calliope"));
    QCOMPARE(iq.email(), QLatin1String("bard@shakespeare.lit"));
    QVERIFY(iq.form().isNull());
    QVERIFY(iq.outOfBandUrl().isNull());
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testSetWithForm()
{
    const QByteArray xml(
        "<iq id=\"reg4\" to=\"contests.shakespeare.lit\" from=\"juliet@capulet.com/balcony\" type=\"set\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<x xmlns=\"jabber:x:data\" type=\"submit\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>jabber:iq:register</value>"
        "</field>"
        "<field type=\"text-single\" label=\"Given Name\" var=\"first\">"
        "<value>Juliet</value>"
        "</field>"
        "<field type=\"text-single\" label=\"Family Name\" var=\"last\">"
        "<value>Capulet</value>"
        "</field>"
        "<field type=\"text-single\" label=\"Email Address\" var=\"email\">"
        "<value>juliet@capulet.com</value>"
        "</field>"
        "<field type=\"list-single\" label=\"Gender\" var=\"x-gender\">"
        "<value>F</value>"
        "</field>"
        "</x>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg4"));
    QCOMPARE(iq.to(), QLatin1String("contests.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(iq.type(), QXmppIq::Set);
    QVERIFY(iq.username().isNull());
    QVERIFY(iq.password().isNull());
    QVERIFY(iq.email().isNull());
    QVERIFY(!iq.form().isNull());
    QVERIFY(iq.outOfBandUrl().isNull());
    serializePacket(iq, xml);

    QXmppRegisterIq sIq;
    sIq.setId(QLatin1String("reg4"));
    sIq.setTo(QLatin1String("contests.shakespeare.lit"));
    sIq.setFrom(QLatin1String("juliet@capulet.com/balcony"));
    sIq.setType(QXmppIq::Set);
    sIq.setForm(QXmppDataForm(
        QXmppDataForm::Submit,
        QList<QXmppDataForm::Field>()
            << QXmppDataForm::Field(
                   QXmppDataForm::Field::HiddenField,
                   u"FORM_TYPE"_s,
                   u"jabber:iq:register"_s)
            << QXmppDataForm::Field(
                   QXmppDataForm::Field::TextSingleField,
                   u"first"_s,
                   u"Juliet"_s,
                   false,
                   u"Given Name"_s)
            << QXmppDataForm::Field(
                   QXmppDataForm::Field::TextSingleField,
                   u"last"_s,
                   u"Capulet"_s,
                   false,
                   u"Family Name"_s)
            << QXmppDataForm::Field(
                   QXmppDataForm::Field::TextSingleField,
                   u"email"_s,
                   u"juliet@capulet.com"_s,
                   false,
                   u"Email Address"_s)
            << QXmppDataForm::Field(
                   QXmppDataForm::Field::ListSingleField,
                   u"x-gender"_s,
                   u"F"_s,
                   false,
                   u"Gender"_s)));
    serializePacket(sIq, xml);
}

void tst_QXmppRegisterIq::testBobData()
{
    const QByteArray xml = QByteArrayLiteral(
        "<iq type=\"result\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<data xmlns=\"urn:xmpp:bob\" "
        "cid=\"sha1+5a4c38d44fc64805cbb2d92d8b208be13ff40c0f@bob.xmpp.org\" "
        "type=\"image/png\">"
        "iVBORw0KGgoAAAANSUhEUgAAALQAAAA8BAMAAAA9AI20AAAAG1BMVEX///8AAADf39+"
        "/v79/f39fX1+fn58/Pz8fHx/8ACGJAAAACXBIWXMAAA7EAAAOxAGVKw4bAAADS0lEQV"
        "RYhe2WS3MSQRCAYTf7OKY1kT0CxsRjHmh5BENIjqEk6pHVhFzdikqO7CGyP9t59Ox2z"
        "y6UeWBVqugLzM70Nz39mqnV1lIWgBWiYXV0BYfNZ0mvwypds1r62vH/gf76ZL/88Qlc"
        "41zeAnQrpx5H3z1Npfr5ovmHusa9SpRiNNIOcdrto6PJ5LLfb5bp9zM+VDq/vptxDEa"
        "a1sql9I3R5KhtfQsA5gNCWYyulV3TyTUDdfL56BvdDl4x7RiybDq9uBgxh1TTPUHDvA"
        "qNQb+LpT5sWehxJZKKcU2MZ6sDE7PMgW2mdlBGdy6ODe6fJFdMI+us95dNqftDMdwU6"
        "+MhpuTS9slcy5TFAcwq0Jt6qssJMTQGp4BGURlmSsNoo5oHL4kqc66NdkDO75mIfCxm"
        "RAlvHxMLdcb7JONavMJbttXXKoMSneYu3OQTlwkUh4mNayi6js55/2VcsZOQfXIYelz"
        "xLcntEGc3WVCsCORJVCc5r0ajAcq+EO1Q0oPm7n7+X/3jEReGdL6qT7Ml6FCjY+quJC"
        "r+D01f6BG0SaHG56ZG32DnY2jcEV1+pU0kxTaEwaGcekN7jyu50U/TV4q6YeieyiNTu"
        "klDKZLukyjKVNwotCUB3B0XO1WjHT3c0DHSO2zACwut8GOiljJIHaJsrlof/fpWNzGM"
        "os6TgIY0hZNpJshzSi4igOhy3cl4qK+YgnqHkAYcZEgdW6/HyrEK7afoY7RCFzArLl2"
        "LLDdrdmmHZfROajwIDfWj8yQG+rzwlA3WvdJiMHtjUekiNrp1oCbmyZDEyKROGjFVDr"
        "PRzlkR9UAfG/OErnPxrop5BwpoEpXQorq2zcGxbnBJndx8Bh0yljGiGv0B4E8+YP3Xp"
        "2rGydZNy4csW8W2pIvWhvijoujRJ0luXsoymV+8AXvE9HjII72+oReS6OfomHe3xWg/"
        "f2coSbDa1XZ1CvGMjy1nH9KBl83oPnQKi+vAXKLjCrRvvT2WCMkPmSFbquiVuTH1qjv"
        "p4j/u7CWyI5/Hn3KAaJJ90eP0Zp1Kjets4WPaElkxheF7cpBESzXuIdLwyFjSub07tB"
        "6JjxH3DGiu+zwHHimdtFsMvKqG/nBxm2TwbvyU6LWs5RnJX4dSldg3QhDLAAAAAElFT"
        "kSuQmCC"
        "</data>"
        "</query>"
        "</iq>");

    QXmppBitsOfBinaryData data;
    data.setCid(QXmppBitsOfBinaryContentId::fromContentId(
        u"sha1+5a4c38d44fc64805cbb2d92d8b208be13ff40c0f@bob.xmpp.org"_s));
    data.setContentType(QMimeDatabase().mimeTypeForName(u"image/png"_s));
    data.setData(QByteArray::fromBase64(QByteArrayLiteral(
        "iVBORw0KGgoAAAANSUhEUgAAALQAAAA8BAMAAAA9AI20AAAAG1BMVEX///8AAADf39+"
        "/v79/f39fX1+fn58/Pz8fHx/8ACGJAAAACXBIWXMAAA7EAAAOxAGVKw4bAAADS0lEQV"
        "RYhe2WS3MSQRCAYTf7OKY1kT0CxsRjHmh5BENIjqEk6pHVhFzdikqO7CGyP9t59Ox2z"
        "y6UeWBVqugLzM70Nz39mqnV1lIWgBWiYXV0BYfNZ0mvwypds1r62vH/gf76ZL/88Qlc"
        "41zeAnQrpx5H3z1Npfr5ovmHusa9SpRiNNIOcdrto6PJ5LLfb5bp9zM+VDq/vptxDEa"
        "a1sql9I3R5KhtfQsA5gNCWYyulV3TyTUDdfL56BvdDl4x7RiybDq9uBgxh1TTPUHDvA"
        "qNQb+LpT5sWehxJZKKcU2MZ6sDE7PMgW2mdlBGdy6ODe6fJFdMI+us95dNqftDMdwU6"
        "+MhpuTS9slcy5TFAcwq0Jt6qssJMTQGp4BGURlmSsNoo5oHL4kqc66NdkDO75mIfCxm"
        "RAlvHxMLdcb7JONavMJbttXXKoMSneYu3OQTlwkUh4mNayi6js55/2VcsZOQfXIYelz"
        "xLcntEGc3WVCsCORJVCc5r0ajAcq+EO1Q0oPm7n7+X/3jEReGdL6qT7Ml6FCjY+quJC"
        "r+D01f6BG0SaHG56ZG32DnY2jcEV1+pU0kxTaEwaGcekN7jyu50U/TV4q6YeieyiNTu"
        "klDKZLukyjKVNwotCUB3B0XO1WjHT3c0DHSO2zACwut8GOiljJIHaJsrlof/fpWNzGM"
        "os6TgIY0hZNpJshzSi4igOhy3cl4qK+YgnqHkAYcZEgdW6/HyrEK7afoY7RCFzArLl2"
        "LLDdrdmmHZfROajwIDfWj8yQG+rzwlA3WvdJiMHtjUekiNrp1oCbmyZDEyKROGjFVDr"
        "PRzlkR9UAfG/OErnPxrop5BwpoEpXQorq2zcGxbnBJndx8Bh0yljGiGv0B4E8+YP3Xp"
        "2rGydZNy4csW8W2pIvWhvijoujRJ0luXsoymV+8AXvE9HjII72+oReS6OfomHe3xWg/"
        "f2coSbDa1XZ1CvGMjy1nH9KBl83oPnQKi+vAXKLjCrRvvT2WCMkPmSFbquiVuTH1qjv"
        "p4j/u7CWyI5/Hn3KAaJJ90eP0Zp1Kjets4WPaElkxheF7cpBESzXuIdLwyFjSub07tB"
        "6JjxH3DGiu+zwHHimdtFsMvKqG/nBxm2TwbvyU6LWs5RnJX4dSldg3QhDLAAAAAElFT"
        "kSuQmCC")));

    QXmppRegisterIq parsedIq;
    parsePacket(parsedIq, xml);
    QCOMPARE(parsedIq.type(), QXmppIq::Result);
    QCOMPARE(parsedIq.id(), u""_s);
    QCOMPARE(parsedIq.bitsOfBinaryData().size(), 1);
    QCOMPARE(parsedIq.bitsOfBinaryData().first().cid().algorithm(), data.cid().algorithm());
    QCOMPARE(parsedIq.bitsOfBinaryData().first().cid().hash(), data.cid().hash());
    QCOMPARE(parsedIq.bitsOfBinaryData().first().cid(), data.cid());
    QCOMPARE(parsedIq.bitsOfBinaryData().first().contentType(), data.contentType());
    QCOMPARE(parsedIq.bitsOfBinaryData().first().maxAge(), data.maxAge());
    QCOMPARE(parsedIq.bitsOfBinaryData().first().data(), data.data());
    QCOMPARE(parsedIq.bitsOfBinaryData().first(), data);
    serializePacket(parsedIq, xml);

    QXmppRegisterIq iq;
    iq.setType(QXmppIq::Result);
    iq.setId(u""_s);
    QXmppBitsOfBinaryDataList bobDataList;
    bobDataList << data;
    iq.setBitsOfBinaryData(bobDataList);
    serializePacket(iq, xml);

    QXmppRegisterIq iq2;
    iq2.setType(QXmppIq::Result);
    iq2.setId(u""_s);
    iq2.bitsOfBinaryData() << data;
    serializePacket(iq2, xml);

    // test const getter
    const QXmppRegisterIq constIq = iq;
    QCOMPARE(constIq.bitsOfBinaryData(), iq.bitsOfBinaryData());
}

void tst_QXmppRegisterIq::testRegistered()
{
    const QByteArray xml = QByteArrayLiteral(
        "<iq type=\"result\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<registered/>"
        "<username>juliet</username>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QVERIFY(iq.isRegistered());
    QCOMPARE(iq.username(), u"juliet"_s);
    serializePacket(iq, xml);

    iq = QXmppRegisterIq();
    iq.setId(u""_s);
    iq.setType(QXmppIq::Result);
    iq.setIsRegistered(true);
    iq.setUsername(u"juliet"_s);
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testRemove()
{
    const QByteArray xml = QByteArrayLiteral(
        "<iq type=\"result\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<remove/>"
        "<username>juliet</username>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QVERIFY(iq.isRemove());
    QCOMPARE(iq.username(), u"juliet"_s);
    serializePacket(iq, xml);

    iq = QXmppRegisterIq();
    iq.setId(u""_s);
    iq.setType(QXmppIq::Result);
    iq.setIsRemove(true);
    iq.setUsername(u"juliet"_s);
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testChangePassword()
{
    const QByteArray xml = QByteArrayLiteral(
        "<iq id=\"changePassword1\" to=\"shakespeare.lit\" type=\"set\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<username>bill</username>"
        "<password>m1cr0$0ft</password>"
        "</query>"
        "</iq>");

    auto iq = QXmppRegisterIq::createChangePasswordRequest(
        u"bill"_s,
        u"m1cr0$0ft"_s,
        u"shakespeare.lit"_s);
    iq.setId(u"changePassword1"_s);
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testUnregistration()
{
    const QByteArray xml = QByteArrayLiteral(
        "<iq id=\"unreg1\" to=\"shakespeare.lit\" type=\"set\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<remove/>"
        "</query>"
        "</iq>");

    auto iq = QXmppRegisterIq::createUnregistrationRequest(u"shakespeare.lit"_s);
    iq.setId(u"unreg1"_s);
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppRegisterIq)
#include "tst_qxmppregisteriq.moc"
