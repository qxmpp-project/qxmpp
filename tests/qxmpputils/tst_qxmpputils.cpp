// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppError.h"
#include "QXmppHash.h"
#include "QXmppHashing_p.h"
#include "QXmppUtils.h"

#include "util.h"
#include <QObject>

using namespace QXmpp;
using namespace QXmpp::Private;

Q_DECLARE_METATYPE(QXmpp::HashAlgorithm);

class tst_QXmppUtils : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testCrc32();
    Q_SLOT void testHmac();
    Q_SLOT void testJid();
    Q_SLOT void testMime();
    Q_SLOT void testTimezoneOffset();
    Q_SLOT void testStanzaHash();
    Q_SLOT void testCalculateHashes_data();
    Q_SLOT void testCalculateHashes();
};

void tst_QXmppUtils::testCrc32()
{
    quint32 crc = QXmppUtils::generateCrc32(QByteArray());
    QCOMPARE(crc, 0u);

    crc = QXmppUtils::generateCrc32(QByteArray("Hi There"));
    QCOMPARE(crc, 0xDB143BBEu);
}

void tst_QXmppUtils::testHmac()
{
    QByteArray hmac = QXmppUtils::generateHmacMd5(QByteArray(16, '\x0b'), QByteArray("Hi There"));
    QCOMPARE(hmac, QByteArray::fromHex("9294727a3638bb1c13f48ef8158bfc9d"));

    hmac = QXmppUtils::generateHmacMd5(QByteArray("Jefe"), QByteArray("what do ya want for nothing?"));
    QCOMPARE(hmac, QByteArray::fromHex("750c783e6ab0b503eaa86e310a5db738"));

    hmac = QXmppUtils::generateHmacMd5(QByteArray(16, '\xaa'), QByteArray(50, '\xdd'));
    QCOMPARE(hmac, QByteArray::fromHex("56be34521d144c88dbb8c733f0e8b3f6"));
}

void tst_QXmppUtils::testJid()
{
    QCOMPARE(QXmppUtils::jidToBareJid("foo@example.com/resource"), QLatin1String("foo@example.com"));
    QCOMPARE(QXmppUtils::jidToBareJid("foo@example.com"), QLatin1String("foo@example.com"));
    QCOMPARE(QXmppUtils::jidToBareJid("example.com"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToBareJid(QString()), QString());

    QCOMPARE(QXmppUtils::jidToDomain("foo@example.com/resource"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToDomain("foo@example.com"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToDomain("example.com"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToDomain(QString()), QString());

    QCOMPARE(QXmppUtils::jidToResource("foo@example.com/resource"), QLatin1String("resource"));
    QCOMPARE(QXmppUtils::jidToResource("foo@example.com"), QString());
    QCOMPARE(QXmppUtils::jidToResource("example.com"), QString());
    QCOMPARE(QXmppUtils::jidToResource(QString()), QString());

    QCOMPARE(QXmppUtils::jidToUser("foo@example.com/resource"), QLatin1String("foo"));
    QCOMPARE(QXmppUtils::jidToUser("foo@example.com"), QLatin1String("foo"));
    QCOMPARE(QXmppUtils::jidToUser("example.com"), QString());
    QCOMPARE(QXmppUtils::jidToUser(QString()), QString());
}

// FIXME: how should we test MIME detection without expose getImageType?
#if 0
QString getImageType(const QByteArray &contents);

static void testMimeType(const QString &fileName, const QString fileType)
{
    // load file from resources
    QFile file(":/" + fileName);
    QCOMPARE(file.open(QIODevice::ReadOnly), true);
    QCOMPARE(getImageType(file.readAll()), fileType);
    file.close();
}

void tst_QXmppUtils::testMime()
{
    testMimeType("test.bmp", "image/bmp");
    testMimeType("test.gif", "image/gif");
    testMimeType("test.jpg", "image/jpeg");
    testMimeType("test.mng", "video/x-mng");
    testMimeType("test.png", "image/png");
    testMimeType("test.svg", "image/svg+xml");
    testMimeType("test.xpm", "image/x-xpm");
}
#else
void tst_QXmppUtils::testMime()
{
}
#endif

void tst_QXmppUtils::testTimezoneOffset()
{
    // parsing
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("Z"), 0);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("+00:00"), 0);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("-00:00"), 0);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("+01:30"), 5400);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("-01:30"), -5400);

    // serialization
    QCOMPARE(QXmppUtils::timezoneOffsetToString(0), QLatin1String("Z"));
    QCOMPARE(QXmppUtils::timezoneOffsetToString(5400), QLatin1String("+01:30"));
    QCOMPARE(QXmppUtils::timezoneOffsetToString(-5400), QLatin1String("-01:30"));
}

