// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppOmemoMemoryStorage.h"

#include <QtTest>

class tst_QXmppOmemoMemoryStorage : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testOwnDevice();
    Q_SLOT void testSignedPreKeyPairs();
    Q_SLOT void testPreKeyPairs();
    Q_SLOT void testDevices();
    Q_SLOT void testResetAll();

    QXmppOmemoMemoryStorage m_omemoStorage;
};

void tst_QXmppOmemoMemoryStorage::testOwnDevice()
{
    auto future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    auto optionalResult = future.result().ownDevice;
    QVERIFY(!optionalResult);

    QXmppOmemoStorage::OwnDevice ownDevice;

    m_omemoStorage.setOwnDevice(ownDevice);

    // Check the default values.
    future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    optionalResult = future.result().ownDevice;
    QVERIFY(optionalResult);
    auto result = optionalResult.value();
    QCOMPARE(result.id, 0);
    QVERIFY(result.label.isEmpty());
    QVERIFY(result.privateIdentityKey.isEmpty());
    QVERIFY(result.publicIdentityKey.isEmpty());
    QCOMPARE(result.latestSignedPreKeyId, 1);
    QCOMPARE(result.latestPreKeyId, 1);

    ownDevice.id = 1;
    ownDevice.label = QStringLiteral("Notebook");
    ownDevice.privateIdentityKey = QByteArray::fromBase64(QByteArrayLiteral("ZDVNZFdJeFFUa3N6ZWdSUG9scUdoQXFpWERGbHRsZTIK"));
    ownDevice.publicIdentityKey = QByteArray::fromBase64(QByteArrayLiteral("dUsxSTJyM2tKVHE1TzNXbk1Xd0tpMGY0TnFleDRYUGkK"));
    ownDevice.latestSignedPreKeyId = 2;
    ownDevice.latestPreKeyId = 100;

    m_omemoStorage.setOwnDevice(ownDevice);

    // Check the set values.
    future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    optionalResult = future.result().ownDevice;
    QVERIFY(optionalResult);
    result = optionalResult.value();
    QCOMPARE(result.id, 1);
    QCOMPARE(result.label, QStringLiteral("Notebook"));
    QCOMPARE(result.privateIdentityKey, QByteArray::fromBase64(QByteArrayLiteral("ZDVNZFdJeFFUa3N6ZWdSUG9scUdoQXFpWERGbHRsZTIK")));
    QCOMPARE(result.publicIdentityKey, QByteArray::fromBase64(QByteArrayLiteral("dUsxSTJyM2tKVHE1TzNXbk1Xd0tpMGY0TnFleDRYUGkK")));
    QCOMPARE(result.latestSignedPreKeyId, 2);
    QCOMPARE(result.latestPreKeyId, 100);
}

void tst_QXmppOmemoMemoryStorage::testSignedPreKeyPairs()
{
    auto future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    auto result = future.result().signedPreKeyPairs;
    QVERIFY(result.isEmpty());

    QXmppOmemoStorage::SignedPreKeyPair signedPreKeyPair1;
    signedPreKeyPair1.creationDate = QDateTime(QDate(2022, 01, 01), QTime());
    signedPreKeyPair1.data = QByteArrayLiteral("FaZmWjwqppAoMff72qTzUIktGUbi4pAmds1Cuh6OElmi");

    QXmppOmemoStorage::SignedPreKeyPair signedPreKeyPair2;
    signedPreKeyPair2.creationDate = QDateTime(QDate(2022, 01, 02), QTime());
    signedPreKeyPair2.data = QByteArrayLiteral("jsrj4UYQqaHJrlysNu0uoHgmAU8ffknPpwKJhdqLYgIU");

    QHash<uint32_t, QXmppOmemoStorage::SignedPreKeyPair> signedPreKeyPairs = { { 1, signedPreKeyPair1 },
                                                                               { 2, signedPreKeyPair2 } };

    m_omemoStorage.addSignedPreKeyPair(1, signedPreKeyPair1);
    m_omemoStorage.addSignedPreKeyPair(2, signedPreKeyPair2);

    future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    result = future.result().signedPreKeyPairs;
    const auto signedPreKeyPairResult1 = result.value(1);
    QCOMPARE(signedPreKeyPairResult1.creationDate, QDateTime(QDate(2022, 01, 01), QTime()));
    QCOMPARE(signedPreKeyPairResult1.data, QByteArrayLiteral("FaZmWjwqppAoMff72qTzUIktGUbi4pAmds1Cuh6OElmi"));
    const auto signedPreKeyPairResult2 = result.value(2);
    QCOMPARE(signedPreKeyPairResult2.creationDate, QDateTime(QDate(2022, 01, 02), QTime()));
    QCOMPARE(signedPreKeyPairResult2.data, QByteArrayLiteral("jsrj4UYQqaHJrlysNu0uoHgmAU8ffknPpwKJhdqLYgIU"));

    signedPreKeyPairs.remove(1);
    m_omemoStorage.removeSignedPreKeyPair(1);

    future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    result = future.result().signedPreKeyPairs;
    const auto signedPreKeyPairResult = result.value(2);
    QCOMPARE(signedPreKeyPairResult.creationDate, QDateTime(QDate(2022, 01, 02), QTime()));
    QCOMPARE(signedPreKeyPairResult.data, QByteArrayLiteral("jsrj4UYQqaHJrlysNu0uoHgmAU8ffknPpwKJhdqLYgIU"));
}

