/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Authors:
 *  Melvin Keskin
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "QXmppConstants.cpp"
#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppTrustMemoryStorage.h"

#include "util.h"
#include <QObject>
#include <QSet>

class tst_QXmppTrustMemoryStorage : public QObject
{
    Q_OBJECT

signals:
    void resultProcessed();

private slots:
    void testOwnKeys();
    void testKeys();
    void testkeysWithTrustLevels();
    void testTrustLevels();
    void testKeysForLaterTrustDecisions();

private:
    QXmppTrustMemoryStorage m_trustStorage;
};

void tst_QXmppTrustMemoryStorage::testOwnKeys()
{
    QSignalSpy resultProcessedSpy(this, &tst_QXmppTrustMemoryStorage::resultProcessed);

    m_trustStorage.addOwnKey(ns_ox, QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"));
    m_trustStorage.addOwnKey(ns_omemo, QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020"));

    auto future = m_trustStorage.ownKey(ns_ox);
    QXmpp::Private::await(future, [=](const auto &&result) {
        QCOMPARE(result, QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"));
        emit resultProcessed();
    });

    future = m_trustStorage.ownKey(ns_omemo);
    QXmpp::Private::await(future, [=](const auto &&result) {
        QCOMPARE(result, QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020"));
        emit resultProcessed();
    });

    m_trustStorage.removeOwnKey(ns_omemo);

    future = m_trustStorage.ownKey(ns_omemo);
    QXmpp::Private::await(future, [=](const auto &&result) {
        QVERIFY(result.isEmpty());
        emit resultProcessed();
    });

    while (resultProcessedSpy.count() < 3) {
        QVERIFY2(resultProcessedSpy.wait(), "Request timed out!");
    }
}

void tst_QXmppTrustMemoryStorage::testKeys()
{
    QSignalSpy resultProcessedSpy(this, &tst_QXmppTrustMemoryStorage::resultProcessed);

    m_trustStorage.addKeys(
        ns_ox,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"),
          QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020") },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4") },
        QXmppTrustStorage::ManuallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("59a027a56c96d619c5a281f7a09a3d05ae5762892c9cb3de2514c08f13dbbf7f"),
          QStringLiteral("ff578add1d8bb633c14f77a5f1fd2ae03bf3a942527c5cb97e42a428f345358c") });

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("b423f5088de9a924d51b31581723d850c7cc67d0a4fe6b267c3d301ff56d2413"),
          QStringLiteral("d9f849b6b828309c5f2c8df4f38fd891887da5aaa24a22c50d52f69b4a80817e") },
        QXmppTrustStorage::ManuallyDistrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("623548d3835c6d33ef5cb680f7944ef381cf712bf23a0119dabe5c4f252cd02f") },
        QXmppTrustStorage::Authenticated);

    auto future = m_trustStorage.keys(ns_ox);
    QXmpp::Private::await(future, [=](const auto &&result) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto resultSet = QSet(result.constBegin(), result.constEnd());
#else
        const auto resultSet = result.toSet();
#endif
        QCOMPARE(
            resultSet,
            QSet({ QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"),
                   QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020") }));
        emit resultProcessed();
    });

    future = m_trustStorage.keys(ns_omemo);
    QXmpp::Private::await(future, [=](const auto &&result) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto resultSet = QSet(result.constBegin(), result.constEnd());
#else
        const auto resultSet = result.toSet();
#endif
        QCOMPARE(
            resultSet,
            QSet({ QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"),
                   QStringLiteral("59a027a56c96d619c5a281f7a09a3d05ae5762892c9cb3de2514c08f13dbbf7f"),
                   QStringLiteral("ff578add1d8bb633c14f77a5f1fd2ae03bf3a942527c5cb97e42a428f345358c"),
                   QStringLiteral("b423f5088de9a924d51b31581723d850c7cc67d0a4fe6b267c3d301ff56d2413"),
                   QStringLiteral("d9f849b6b828309c5f2c8df4f38fd891887da5aaa24a22c50d52f69b4a80817e"),
                   QStringLiteral("623548d3835c6d33ef5cb680f7944ef381cf712bf23a0119dabe5c4f252cd02f") }));
        emit resultProcessed();
    });

    future = m_trustStorage.keys(ns_omemo, QStringLiteral("bob@example.com"));
    QXmpp::Private::await(future, [=](const auto &&result) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto resultSet = QSet(result.constBegin(), result.constEnd());
#else
        const auto resultSet = result.toSet();
#endif
        QCOMPARE(
            resultSet,
            QSet({ QStringLiteral("59a027a56c96d619c5a281f7a09a3d05ae5762892c9cb3de2514c08f13dbbf7f"),
                   QStringLiteral("ff578add1d8bb633c14f77a5f1fd2ae03bf3a942527c5cb97e42a428f345358c"),
                   QStringLiteral("b423f5088de9a924d51b31581723d850c7cc67d0a4fe6b267c3d301ff56d2413"),
                   QStringLiteral("d9f849b6b828309c5f2c8df4f38fd891887da5aaa24a22c50d52f69b4a80817e"),
                   QStringLiteral("623548d3835c6d33ef5cb680f7944ef381cf712bf23a0119dabe5c4f252cd02f") }));
        emit resultProcessed();
    });

    future = m_trustStorage.keys(ns_omemo, QStringLiteral("bob@example.com"), QXmppTrustStorage::ManuallyDistrusted);
    QXmpp::Private::await(future, [=](const auto &&result) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto resultSet = QSet(result.constBegin(), result.constEnd());
#else
        const auto resultSet = result.toSet();
#endif
        QCOMPARE(
            resultSet,
            QSet({ QStringLiteral("b423f5088de9a924d51b31581723d850c7cc67d0a4fe6b267c3d301ff56d2413"),
                   QStringLiteral("d9f849b6b828309c5f2c8df4f38fd891887da5aaa24a22c50d52f69b4a80817e")

            }));
        emit resultProcessed();
    });

    future = m_trustStorage.keys(ns_omemo, QStringLiteral("bob@example.com"), QXmppTrustStorage::AutomaticallyDistrusted);
    QXmpp::Private::await(future, [=](const auto &&result) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto resultSet = QSet(result.constBegin(), result.constEnd());
#else
        const auto resultSet = result.toSet();
#endif
        QCOMPARE(
            resultSet,
            QSet({ QStringLiteral("59a027a56c96d619c5a281f7a09a3d05ae5762892c9cb3de2514c08f13dbbf7f"),
                   QStringLiteral("ff578add1d8bb633c14f77a5f1fd2ae03bf3a942527c5cb97e42a428f345358c")

            }));
        emit resultProcessed();
    });

    m_trustStorage.removeKeys(
        ns_omemo,
        { QStringLiteral("b423f5088de9a924d51b31581723d850c7cc67d0a4fe6b267c3d301ff56d2413"),
          QStringLiteral("d9f849b6b828309c5f2c8df4f38fd891887da5aaa24a22c50d52f69b4a80817e") });

    future = m_trustStorage.keys(ns_omemo, QStringLiteral("bob@example.com"));
    QXmpp::Private::await(future, [=](const auto &&result) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto resultSet = QSet(result.constBegin(), result.constEnd());
#else
        const auto resultSet = result.toSet();
#endif
        QCOMPARE(
            resultSet,
            QSet({ QStringLiteral("59a027a56c96d619c5a281f7a09a3d05ae5762892c9cb3de2514c08f13dbbf7f"),
                   QStringLiteral("ff578add1d8bb633c14f77a5f1fd2ae03bf3a942527c5cb97e42a428f345358c"),
                   QStringLiteral("623548d3835c6d33ef5cb680f7944ef381cf712bf23a0119dabe5c4f252cd02f") }));
        emit resultProcessed();
    });

    m_trustStorage.removeKeys(ns_omemo);

    future = m_trustStorage.keys(ns_omemo);
    QXmpp::Private::await(future, [=](const auto &&result) {
        QVERIFY(result.isEmpty());
        emit resultProcessed();
    });

    m_trustStorage.removeKeys();

    future = m_trustStorage.keys(ns_ox);
    QXmpp::Private::await(future, [=](const auto &&result) {
        QVERIFY(result.isEmpty());
        emit resultProcessed();
    });

    while (resultProcessedSpy.count() < 8) {
        QVERIFY2(resultProcessedSpy.wait(), "Request timed out!");
    }
}

