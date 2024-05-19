// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBitsOfBinaryContentId.h"
#include "QXmppBitsOfBinaryIq.h"

#include "util.h"

#include <QMimeType>
#include <QObject>

class tst_QXmppBitsOfBinaryIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testBasic();
    Q_SLOT void testResult();
    Q_SLOT void testOtherSubelement();
    Q_SLOT void testIsBobIq();
    Q_SLOT void fromByteArray();
};

void tst_QXmppBitsOfBinaryIq::testBasic()
{
    const QByteArray xml(
        "<iq id=\"get-data-1\" "
        "to=\"ladymacbeth@shakespeare.lit/castle\" "
        "from=\"doctor@shakespeare.lit/pda\" "
        "type=\"get\">"
        "<data xmlns=\"urn:xmpp:bob\" cid=\"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org\"></data>"
        "</iq>");

    QXmppBitsOfBinaryIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.from(), u"doctor@shakespeare.lit/pda"_s);
    QCOMPARE(iq.id(), u"get-data-1"_s);
    QCOMPARE(iq.to(), u"ladymacbeth@shakespeare.lit/castle"_s);
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.cid().toContentId(), u"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"_s);
    QCOMPARE(iq.contentType(), QMimeType());
    QCOMPARE(iq.data(), QByteArray());
    QCOMPARE(iq.maxAge(), -1);
    serializePacket(iq, xml);

    iq = QXmppBitsOfBinaryIq();
    iq.setFrom(u"doctor@shakespeare.lit/pda"_s);
    iq.setId(u"get-data-1"_s);
    iq.setTo(u"ladymacbeth@shakespeare.lit/castle"_s);
    iq.setType(QXmppIq::Get);
    iq.setCid(QXmppBitsOfBinaryContentId::fromContentId(u"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"_s));
    serializePacket(iq, xml);
}

void tst_QXmppBitsOfBinaryIq::testResult()
{
    const QByteArray xml = QByteArrayLiteral(
        "<iq id=\"data-result\" "
        "to=\"doctor@shakespeare.lit/pda\" "
        "from=\"ladymacbeth@shakespeare.lit/castle\" "
        "type=\"result\">"
        "<data xmlns=\"urn:xmpp:bob\" "
        "cid=\"sha1+5a4c38d44fc64805cbb2d92d8b208be13ff40c0f@bob.xmpp.org\" "
        "max-age=\"86400\" "
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
        "</iq>");

    const auto data = QByteArray::fromBase64(QByteArrayLiteral(
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
        "kSuQmCC"));

    QXmppBitsOfBinaryIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.id(), u"data-result"_s);
    QCOMPARE(iq.cid().algorithm(), QCryptographicHash::Sha1);
    QCOMPARE(iq.cid().hash(), QByteArray::fromHex(QByteArrayLiteral("5a4c38d44fc64805cbb2d92d8b208be13ff40c0f")));
    QCOMPARE(iq.contentType(), QMimeDatabase().mimeTypeForName(u"image/png"_s));
    QCOMPARE(iq.maxAge(), 86400);
    QCOMPARE(iq.data(), data);
    serializePacket(iq, xml);

    iq = QXmppBitsOfBinaryIq();
    iq.setId(u"data-result"_s);
    iq.setFrom(u"ladymacbeth@shakespeare.lit/castle"_s);
    iq.setTo(u"doctor@shakespeare.lit/pda"_s);
    iq.setType(QXmppIq::Result);
    iq.setCid(QXmppBitsOfBinaryContentId::fromContentId(
        u"sha1+5a4c38d44fc64805cbb2d92d8b208be13ff40c0f@bob.xmpp.org"_s));
    iq.setContentType(QMimeDatabase().mimeTypeForName(u"image/png"_s));
    iq.setMaxAge(86400);
    iq.setData(data);
    serializePacket(iq, xml);
}

void tst_QXmppBitsOfBinaryIq::testOtherSubelement()
{
    const QByteArray xml(
        "<iq id=\"get-data-1\" "
        "to=\"ladymacbeth@shakespeare.lit/castle\" "
        "from=\"doctor@shakespeare.lit/pda\" "
        "type=\"get\">"
        "<data xmlns=\"org.example.other.data\" cid=\"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org\"></data>"
        "<data xmlns=\"urn:xmpp:bob\" cid=\"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org\"></data>"
        "</iq>");

    QXmppBitsOfBinaryIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.from(), u"doctor@shakespeare.lit/pda"_s);
    QCOMPARE(iq.id(), u"get-data-1"_s);
    QCOMPARE(iq.to(), u"ladymacbeth@shakespeare.lit/castle"_s);
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.cid().toContentId(), u"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"_s);
    QCOMPARE(iq.contentType(), QMimeType());
    QCOMPARE(iq.data(), QByteArray());
    QCOMPARE(iq.maxAge(), -1);
}

void tst_QXmppBitsOfBinaryIq::testIsBobIq()
{
    QDomDocument doc;

    const QByteArray xmlSimple(
        "<iq id=\"get-data-1\" "
        "to=\"ladymacbeth@shakespeare.lit/castle\" "
        "from=\"doctor@shakespeare.lit/pda\" "
        "type=\"get\">"
        "<data xmlns=\"urn:xmpp:bob\" cid=\"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org\"></data>"
        "</iq>");
    QVERIFY(doc.setContent(xmlSimple, true));
    QCOMPARE(QXmppBitsOfBinaryIq::isBitsOfBinaryIq(doc.documentElement()), true);

    // IQs must have only one child element
    const QByteArray xmlMultipleElements(
        "<iq id=\"get-data-1\" "
        "to=\"ladymacbeth@shakespeare.lit/castle\" "
        "from=\"doctor@shakespeare.lit/pda\" "
        "type=\"get\">"
        "<data xmlns=\"urn:xmpp:other-data-format:0\" cid=\"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org\"></data>"
        "<data xmlns=\"urn:xmpp:bob\" cid=\"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org\"></data>"
        "</iq>");
    QVERIFY(doc.setContent(xmlMultipleElements, true));
    QCOMPARE(QXmppBitsOfBinaryIq::isBitsOfBinaryIq(doc.documentElement()), false);

    const QByteArray xmlWithoutBobData(
        "<iq id=\"get-data-1\" "
        "to=\"ladymacbeth@shakespeare.lit/castle\" "
        "from=\"doctor@shakespeare.lit/pda\" "
        "type=\"get\">"
        "<data xmlns=\"urn:xmpp:other-data-format:0\" cid=\"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org\"></data>"
        "</iq>");
    QVERIFY(doc.setContent(xmlWithoutBobData, true));
    QCOMPARE(QXmppBitsOfBinaryIq::isBitsOfBinaryIq(doc.documentElement()), false);
}

void tst_QXmppBitsOfBinaryIq::fromByteArray()
{
    auto data = QByteArray::fromBase64(
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
        "kSuQmCC");
    auto size = data.size();
    auto bobData = QXmppBitsOfBinaryData::fromByteArray(std::move(data));
    QCOMPARE(bobData.cid().toContentId(), u"sha1+5a4c38d44fc64805cbb2d92d8b208be13ff40c0f@bob.xmpp.org"_s);
    QCOMPARE(bobData.data().size(), size);
}

QTEST_MAIN(tst_QXmppBitsOfBinaryIq)
#include "tst_qxmppbitsofbinaryiq.moc"