void tst_QXmppOmemoMemoryStorage::testPreKeyPairs()
{
    auto future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    auto result = future.result().preKeyPairs;
    QVERIFY(result.isEmpty());

    const QHash<uint32_t, QByteArray> preKeyPairs1 = { { 1, QByteArrayLiteral("RZLgD0lmL2WpJbskbGKFRMZL4zqSSvU0rElmO7UwGSVt") },
                                                       { 2, QByteArrayLiteral("3PGPNsf9P7pPitp9dt2uvZYT4HkxdHJAbWqLvOPXUeca") } };
    const QHash<uint32_t, QByteArray> preKeyPairs2 = { { 3, QByteArrayLiteral("LpLBVXejfU4d0qcPOJCRNDDg9IMbOujpV3UTYtZU9LTy") } };

    QHash<uint32_t, QByteArray> preKeyPairs;
    preKeyPairs.insert(preKeyPairs1);
    preKeyPairs.insert(preKeyPairs2);

    m_omemoStorage.addPreKeyPairs(preKeyPairs1);
    m_omemoStorage.addPreKeyPairs(preKeyPairs2);

    future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    result = future.result().preKeyPairs;
    QCOMPARE(result, preKeyPairs);

    preKeyPairs.remove(1);
    m_omemoStorage.removePreKeyPair(1);

    future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    result = future.result().preKeyPairs;
    QCOMPARE(result, preKeyPairs);
}