void tst_QXmppTrustMemoryStorage::testkeysWithTrustLevels()
{
    QSignalSpy resultProcessedSpy(this, &tst_QXmppTrustMemoryStorage::resultProcessed);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("a3f4ba508a328bfc00db8107d6d6117a58d020b457182a1a95e26f4cfb8f6a33"),
          QStringLiteral("cfedf544fa490ce691976c03c851964bda354c52594418bc2d11983f75d89ac6") });

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("7e00dcc83c9535604c246a6e154b2e4ad5605f214e8218f4c0f0a5b62ff78bd4"),
          QStringLiteral("6b33a759254ce2835d2f1351b7ee08b1de67d2c476338a0d6f93a163678dc76a") },
        QXmppTrustStorage::ManuallyDistrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"),
          QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020") },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("231a55dd686b14a5c693f169a9ccc4a3331a7d43f3a70ab7c2289365fe4c7311"),
          QStringLiteral("01113aa7c47c75510f00f879e89979e03e78c062f288ef62a9d07685bbc7f252") },
        QXmppTrustStorage::ManuallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95"),
          QStringLiteral("44824eb8356c06b4d49d4ed9909636b6043ae153183eb6fdb6918baa24718c41") },
        QXmppTrustStorage::Authenticated);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("182038c37d89e067e1897e07dca63c04ed9d8c5b2f76cdb2a3c086ffd2c6c78b"),
          QStringLiteral("416aed9043a8a9f7d25f75325e23d33f1a0535fb120576b290958a60ca8a4e62") },
        QXmppTrustStorage::Authenticated);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QStringLiteral("b0e67b606d02b4660abb2ba89c3c74b6c010541a1a192e6949e178caecd91749") },
        QXmppTrustStorage::AutomaticallyTrusted);

    auto futureList = m_trustStorage.trustedKeys(ns_omemo, QStringLiteral("alice@example.org"));
    QXmpp::Private::await(futureList, [&](const auto &&result) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto resultSet = QSet(result.constBegin(), result.constEnd());
#else
        const auto resultSet = result.toSet();
#endif
        QCOMPARE(
            resultSet,
            QSet({
                QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"),
                QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020"),
                QStringLiteral("231a55dd686b14a5c693f169a9ccc4a3331a7d43f3a70ab7c2289365fe4c7311"),
                QStringLiteral("01113aa7c47c75510f00f879e89979e03e78c062f288ef62a9d07685bbc7f252"),
                QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95"),
                QStringLiteral("44824eb8356c06b4d49d4ed9909636b6043ae153183eb6fdb6918baa24718c41"),
            }));
        emit resultProcessed();
    });

    futureList = m_trustStorage.unauthenticatedKeys(ns_omemo, QStringLiteral("alice@example.org"));
    QXmpp::Private::await(futureList, [&](const auto &&result) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto resultSet = QSet(result.constBegin(), result.constEnd());
#else
        const auto resultSet = result.toSet();
#endif
        QCOMPARE(
            resultSet,
            QSet({ QStringLiteral("a3f4ba508a328bfc00db8107d6d6117a58d020b457182a1a95e26f4cfb8f6a33"),
                   QStringLiteral("cfedf544fa490ce691976c03c851964bda354c52594418bc2d11983f75d89ac6"),
                   QStringLiteral("7e00dcc83c9535604c246a6e154b2e4ad5605f214e8218f4c0f0a5b62ff78bd4"),
                   QStringLiteral("6b33a759254ce2835d2f1351b7ee08b1de67d2c476338a0d6f93a163678dc76a"),
                   QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"),
                   QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020"),
                   QStringLiteral("231a55dd686b14a5c693f169a9ccc4a3331a7d43f3a70ab7c2289365fe4c7311"),
                   QStringLiteral("01113aa7c47c75510f00f879e89979e03e78c062f288ef62a9d07685bbc7f252") }));
        emit resultProcessed();
    });

    const auto futureHashMap = m_trustStorage.authenticatedKeys(ns_omemo);
    QXmpp::Private::await(futureHashMap, [&](const auto &&result) {
        QCOMPARE(
            result,
            QMultiHash({ std::pair(
                             QStringLiteral("alice@example.org"),
                             QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95")),
                         std::pair(
                             QStringLiteral("alice@example.org"),
                             QStringLiteral("44824eb8356c06b4d49d4ed9909636b6043ae153183eb6fdb6918baa24718c41")),
                         std::pair(
                             QStringLiteral("bob@example.com"),
                             QStringLiteral("182038c37d89e067e1897e07dca63c04ed9d8c5b2f76cdb2a3c086ffd2c6c78b")),
                         std::pair(
                             QStringLiteral("bob@example.com"),
                             QStringLiteral("416aed9043a8a9f7d25f75325e23d33f1a0535fb120576b290958a60ca8a4e62")) }));
        emit resultProcessed();
    });

    futureList = m_trustStorage.keyOwnersWithAuthenticatedKeys(ns_omemo);
    QXmpp::Private::await(futureList, [&](const auto &&result) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto resultSet = QSet(result.constBegin(), result.constEnd());
#else
        const auto resultSet = result.toSet();
#endif
        QCOMPARE(
            resultSet,
            QSet({ QStringLiteral("alice@example.org"),
                   QStringLiteral("bob@example.com") }));
        emit resultProcessed();
    });

    while (resultProcessedSpy.count() < 4) {
        QVERIFY2(resultProcessedSpy.wait(), "Request timed out!");
    }

    m_trustStorage.removeKeys();
}

