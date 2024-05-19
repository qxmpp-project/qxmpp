// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBitsOfBinaryContentId.h"

#include "util.h"

#include <QObject>

Q_DECLARE_METATYPE(QCryptographicHash::Algorithm)

class tst_QXmppBitsOfBinaryContentId : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testBasic();

    Q_SLOT void testFromContentId_data();
    Q_SLOT void testFromContentId();

    Q_SLOT void testFromCidUrl_data();
    Q_SLOT void testFromCidUrl();

    Q_SLOT void testEmpty();

    Q_SLOT void testIsValid_data();
    Q_SLOT void testIsValid();

    Q_SLOT void testIsBobContentId_data();
    Q_SLOT void testIsBobContentId();

    Q_SLOT void testUnsupportedAlgorithm();
};

void tst_QXmppBitsOfBinaryContentId::testBasic()
{
    // test fromCidUrl()
    QXmppBitsOfBinaryContentId cid = QXmppBitsOfBinaryContentId::fromCidUrl(QStringLiteral(
        "cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"));

    QCOMPARE(cid.algorithm(), QCryptographicHash::Sha1);
    QCOMPARE(cid.hash().toHex(), QByteArrayLiteral("8f35fef110ffc5df08d579a50083ff9308fb6242"));
    QCOMPARE(cid.toCidUrl(), u"cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"_s);
    QCOMPARE(cid.toContentId(), u"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"_s);

    // test fromContentId()
    cid = QXmppBitsOfBinaryContentId::fromContentId(QStringLiteral(
        "sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"));

    QCOMPARE(cid.algorithm(), QCryptographicHash::Sha1);
    QCOMPARE(cid.hash().toHex(), QByteArrayLiteral("8f35fef110ffc5df08d579a50083ff9308fb6242"));
    QCOMPARE(cid.toCidUrl(), u"cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"_s);
    QCOMPARE(cid.toContentId(), u"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"_s);

    // test setters
    cid = QXmppBitsOfBinaryContentId();
    cid.setHash(QByteArray::fromHex(QByteArrayLiteral("8f35fef110ffc5df08d579a50083ff9308fb6242")));
    cid.setAlgorithm(QCryptographicHash::Sha1);

    QCOMPARE(cid.algorithm(), QCryptographicHash::Sha1);
    QCOMPARE(cid.hash().toHex(), QByteArrayLiteral("8f35fef110ffc5df08d579a50083ff9308fb6242"));
    QCOMPARE(cid.toCidUrl(), u"cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"_s);
    QCOMPARE(cid.toContentId(), u"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"_s);
}

void tst_QXmppBitsOfBinaryContentId::testFromContentId_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("isValid");

#define ROW(NAME, INPUT, IS_VALID) \
    QTest::newRow(NAME) << QStringLiteral(INPUT) << IS_VALID

    ROW("valid", "sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org", true);
    ROW("wrong-namespace", "sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob_222.xmpp.org", false);
    ROW("no-namespace", "sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@", false);
    ROW("url", "cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org", false);
    ROW("url-and-wrong-namespace", "cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob_222.xmpp.org", false);
    ROW("too-many-pluses", "sha1+sha256+sha3-256+blake2b256+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org", false);
    ROW("wrong-hash-length", "cid:sha1+08d579a50083ff9308fb6242@bob.xmpp.org", false);

#undef ROW
}

void tst_QXmppBitsOfBinaryContentId::testFromContentId()
{
    QFETCH(QString, input);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppBitsOfBinaryContentId::fromContentId(input).isValid(), isValid);
}

void tst_QXmppBitsOfBinaryContentId::testFromCidUrl_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("isValid");

#define ROW(NAME, INPUT, IS_VALID) \
    QTest::newRow(NAME) << QStringLiteral(INPUT) << IS_VALID

    ROW("valid", "cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org", true);
    ROW("no-url", "sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org", false);
    ROW("wrong-namespace", "cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@other", false);
    ROW("too-many-pluses", "cid:sha1+sha256+sha3-256+blake2b256+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org", false);
#undef ROW
}

void tst_QXmppBitsOfBinaryContentId::testFromCidUrl()
{
    QFETCH(QString, input);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppBitsOfBinaryContentId::fromCidUrl(input).isValid(), isValid);
}

void tst_QXmppBitsOfBinaryContentId::testEmpty()
{
    QXmppBitsOfBinaryContentId cid;
    QVERIFY(cid.toCidUrl().isEmpty());
    QVERIFY(cid.toContentId().isEmpty());
}

void tst_QXmppBitsOfBinaryContentId::testIsValid_data()
{
    QTest::addColumn<QByteArray>("hash");
    QTest::addColumn<QCryptographicHash::Algorithm>("algorithm");
    QTest::addColumn<bool>("isValid");

#define ROW(NAME, HASH, ALGORITHM, IS_VALID) \
    QTest::newRow(NAME) << QByteArray::fromHex(HASH) << ALGORITHM << IS_VALID

    ROW("valid",
        "8f35fef110ffc5df08d579a50083ff9308fb6242",
        QCryptographicHash::Sha1,
        true);
    ROW("valid-sha256",
        "01ba4719c80b6fe911b091a7c05124b64eeece964e09c058ef8f9805daca546b",
        QCryptographicHash::Sha256,
        true);
    ROW("wrong-hash-length", "8f35fef110ffc5df08", QCryptographicHash::Sha1, false);

#undef ROW
}

void tst_QXmppBitsOfBinaryContentId::testIsValid()
{
    QFETCH(QByteArray, hash);
    QFETCH(QCryptographicHash::Algorithm, algorithm);
    QFETCH(bool, isValid);

    QXmppBitsOfBinaryContentId contentId;
    contentId.setAlgorithm(algorithm);
    contentId.setHash(hash);

    QCOMPARE(contentId.isValid(), isValid);
}

void tst_QXmppBitsOfBinaryContentId::testIsBobContentId_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("checkIsUrl");
    QTest::addColumn<bool>("isValid");

#define ROW(NAME, INPUT, CHECK_IS_URL, IS_VALID) \
    QTest::newRow(NAME) << QStringLiteral(INPUT) << CHECK_IS_URL << IS_VALID

    ROW("valid-url-check-url", "cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org", true, true);
    ROW("valid-url-no-check-url", "cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org", false, true);
    ROW("valid-id-no-check-url", "sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org", false, true);
    ROW("not-an-url", "sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org", true, false);

    ROW("invalid-namespace-id", "sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org.org.org", false, false);
    ROW("invalid-namespace-url", "cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org.org.org", true, false);

    ROW("no-hash-algorithm", "sha18f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org", false, false);
#undef ROW
}

void tst_QXmppBitsOfBinaryContentId::testIsBobContentId()
{
    QFETCH(QString, input);
    QFETCH(bool, checkIsUrl);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppBitsOfBinaryContentId::isBitsOfBinaryContentId(input, checkIsUrl), isValid);
}

void tst_QXmppBitsOfBinaryContentId::testUnsupportedAlgorithm()
{
    QCOMPARE(
        QXmppBitsOfBinaryContentId::fromContentId(
            u"blake2s160+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"_s),
        QXmppBitsOfBinaryContentId());
}

QTEST_MAIN(tst_QXmppBitsOfBinaryContentId)
#include "tst_qxmppbitsofbinarycontentid.moc"