void tst_QXmppOmemoMemoryStorage::testDevices()
{
    auto future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    auto result = future.result().devices;
    QVERIFY(result.isEmpty());

    QXmppOmemoStorage::Device deviceAlice;
    deviceAlice.label = QStringLiteral("Desktop");
    deviceAlice.keyId = QByteArray::fromBase64(QByteArrayLiteral("bEFLaDRQRkFlYXdyakE2aURoN0wyMzk2NTJEM2hRMgo="));
    deviceAlice.session = QByteArray::fromBase64(QByteArrayLiteral("Cs8CCAQSIQWIhBRMdJ80tLVT7ius0H1LutRLeXBid68NH90M/kwhGxohBT+2kM/wVQ2UrZZPJBRmGZP0ZoCCWiET7KxA3ieAa888IiBSTWnp4qrTeo7z9kfKRaAFy+fYwPBI2HCSOxfC0anyPigAMmsKIQXZ95Xs7I+tOsg76eLtp266XTuCF8STa+VZkXPPJ00WSRIgmJ73wjhXPZqIt9ofB0NVwbWOKnYzQ90SHJEd/hyBHkUaJAgAEiDxXDT00+zpJd+TKJrD6nWQxQZhB8I7vCRdD/Oxw61MYjpJCiEFmTV1l+cOLEytoTp17VOEunYlCZmDqn/qoUYI/8P9ZQsaJAgBEiB/QP+9Lb0YOhSQmIr/X75Vs1FME1qzmohSzqBVTzbfZFCnf1jsR2AAaiEFPxj3VK+knGrndOjcgMXI4wEfH/0VrbgJqobGWbewYyA="));
    deviceAlice.unrespondedSentStanzasCount = 10;
    deviceAlice.unrespondedReceivedStanzasCount = 11;
    deviceAlice.removalFromDeviceListDate = QDateTime(QDate(2022, 01, 01), QTime());

    QXmppOmemoStorage::Device deviceBob1;
    deviceBob1.label = QStringLiteral("Phone");
    deviceBob1.keyId = QByteArray::fromBase64(QByteArrayLiteral("WTV6c3B2UFhYbE9OQ1d0N0ZScUhLWXpmYnY2emJoego="));
    deviceBob1.session = QByteArray::fromBase64(QByteArrayLiteral("CvgCCAQSIQXZwE+G9R6ECMxKWPMidwcx3lPboUT2KEoea3B2T3vjUBohBQ7qW+Fb9Gi/SLsuQTv2TRixF0zLx2/mw0V4arjYSmgHIiCwuvEP2eyFU7FsbtSZBWKt+hH/DwBF7C0WrfxDrSu1bSgAMmsKIQXm5tRa73ZcUWn7fQa2YlDv+yLw1copPjdRZCrGcK7cNRIg0OXBvqBTAfyiUlLKW3LDIiSMHkRYYWDyknSJz3s+81oaJAgAEiAQlSKV+70EMYAjjW88dO52dp9e/aDhT8YUDHNFaCFUxTpJCiEF2OE4fb7Quwg0PMeJfT1uXmq/YXVaos9A7bn37TySiWkaJAgAEiDJlr5w0mBHBHZzttfVyvd2y2IzBV7bGdoX+lKHaEGIoUonCAwSIQXN7Y76Vwcsaubw8EHYaIPnBB11WjEEYcEPalwlgEUECRgCUMgnWMgnYABqIQXN7Y76Vwcsaubw8EHYaIPnBB11WjEEYcEPalwlgEUECQ=="));
    deviceBob1.unrespondedSentStanzasCount = 20;
    deviceBob1.unrespondedReceivedStanzasCount = 21;
    deviceBob1.removalFromDeviceListDate = QDateTime(QDate(2022, 01, 02), QTime());

    QXmppOmemoStorage::Device deviceBob2;
    deviceBob2.label = QStringLiteral("Tablet");
    deviceBob2.keyId = QByteArray::fromBase64(QByteArrayLiteral("U0tXcUlSVHVISzZLYUdGcW53czBtdXYxTEt2blVsbQo="));
    deviceBob2.session = QByteArray::fromBase64(QByteArrayLiteral("CvgCCAQSIQU/tpDP8FUNlK2WTyQUZhmT9GaAglohE+ysQN4ngGvPPBohBdnAT4b1HoQIzEpY8yJ3BzHeU9uhRPYoSh5rcHZPe+NQIiBNmwyjLm5xdbf5f9ab9AASopfdiSybMFMdS4SQR5pSTygAMmsKIQW5FhVKpKUzKlhUCfoCmMwoo5jUFn7+NrcOQl6CQYraZRIgkNHGSWgeoLUvYMM8wsgqU4RUv8ymv/Kv4LLJb8q4vlEaJAgAEiA/GmWir7/6tWyOTrGXsehUnnPZhFs6zGvTDNe1LZaIeTpJCiEFa7t/sVQV2uofS36GbijY63d2B4yJKFGDu6K96cU5PFsaJAgAEiA6kX2jqwfZkN0AmNOZGLPg9J8ryrSSpo74DxU85z0q/konCE4SIQWZRzzFf3M1/gzbg9/xUsNcyiUnr5jAjLpSPOj7BOW6BBgCUKd/WKd/YABqIQWZRzzFf3M1/gzbg9/xUsNcyiUnr5jAjLpSPOj7BOW6BA=="));
    deviceBob2.unrespondedSentStanzasCount = 30;
    deviceBob2.unrespondedReceivedStanzasCount = 31;
    deviceBob2.removalFromDeviceListDate = QDateTime(QDate(2022, 01, 03), QTime());

    m_omemoStorage.addDevice(QStringLiteral("alice@example.org"), 1, deviceAlice);
    m_omemoStorage.addDevice(QStringLiteral("bob@example.com"), 1, deviceBob1);
    m_omemoStorage.addDevice(QStringLiteral("bob@example.com"), 2, deviceBob2);

    future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    result = future.result().devices;
    QCOMPARE(result.size(), 2);

    auto resultDevicesAlice = result.value(QStringLiteral("alice@example.org"));
    QCOMPARE(resultDevicesAlice.size(), 1);

    auto resultDeviceAlice = resultDevicesAlice.value(1);
    QCOMPARE(resultDeviceAlice.label, QStringLiteral("Desktop"));
    QCOMPARE(resultDeviceAlice.keyId, QByteArray::fromBase64(QByteArrayLiteral("bEFLaDRQRkFlYXdyakE2aURoN0wyMzk2NTJEM2hRMgo=")));
    QCOMPARE(resultDeviceAlice.session, QByteArray::fromBase64(QByteArrayLiteral("Cs8CCAQSIQWIhBRMdJ80tLVT7ius0H1LutRLeXBid68NH90M/kwhGxohBT+2kM/wVQ2UrZZPJBRmGZP0ZoCCWiET7KxA3ieAa888IiBSTWnp4qrTeo7z9kfKRaAFy+fYwPBI2HCSOxfC0anyPigAMmsKIQXZ95Xs7I+tOsg76eLtp266XTuCF8STa+VZkXPPJ00WSRIgmJ73wjhXPZqIt9ofB0NVwbWOKnYzQ90SHJEd/hyBHkUaJAgAEiDxXDT00+zpJd+TKJrD6nWQxQZhB8I7vCRdD/Oxw61MYjpJCiEFmTV1l+cOLEytoTp17VOEunYlCZmDqn/qoUYI/8P9ZQsaJAgBEiB/QP+9Lb0YOhSQmIr/X75Vs1FME1qzmohSzqBVTzbfZFCnf1jsR2AAaiEFPxj3VK+knGrndOjcgMXI4wEfH/0VrbgJqobGWbewYyA=")));
    QCOMPARE(resultDeviceAlice.unrespondedSentStanzasCount, 10);
    QCOMPARE(resultDeviceAlice.unrespondedReceivedStanzasCount, 11);
    QCOMPARE(resultDeviceAlice.removalFromDeviceListDate, QDateTime(QDate(2022, 01, 01), QTime()));

    auto resultDevicesBob = result.value(QStringLiteral("bob@example.com"));
    QCOMPARE(resultDevicesBob.size(), 2);

    auto resultDeviceBob1 = resultDevicesBob.value(1);
    QCOMPARE(resultDeviceBob1.label, QStringLiteral("Phone"));
    QCOMPARE(resultDeviceBob1.keyId, QByteArray::fromBase64(QByteArrayLiteral("WTV6c3B2UFhYbE9OQ1d0N0ZScUhLWXpmYnY2emJoego=")));
    QCOMPARE(resultDeviceBob1.session, QByteArray::fromBase64(QByteArrayLiteral("CvgCCAQSIQXZwE+G9R6ECMxKWPMidwcx3lPboUT2KEoea3B2T3vjUBohBQ7qW+Fb9Gi/SLsuQTv2TRixF0zLx2/mw0V4arjYSmgHIiCwuvEP2eyFU7FsbtSZBWKt+hH/DwBF7C0WrfxDrSu1bSgAMmsKIQXm5tRa73ZcUWn7fQa2YlDv+yLw1copPjdRZCrGcK7cNRIg0OXBvqBTAfyiUlLKW3LDIiSMHkRYYWDyknSJz3s+81oaJAgAEiAQlSKV+70EMYAjjW88dO52dp9e/aDhT8YUDHNFaCFUxTpJCiEF2OE4fb7Quwg0PMeJfT1uXmq/YXVaos9A7bn37TySiWkaJAgAEiDJlr5w0mBHBHZzttfVyvd2y2IzBV7bGdoX+lKHaEGIoUonCAwSIQXN7Y76Vwcsaubw8EHYaIPnBB11WjEEYcEPalwlgEUECRgCUMgnWMgnYABqIQXN7Y76Vwcsaubw8EHYaIPnBB11WjEEYcEPalwlgEUECQ==")));
    QCOMPARE(resultDeviceBob1.unrespondedSentStanzasCount, 20);
    QCOMPARE(resultDeviceBob1.unrespondedReceivedStanzasCount, 21);
    QCOMPARE(resultDeviceBob1.removalFromDeviceListDate, QDateTime(QDate(2022, 01, 02), QTime()));

    auto resultDeviceBob2 = resultDevicesBob.value(2);
    QCOMPARE(resultDeviceBob2.label, QStringLiteral("Tablet"));
    QCOMPARE(resultDeviceBob2.keyId, QByteArray::fromBase64(QByteArrayLiteral("U0tXcUlSVHVISzZLYUdGcW53czBtdXYxTEt2blVsbQo=")));
    QCOMPARE(resultDeviceBob2.session, QByteArray::fromBase64(QByteArrayLiteral("CvgCCAQSIQU/tpDP8FUNlK2WTyQUZhmT9GaAglohE+ysQN4ngGvPPBohBdnAT4b1HoQIzEpY8yJ3BzHeU9uhRPYoSh5rcHZPe+NQIiBNmwyjLm5xdbf5f9ab9AASopfdiSybMFMdS4SQR5pSTygAMmsKIQW5FhVKpKUzKlhUCfoCmMwoo5jUFn7+NrcOQl6CQYraZRIgkNHGSWgeoLUvYMM8wsgqU4RUv8ymv/Kv4LLJb8q4vlEaJAgAEiA/GmWir7/6tWyOTrGXsehUnnPZhFs6zGvTDNe1LZaIeTpJCiEFa7t/sVQV2uofS36GbijY63d2B4yJKFGDu6K96cU5PFsaJAgAEiA6kX2jqwfZkN0AmNOZGLPg9J8ryrSSpo74DxU85z0q/konCE4SIQWZRzzFf3M1/gzbg9/xUsNcyiUnr5jAjLpSPOj7BOW6BBgCUKd/WKd/YABqIQWZRzzFf3M1/gzbg9/xUsNcyiUnr5jAjLpSPOj7BOW6BA==")));
    QCOMPARE(resultDeviceBob2.unrespondedSentStanzasCount, 30);
    QCOMPARE(resultDeviceBob2.unrespondedReceivedStanzasCount, 31);
    QCOMPARE(resultDeviceBob2.removalFromDeviceListDate, QDateTime(QDate(2022, 01, 03), QTime()));

    m_omemoStorage.removeDevice(QStringLiteral("bob@example.com"), 2);

    future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    result = future.result().devices;
    QCOMPARE(result.size(), 2);

    resultDevicesAlice = result.value(QStringLiteral("alice@example.org"));
    QCOMPARE(resultDevicesAlice.size(), 1);

    resultDeviceAlice = resultDevicesAlice.value(1);
    QCOMPARE(resultDeviceAlice.label, QStringLiteral("Desktop"));
    QCOMPARE(resultDeviceAlice.keyId, QByteArray::fromBase64(QByteArrayLiteral("bEFLaDRQRkFlYXdyakE2aURoN0wyMzk2NTJEM2hRMgo=")));
    QCOMPARE(resultDeviceAlice.session, QByteArray::fromBase64(QByteArrayLiteral("Cs8CCAQSIQWIhBRMdJ80tLVT7ius0H1LutRLeXBid68NH90M/kwhGxohBT+2kM/wVQ2UrZZPJBRmGZP0ZoCCWiET7KxA3ieAa888IiBSTWnp4qrTeo7z9kfKRaAFy+fYwPBI2HCSOxfC0anyPigAMmsKIQXZ95Xs7I+tOsg76eLtp266XTuCF8STa+VZkXPPJ00WSRIgmJ73wjhXPZqIt9ofB0NVwbWOKnYzQ90SHJEd/hyBHkUaJAgAEiDxXDT00+zpJd+TKJrD6nWQxQZhB8I7vCRdD/Oxw61MYjpJCiEFmTV1l+cOLEytoTp17VOEunYlCZmDqn/qoUYI/8P9ZQsaJAgBEiB/QP+9Lb0YOhSQmIr/X75Vs1FME1qzmohSzqBVTzbfZFCnf1jsR2AAaiEFPxj3VK+knGrndOjcgMXI4wEfH/0VrbgJqobGWbewYyA=")));
    QCOMPARE(resultDeviceAlice.unrespondedSentStanzasCount, 10);
    QCOMPARE(resultDeviceAlice.unrespondedReceivedStanzasCount, 11);
    QCOMPARE(resultDeviceAlice.removalFromDeviceListDate, QDateTime(QDate(2022, 01, 01), QTime()));

    resultDevicesBob = result.value(QStringLiteral("bob@example.com"));
    QCOMPARE(resultDevicesBob.size(), 1);

    resultDeviceBob1 = resultDevicesBob.value(1);
    QCOMPARE(resultDeviceBob1.label, QStringLiteral("Phone"));
    QCOMPARE(resultDeviceBob1.keyId, QByteArray::fromBase64(QByteArrayLiteral("WTV6c3B2UFhYbE9OQ1d0N0ZScUhLWXpmYnY2emJoego=")));
    QCOMPARE(resultDeviceBob1.session, QByteArray::fromBase64(QByteArrayLiteral("CvgCCAQSIQXZwE+G9R6ECMxKWPMidwcx3lPboUT2KEoea3B2T3vjUBohBQ7qW+Fb9Gi/SLsuQTv2TRixF0zLx2/mw0V4arjYSmgHIiCwuvEP2eyFU7FsbtSZBWKt+hH/DwBF7C0WrfxDrSu1bSgAMmsKIQXm5tRa73ZcUWn7fQa2YlDv+yLw1copPjdRZCrGcK7cNRIg0OXBvqBTAfyiUlLKW3LDIiSMHkRYYWDyknSJz3s+81oaJAgAEiAQlSKV+70EMYAjjW88dO52dp9e/aDhT8YUDHNFaCFUxTpJCiEF2OE4fb7Quwg0PMeJfT1uXmq/YXVaos9A7bn37TySiWkaJAgAEiDJlr5w0mBHBHZzttfVyvd2y2IzBV7bGdoX+lKHaEGIoUonCAwSIQXN7Y76Vwcsaubw8EHYaIPnBB11WjEEYcEPalwlgEUECRgCUMgnWMgnYABqIQXN7Y76Vwcsaubw8EHYaIPnBB11WjEEYcEPalwlgEUECQ==")));
    QCOMPARE(resultDeviceBob1.unrespondedSentStanzasCount, 20);
    QCOMPARE(resultDeviceBob1.unrespondedReceivedStanzasCount, 21);
    QCOMPARE(resultDeviceBob1.removalFromDeviceListDate, QDateTime(QDate(2022, 01, 02), QTime()));

    m_omemoStorage.removeDevice(QStringLiteral("alice@example.org"), 1);

    future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    result = future.result().devices;
    QCOMPARE(result.size(), 1);

    resultDevicesBob = result.value(QStringLiteral("bob@example.com"));
    QCOMPARE(resultDevicesBob.size(), 1);

    resultDeviceBob1 = resultDevicesBob.value(1);
    QCOMPARE(resultDeviceBob1.label, QStringLiteral("Phone"));
    QCOMPARE(resultDeviceBob1.keyId, QByteArray::fromBase64(QByteArrayLiteral("WTV6c3B2UFhYbE9OQ1d0N0ZScUhLWXpmYnY2emJoego=")));
    QCOMPARE(resultDeviceBob1.session, QByteArray::fromBase64(QByteArrayLiteral("CvgCCAQSIQXZwE+G9R6ECMxKWPMidwcx3lPboUT2KEoea3B2T3vjUBohBQ7qW+Fb9Gi/SLsuQTv2TRixF0zLx2/mw0V4arjYSmgHIiCwuvEP2eyFU7FsbtSZBWKt+hH/DwBF7C0WrfxDrSu1bSgAMmsKIQXm5tRa73ZcUWn7fQa2YlDv+yLw1copPjdRZCrGcK7cNRIg0OXBvqBTAfyiUlLKW3LDIiSMHkRYYWDyknSJz3s+81oaJAgAEiAQlSKV+70EMYAjjW88dO52dp9e/aDhT8YUDHNFaCFUxTpJCiEF2OE4fb7Quwg0PMeJfT1uXmq/YXVaos9A7bn37TySiWkaJAgAEiDJlr5w0mBHBHZzttfVyvd2y2IzBV7bGdoX+lKHaEGIoUonCAwSIQXN7Y76Vwcsaubw8EHYaIPnBB11WjEEYcEPalwlgEUECRgCUMgnWMgnYABqIQXN7Y76Vwcsaubw8EHYaIPnBB11WjEEYcEPalwlgEUECQ==")));
    QCOMPARE(resultDeviceBob1.unrespondedSentStanzasCount, 20);
    QCOMPARE(resultDeviceBob1.unrespondedReceivedStanzasCount, 21);
    QCOMPARE(resultDeviceBob1.removalFromDeviceListDate, QDateTime(QDate(2022, 01, 02), QTime()));

    m_omemoStorage.addDevice(QStringLiteral("alice@example.org"), 1, deviceAlice);
    m_omemoStorage.addDevice(QStringLiteral("bob@example.com"), 2, deviceBob2);
    m_omemoStorage.removeDevices(QStringLiteral("bob@example.com"));

    future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    result = future.result().devices;
    QCOMPARE(result.size(), 1);

    resultDevicesAlice = result.value(QStringLiteral("alice@example.org"));
    QCOMPARE(resultDevicesAlice.size(), 1);

    resultDeviceAlice = resultDevicesAlice.value(1);
    QCOMPARE(resultDeviceAlice.label, QStringLiteral("Desktop"));
    QCOMPARE(resultDeviceAlice.keyId, QByteArray::fromBase64(QByteArrayLiteral("bEFLaDRQRkFlYXdyakE2aURoN0wyMzk2NTJEM2hRMgo=")));
    QCOMPARE(resultDeviceAlice.session, QByteArray::fromBase64(QByteArrayLiteral("Cs8CCAQSIQWIhBRMdJ80tLVT7ius0H1LutRLeXBid68NH90M/kwhGxohBT+2kM/wVQ2UrZZPJBRmGZP0ZoCCWiET7KxA3ieAa888IiBSTWnp4qrTeo7z9kfKRaAFy+fYwPBI2HCSOxfC0anyPigAMmsKIQXZ95Xs7I+tOsg76eLtp266XTuCF8STa+VZkXPPJ00WSRIgmJ73wjhXPZqIt9ofB0NVwbWOKnYzQ90SHJEd/hyBHkUaJAgAEiDxXDT00+zpJd+TKJrD6nWQxQZhB8I7vCRdD/Oxw61MYjpJCiEFmTV1l+cOLEytoTp17VOEunYlCZmDqn/qoUYI/8P9ZQsaJAgBEiB/QP+9Lb0YOhSQmIr/X75Vs1FME1qzmohSzqBVTzbfZFCnf1jsR2AAaiEFPxj3VK+knGrndOjcgMXI4wEfH/0VrbgJqobGWbewYyA=")));
    QCOMPARE(resultDeviceAlice.unrespondedSentStanzasCount, 10);
    QCOMPARE(resultDeviceAlice.unrespondedReceivedStanzasCount, 11);
    QCOMPARE(resultDeviceAlice.removalFromDeviceListDate, QDateTime(QDate(2022, 01, 01), QTime()));
}