void tst_QXmppTrustMemoryStorage::testTrustLevels()
{
    QSignalSpy resultProcessedSpy(this, &tst_QXmppTrustMemoryStorage::resultProcessed);

    m_trustStorage.addKeys(
        ns_ox,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("95b1ce6700692998e6187ad7d220173864742957f397d44b771f26f98a861207") },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95"),
          QStringLiteral("254e294fb22fa6282d97eed013cec192ae2bfc2fe6848d450a36395a68dbb708") },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4") },
        QXmppTrustStorage::ManuallyTrusted);

    auto futureTrustLevel = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95"));

    QXmpp::Private::await(futureTrustLevel, [&](const auto &&result) {
        QCOMPARE(result, QXmppTrustStorage::AutomaticallyTrusted);
        emit resultProcessed();
    });

    m_trustStorage.setTrustLevel(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95"),
          QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4") },
        QXmppTrustStorage::Authenticated);

    futureTrustLevel = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95"));

    QXmpp::Private::await(futureTrustLevel, [&](const auto &&result) {
        QCOMPARE(result, QXmppTrustStorage::Authenticated);
        emit resultProcessed();
    });

    futureTrustLevel = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"));

    QXmpp::Private::await(futureTrustLevel, [&](const auto &&result) {
        QCOMPARE(result, QXmppTrustStorage::Authenticated);
        emit resultProcessed();
    });

    // Set the trust level of a key that is not stored.
    // It is added to the storage automatically.
    m_trustStorage.setTrustLevel(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("f70ea83e32b219200b77d807abbb0d39d380903e5b1d454e28008db3cf458e40") },
        QXmppTrustStorage::Authenticated);

    futureTrustLevel = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        QStringLiteral("f70ea83e32b219200b77d807abbb0d39d380903e5b1d454e28008db3cf458e40"));

    QXmpp::Private::await(futureTrustLevel, [&](const auto &&result) {
        QCOMPARE(result, QXmppTrustStorage::Authenticated);
        emit resultProcessed();
    });

    // Try to retrieve the trust level of a key that is not stored.
    // The default value is returned.
    futureTrustLevel = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        QStringLiteral("5972f81037f35066d53d05a34fda6605e882a42073619bf795401a8fe51b3f21"));

    QXmpp::Private::await(futureTrustLevel, [&](const auto &&result) {
        QCOMPARE(result, QXmppTrustStorage::AutomaticallyDistrusted);
        emit resultProcessed();
    });

    while (resultProcessedSpy.count() < 5) {
        QVERIFY2(resultProcessedSpy.wait(), "Request timed out!");
    }

    m_trustStorage.removeKeys();
}