void tst_QXmppUtils::testStanzaHash()
{
    for (int i = 0; i < 100; i++) {
        const QString hash = QXmppUtils::generateStanzaHash(i);
        QCOMPARE(hash.size(), i);

        if (i == 36) {
            QCOMPARE(hash.count('-'), 4);
        }
    }

    const QString hash = QXmppUtils::generateStanzaUuid();
    QCOMPARE(hash.size(), 36);
    QCOMPARE(hash.count('-'), 4);
}

void tst_QXmppUtils::testCalculateHashes_data()
{
    QTest::addColumn<QString>("filePath");
    QTest::addColumn<QByteArray>("hash");
    QTest::addColumn<QXmpp::HashAlgorithm>("algorithm");

    QTest::newRow("svg/md5")
        << QStringLiteral(":/test.svg")
        << QByteArray::fromHex("cf7ab33aca717ed632c32296c8426043")
        << HashAlgorithm::Md5;
    QTest::newRow("svg/sha-1")
        << QStringLiteral(":/test.svg")
        << QByteArray::fromHex("89d8cf114e4ec0758638ee8199af85d0974834bb")
        << HashAlgorithm::Sha1;
    QTest::newRow("svg/sha-224")
        << QStringLiteral(":/test.svg")
        << QByteArray::fromHex("f7f29e8e228a0b7529f6a4bc97b0e6bd080a8a91e8386bc1304ececc")
        << HashAlgorithm::Sha224;
    QTest::newRow("svg/sha-256")
        << QStringLiteral(":/test.svg")
        << QByteArray::fromHex("4736d79aa2912a2693cc17c5548612e1474dd1dfca2e8ddff917358482fd309f")
        << HashAlgorithm::Sha256;
    QTest::newRow("svg/sha-384")
        << QStringLiteral(":/test.svg")
        << QByteArray::fromHex("2f2572eac288d92a6f8ba09ae6e91c12f4ebaedc00df8bbbd284c4d60a483cfb21bbae417ec0688d71aa5a940637f11c")
        << HashAlgorithm::Sha384;
    QTest::newRow("svg/sha-512")
        << QStringLiteral(":/test.svg")
        << QByteArray::fromHex("85d34de6e549895d3c62773f589bb93b19c0bae62681f3fd0f3dba7262c96e87f771db4053ff7c9d0305b72222ccfe182596373917c0d109260973c258058196")
        << HashAlgorithm::Sha512;
    QTest::newRow("svg/sha3-256")
        << QStringLiteral(":/test.svg")
        << QByteArray::fromHex("4079f2effb8968e1540ce7c684a01266175c1af8cb15342fa19b7f7926de9f14")
        << HashAlgorithm::Sha3_256;
    QTest::newRow("svg/sha3-512")
        << QStringLiteral(":/test.svg")
        << QByteArray::fromHex("4c374d4c52fb57311761877a31a160703e5b67c0d3838758fa3698ae5bce10438145478116e3885cd9a8c30cf30391e7cd579d1c4c5b9c3ea8dba50930417931")
        << HashAlgorithm::Sha3_512;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QTest::newRow("svg/blake2b-512")
        << QStringLiteral(":/test.svg")
        << QByteArray::fromHex("a5e86044842e4c8306e9e2ee041fc26d57d172d5cb32346d5ee467c97c5a0b0b2350bc5a4a3dc76b92c48585c2ebbb01cf47fa59a88420fe7bba8f2a18af6f07")
        << HashAlgorithm::Blake2b_512;
#endif
    QTest::newRow("bmp/sha3-256")
        << QStringLiteral(":/test.bmp")
        << QByteArray::fromHex("e50ffd13bb279932923ee10ba6847bec7546f77747074d1a7eeeb82228daf257")
        << HashAlgorithm::Sha3_256;
}

void tst_QXmppUtils::testCalculateHashes()
{
    using Algorithm = QXmpp::HashAlgorithm;
    QFETCH(QString, filePath);
    QFETCH(QByteArray, hash);
    QFETCH(QXmpp::HashAlgorithm, algorithm);

    auto file = std::make_unique<QFile>(filePath);
    QVERIFY(file->open(QFile::ReadOnly));
    auto resultPtr = wait(calculateHashes(std::move(file), { algorithm, Algorithm::Md5, Algorithm::Sha3_512 }));
    auto &[result, _] = *resultPtr;
    auto hashes = expectVariant<std::vector<QXmppHash>>(std::move(result));
    QCOMPARE(int(hashes.size()), int(3));
    QCOMPARE(hashes.front().hash(), hash);
}

QTEST_MAIN(tst_QXmppUtils)
#include "tst_qxmpputils.moc"