void tst_QXmppOmemoMemoryStorage::testResetAll()
{
    m_omemoStorage.setOwnDevice(QXmppOmemoStorage::OwnDevice());

    QXmppOmemoStorage::SignedPreKeyPair signedPreKeyPair;
    signedPreKeyPair.creationDate = QDateTime(QDate(2022, 01, 01), QTime());
    signedPreKeyPair.data = QByteArrayLiteral("FaZmWjwqppAoMff72qTzUIktGUbi4pAmds1Cuh6OElmi");
    m_omemoStorage.addSignedPreKeyPair(1, signedPreKeyPair);

    m_omemoStorage.addPreKeyPairs({ { 1, QByteArrayLiteral("RZLgD0lmL2WpJbskbGKFRMZL4zqSSvU0rElmO7UwGSVt") },
                                    { 2, QByteArrayLiteral("3PGPNsf9P7pPitp9dt2uvZYT4HkxdHJAbWqLvOPXUeca") } });
    m_omemoStorage.addDevice(QStringLiteral("alice@example.org"),
                             123,
                             QXmppOmemoStorage::Device());

    m_omemoStorage.resetAll();

    auto future = m_omemoStorage.allData();
    QVERIFY(future.isFinished());
    auto result = future.result();
    QVERIFY(!result.ownDevice);
    QVERIFY(result.signedPreKeyPairs.isEmpty());
    QVERIFY(result.preKeyPairs.isEmpty());
    QVERIFY(result.devices.isEmpty());
}

QTEST_MAIN(tst_QXmppOmemoMemoryStorage)
#include "tst_qxmppomemomemorystorage.moc"