void tst_QXmppTrustMemoryStorage::testKeysForLaterTrustDecisions()
{
    QSignalSpy resultProcessedSpy(this, &tst_QXmppTrustMemoryStorage::resultProcessed);

    // The key ef2d6dd0b9e63417972646b8dd77a314f2eb2ad425485013ad89b2ef11da2985
    // is passed for later authentication and distrusting.
    // Thus, it is stored for later distrusting.
    m_trustStorage.addKeysForLaterTrustDecisions(
        ns_omemo,
        QStringLiteral("329e98e30385dda31c977f256ff54d6dd3c5f6e70616a4b1d9ec9a12cab21ca1"),
        QStringLiteral("alice@example.org"),
        { QStringLiteral("5a5e7765c85bb40b4265008744e883db45bb527293420590ae34c7015370d627"),
          QStringLiteral("411d398eb69becf16448b86d773c97acf7c2aa190d0980ab9560136813137a71"),
          QStringLiteral("f634283aa7982c98ac0bb3aabd5a501c7f07d7821f581529fb26a07f6692ebe7"),
          QStringLiteral("4584b1af7f566c4f6f66b29faac6add5f57ab53c4db4b77e5b8812671014c45b"),
          QStringLiteral("ef2d6dd0b9e63417972646b8dd77a314f2eb2ad425485013ad89b2ef11da2985") },
        { QStringLiteral("981f7c861755a6cfbeb2452ecb84c6cbf569e914172c93b82467fce850148f27"),
          QStringLiteral("ef2d6dd0b9e63417972646b8dd77a314f2eb2ad425485013ad89b2ef11da2985") });

    m_trustStorage.removeKeysForLaterTrustDecisions(
        ns_omemo,
        { QStringLiteral("f634283aa7982c98ac0bb3aabd5a501c7f07d7821f581529fb26a07f6692ebe7"),
          QStringLiteral("4584b1af7f566c4f6f66b29faac6add5f57ab53c4db4b77e5b8812671014c45b") },
        true);

    m_trustStorage.addKeysForLaterTrustDecisions(
        ns_omemo,
        QStringLiteral("329e98e30385dda31c977f256ff54d6dd3c5f6e70616a4b1d9ec9a12cab21ca1"),
        QStringLiteral("carol@example.net"),
        { QStringLiteral("1a04ea7912e9d4cf8c11e9f3150ca6da8a9eafd3df1ee912e1bac90d09790111") },
        QList<QString>());

    m_trustStorage.addKeysForLaterTrustDecisions(
        ns_omemo,
        QStringLiteral("5e91ac4f2763d2e6d2d5e30c8577b2ed8568495862ffa7abe9f6fe08adad3800"),
        QStringLiteral("carol@example.net"),
        { QStringLiteral("1a04ea7912e9d4cf8c11e9f3150ca6da8a9eafd3df1ee912e1bac90d09790111") },
        QList<QString>());

    m_trustStorage.addKeysForLaterTrustDecisions(
        ns_omemo,
        QStringLiteral("20be62c03430aae1fbca36f94408883fe9ef622054b0d097b4a07c22929cf505"),
        QStringLiteral("carol@example.net"),
        QList<QString>(),
        { QStringLiteral("b03ea296e80405e2b13ec7431325f8dcb48628729677930511d853fb8469b310"),
          QStringLiteral("5f9b49d43e6b11e69e404f1eaa104a023e0a5191987b7c7e88789f693058d643") });

    m_trustStorage.addKeysForLaterTrustDecisions(
        ns_omemo,
        QStringLiteral("68677212ec2243bea457a7649fae063559478315e963442047a6503d6d6cd8d0"),
        QStringLiteral("alice@example.org"),
        { QStringLiteral("7dd98c7d5540e4b8032e3d897c53a023e27ab91b8cd26af777e5ceef0690fddf"),
          QStringLiteral("6f96ba6da31851efc3a93625bcef8ef399934798052c47dcd7207ddb5b5e9cfd") },
        QList<QString>());

    m_trustStorage.removeKeysForLaterTrustDecisions(
        ns_omemo,
        { QStringLiteral("5e91ac4f2763d2e6d2d5e30c8577b2ed8568495862ffa7abe9f6fe08adad3800"),
          QStringLiteral("68677212ec2243bea457a7649fae063559478315e963442047a6503d6d6cd8d0") });

    auto future = m_trustStorage.keysForLaterTrustDecisions(
        ns_omemo,
        { QStringLiteral("68677212ec2243bea457a7649fae063559478315e963442047a6503d6d6cd8d0") },
        true);

    QXmpp::Private::await(future, [=](const auto &&result) {
        QVERIFY(result.isEmpty());
        emit resultProcessed();
    });

    future = m_trustStorage.keysForLaterTrustDecisions(
        ns_omemo,
        { QStringLiteral("329e98e30385dda31c977f256ff54d6dd3c5f6e70616a4b1d9ec9a12cab21ca1") },
        true);

    QXmpp::Private::await(future, [=](const auto &&result) {
        QCOMPARE(
            result,
            QMultiHash({ std::pair(
                             QStringLiteral("alice@example.org"),
                             QStringLiteral("5a5e7765c85bb40b4265008744e883db45bb527293420590ae34c7015370d627")),
                         std::pair(
                             QStringLiteral("alice@example.org"),
                             QStringLiteral("411d398eb69becf16448b86d773c97acf7c2aa190d0980ab9560136813137a71")),
                         std::pair(
                             QStringLiteral("carol@example.net"),
                             QStringLiteral("1a04ea7912e9d4cf8c11e9f3150ca6da8a9eafd3df1ee912e1bac90d09790111")) }));
        emit resultProcessed();
    });

    future = m_trustStorage.keysForLaterTrustDecisions(
        ns_omemo,
        { QStringLiteral("329e98e30385dda31c977f256ff54d6dd3c5f6e70616a4b1d9ec9a12cab21ca1"),
          QStringLiteral("20be62c03430aae1fbca36f94408883fe9ef622054b0d097b4a07c22929cf505") },
        false);

    QXmpp::Private::await(future, [=](const auto &&result) {
        QCOMPARE(
            result,
            QMultiHash({ std::pair(
                             QStringLiteral("alice@example.org"),
                             QStringLiteral("981f7c861755a6cfbeb2452ecb84c6cbf569e914172c93b82467fce850148f27")),
                         std::pair(
                             QStringLiteral("alice@example.org"),
                             QStringLiteral("ef2d6dd0b9e63417972646b8dd77a314f2eb2ad425485013ad89b2ef11da2985")),
                         std::pair(
                             QStringLiteral("carol@example.net"),
                             QStringLiteral("b03ea296e80405e2b13ec7431325f8dcb48628729677930511d853fb8469b310")),
                         std::pair(
                             QStringLiteral("carol@example.net"),
                             QStringLiteral("5f9b49d43e6b11e69e404f1eaa104a023e0a5191987b7c7e88789f693058d643")) }));
        emit resultProcessed();
    });

    // Invert the trust in Carol's key
    // 1a04ea7912e9d4cf8c11e9f3150ca6da8a9eafd3df1ee912e1bac90d09790111 by the
    // sending endpoint with the key ID
    // 329e98e30385dda31c977f256ff54d6dd3c5f6e70616a4b1d9ec9a12cab21ca1.
    m_trustStorage.addKeysForLaterTrustDecisions(
        ns_omemo,
        QStringLiteral("329e98e30385dda31c977f256ff54d6dd3c5f6e70616a4b1d9ec9a12cab21ca1"),
        QStringLiteral("carol@example.net"),
        QList<QString>(),
        { QStringLiteral("1a04ea7912e9d4cf8c11e9f3150ca6da8a9eafd3df1ee912e1bac90d09790111") });

    future = m_trustStorage.keysForLaterTrustDecisions(
        ns_omemo,
        { QStringLiteral("329e98e30385dda31c977f256ff54d6dd3c5f6e70616a4b1d9ec9a12cab21ca1") },
        true);

    QXmpp::Private::await(future, [=](const auto &&result) {
        QCOMPARE(
            result,
            QMultiHash({ std::pair(
                             QStringLiteral("alice@example.org"),
                             QStringLiteral("5a5e7765c85bb40b4265008744e883db45bb527293420590ae34c7015370d627")),
                         std::pair(
                             QStringLiteral("alice@example.org"),
                             QStringLiteral("411d398eb69becf16448b86d773c97acf7c2aa190d0980ab9560136813137a71")) }));
        emit resultProcessed();
    });

    while (resultProcessedSpy.count() < 4) {
        QVERIFY2(resultProcessedSpy.wait(), "Request timed out!");
    }

    m_trustStorage.removeKeys();
}

QTEST_MAIN(tst_QXmppTrustMemoryStorage)
#include "tst_qxmpptrustmemorystorage.moc"
