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
#include "QXmppTrustMemoryStorage.h"
#include "QXmppTrustMessageKeyOwner.h"

#include "util.h"

class tst_QXmppTrustMemoryStorage : public QObject
{
    Q_OBJECT

private slots:
    void testSecurityPolicies();
    void testOwnKeys();
    void testKeys();
    void testTrustLevels();
    void testKeysForPostponedTrustDecisions();

private:
    QXmppTrustMemoryStorage m_trustStorage;
};

void tst_QXmppTrustMemoryStorage::testSecurityPolicies()
{
    auto future = m_trustStorage.securityPolicy(ns_ox);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(result, QXmppTrustStorage::NoSecurityPolicy);

    m_trustStorage.setSecurityPolicies(ns_ox, QXmppTrustStorage::Toakafa);
    m_trustStorage.setSecurityPolicies(ns_omemo, QXmppTrustStorage::Toakafa);

    future = m_trustStorage.securityPolicy(ns_ox);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::Toakafa);

    future = m_trustStorage.securityPolicy(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::Toakafa);

    m_trustStorage.setSecurityPolicies(ns_ox);

    future = m_trustStorage.securityPolicy(ns_ox);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::NoSecurityPolicy);

    m_trustStorage.setSecurityPolicies();

    future = m_trustStorage.securityPolicy(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::NoSecurityPolicy);
}

void tst_QXmppTrustMemoryStorage::testOwnKeys()
{
    m_trustStorage.addOwnKey(ns_ox, QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"));
    m_trustStorage.addOwnKey(ns_omemo, QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020"));

    // own OX key
    auto future = m_trustStorage.ownKey(ns_ox);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(result, QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"));

    // own OMEMO key
    future = m_trustStorage.ownKey(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020"));

    m_trustStorage.removeOwnKey(ns_omemo);

    // no stored own OMEMO key
    future = m_trustStorage.ownKey(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QVERIFY(result.isEmpty());
}

void tst_QXmppTrustMemoryStorage::testKeys()
{
    // Store keys with the default trust level.
    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("59a027a56c96d619c5a281f7a09a3d05ae5762892c9cb3de2514c08f13dbbf7f"),
          QStringLiteral("ff578add1d8bb633c14f77a5f1fd2ae03bf3a942527c5cb97e42a428f345358c") });

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4") },
        QXmppTrustStorage::ManuallyDistrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("0a27a6a7864dcd12719d10fe936f2f0227b491726bc25e08adb7c3cbb9fa3b11") },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("aef49705177e10808c850bd581c444409c713fe4f810199a8b89981c17c94068") },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("b423f5088de9a924d51b31581723d850c7cc67d0a4fe6b267c3d301ff56d2413"),
          QStringLiteral("d9f849b6b828309c5f2c8df4f38fd891887da5aaa24a22c50d52f69b4a80817e") },
        QXmppTrustStorage::ManuallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("623548d3835c6d33ef5cb680f7944ef381cf712bf23a0119dabe5c4f252cd02f") },
        QXmppTrustStorage::Authenticated);

    m_trustStorage.addKeys(
        ns_ox,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"),
          QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020") },
        QXmppTrustStorage::Authenticated);

    QMultiHash<QString, QString> automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                   QStringLiteral("59a027a56c96d619c5a281f7a09a3d05ae5762892c9cb3de2514c08f13dbbf7f") },
                                                                 { QStringLiteral("alice@example.org"),
                                                                   QStringLiteral("ff578add1d8bb633c14f77a5f1fd2ae03bf3a942527c5cb97e42a428f345358c") } };
    QMultiHash<QString, QString> manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                              QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4") } };
    QMultiHash<QString, QString> automaticallyTrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                QStringLiteral("0a27a6a7864dcd12719d10fe936f2f0227b491726bc25e08adb7c3cbb9fa3b11") },
                                                              { QStringLiteral("bob@example.com"),
                                                                QStringLiteral("aef49705177e10808c850bd581c444409c713fe4f810199a8b89981c17c94068") } };
    QMultiHash<QString, QString> manuallyTrustedKeys = { { QStringLiteral("bob@example.com"),
                                                           QStringLiteral("b423f5088de9a924d51b31581723d850c7cc67d0a4fe6b267c3d301ff56d2413") },
                                                         { QStringLiteral("bob@example.com"),
                                                           QStringLiteral("d9f849b6b828309c5f2c8df4f38fd891887da5aaa24a22c50d52f69b4a80817e") } };
    QMultiHash<QString, QString> authenticatedKeys = { { QStringLiteral("bob@example.com"),
                                                         QStringLiteral("623548d3835c6d33ef5cb680f7944ef381cf712bf23a0119dabe5c4f252cd02f") } };

    // all OMEMO keys
    auto future = m_trustStorage.keys(ns_omemo);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    QXmppTrustStorage::AutomaticallyDistrusted,
                    automaticallyDistrustedKeys),
                std::pair(
                    QXmppTrustStorage::ManuallyDistrusted,
                    manuallyDistrustedKeys),
                std::pair(
                    QXmppTrustStorage::AutomaticallyTrusted,
                    automaticallyTrustedKeys),
                std::pair(
                    QXmppTrustStorage::ManuallyTrusted,
                    manuallyTrustedKeys),
                std::pair(
                    QXmppTrustStorage::Authenticated,
                    authenticatedKeys) }));

    // automatically trusted and authenticated OMEMO keys
    future = m_trustStorage.keys(ns_omemo, QXmppTrustStorage::AutomaticallyTrusted | QXmppTrustStorage::Authenticated);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    QXmppTrustStorage::AutomaticallyTrusted,
                    automaticallyTrustedKeys),
                std::pair(
                    QXmppTrustStorage::Authenticated,
                    authenticatedKeys) }));

    m_trustStorage.removeKeys(ns_omemo,
                              { QStringLiteral("59a027a56c96d619c5a281f7a09a3d05ae5762892c9cb3de2514c08f13dbbf7f"),
                                QStringLiteral("0a27a6a7864dcd12719d10fe936f2f0227b491726bc25e08adb7c3cbb9fa3b11") });

    automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                      QStringLiteral("ff578add1d8bb633c14f77a5f1fd2ae03bf3a942527c5cb97e42a428f345358c") } };
    automaticallyTrustedKeys = { { QStringLiteral("bob@example.com"),
                                   QStringLiteral("aef49705177e10808c850bd581c444409c713fe4f810199a8b89981c17c94068") } };

    // all OMEMO keys after removal
    future = m_trustStorage.keys(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    QXmppTrustStorage::AutomaticallyDistrusted,
                    automaticallyDistrustedKeys),
                std::pair(
                    QXmppTrustStorage::ManuallyDistrusted,
                    manuallyDistrustedKeys),
                std::pair(
                    QXmppTrustStorage::AutomaticallyTrusted,
                    automaticallyTrustedKeys),
                std::pair(
                    QXmppTrustStorage::ManuallyTrusted,
                    manuallyTrustedKeys),
                std::pair(
                    QXmppTrustStorage::Authenticated,
                    authenticatedKeys) }));

    m_trustStorage.removeKeys(ns_omemo);

    // no stored OMEMO keys
    future = m_trustStorage.keys(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QVERIFY(result.isEmpty());

    authenticatedKeys = { { QStringLiteral("alice@example.org"),
                            QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4") },
                          { QStringLiteral("alice@example.org"),
                            QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020") } };

    // remaining OX keys
    future = m_trustStorage.keys(ns_ox);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
            QXmppTrustStorage::Authenticated,
            authenticatedKeys) }));

    m_trustStorage.removeKeys();

    // no stored OX keys
    future = m_trustStorage.keys(ns_ox);
    QVERIFY(future.isFinished());
    result = future.result();
    QVERIFY(result.isEmpty());
}

void tst_QXmppTrustMemoryStorage::testTrustLevels()
{
    m_trustStorage.addKeys(
        ns_ox,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95") },
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

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("f44e75946def566527f0233bfc021c98894b3f61cf97a028d3f5527f8553fe80") },
        QXmppTrustStorage::AutomaticallyTrusted);

    auto future = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95"));
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(result, QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.setTrustLevel(
        ns_omemo,
        { { QStringLiteral("alice@example.org"),
            QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95") },
          { QStringLiteral("bob@example.com"),
            QStringLiteral("f44e75946def566527f0233bfc021c98894b3f61cf97a028d3f5527f8553fe80") } },
        QXmppTrustStorage::Authenticated);

    future = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95"));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::Authenticated);

    future = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("f44e75946def566527f0233bfc021c98894b3f61cf97a028d3f5527f8553fe80"));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::Authenticated);

    // Set the trust level of a key that is not stored yet.
    // It is added to the storage automatically.
    m_trustStorage.setTrustLevel(
        ns_omemo,
        { { QStringLiteral("alice@example.org"),
            QStringLiteral("f70ea83e32b219200b77d807abbb0d39d380903e5b1d454e28008db3cf458e40") } },
        QXmppTrustStorage::ManuallyTrusted);

    future = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("f70ea83e32b219200b77d807abbb0d39d380903e5b1d454e28008db3cf458e40"));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::ManuallyTrusted);

    // Try to retrieve the trust level of a key that is not stored yet.
    // The default value is returned.
    future = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("5972f81037f35066d53d05a34fda6605e882a42073619bf795401a8fe51b3f21"));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::AutomaticallyDistrusted);

    // Set the trust levels of all authenticated keys belonging to Alice and
    // Bob.
    m_trustStorage.setTrustLevel(
        ns_omemo,
        { QStringLiteral("alice@example.org"),
          QStringLiteral("bob@example.com") },
        QXmppTrustStorage::Authenticated,
        QXmppTrustStorage::ManuallyDistrusted);

    future = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("019fdc1783ab50e20b28ed604017fada941ebce8412763721e75cbc0ce050d95"));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::ManuallyDistrusted);

    future = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("f44e75946def566527f0233bfc021c98894b3f61cf97a028d3f5527f8553fe80"));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::ManuallyDistrusted);

    // Verify that the default trust level is returned for an unknown key.
    future = m_trustStorage.trustLevel(
        ns_omemo,
        QStringLiteral("c04d3a1b07fc7f80ef0cb143a1a0ac1acf226dc5237fce1620e0361408cf237a"));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::AutomaticallyDistrusted);

    m_trustStorage.removeKeys();
}

void tst_QXmppTrustMemoryStorage::testKeysForPostponedTrustDecisions()
{
    QXmppTrustMessageKeyOwner keyOwnerAlice;
    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    keyOwnerAlice.setTrustedKeys({ QStringLiteral("5a5e7765c85bb40b4265008744e883db45bb527293420590ae34c7015370d627"),
                                   QStringLiteral("411d398eb69becf16448b86d773c97acf7c2aa190d0980ab9560136813137a71"),
                                   QStringLiteral("ef2d6dd0b9e63417972646b8dd77a314f2eb2ad425485013ad89b2ef11da2985") });
    keyOwnerAlice.setDistrustedKeys({ QStringLiteral("981f7c861755a6cfbeb2452ecb84c6cbf569e914172c93b82467fce850148f27"),
                                      QStringLiteral("ef2d6dd0b9e63417972646b8dd77a314f2eb2ad425485013ad89b2ef11da2985") });

    QXmppTrustMessageKeyOwner keyOwnerBobTrustedKeys;
    keyOwnerBobTrustedKeys.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBobTrustedKeys.setTrustedKeys({ QStringLiteral("1a04ea7912e9d4cf8c11e9f3150ca6da8a9eafd3df1ee912e1bac90d09790111") });

    // The key ef2d6dd0b9e63417972646b8dd77a314f2eb2ad425485013ad89b2ef11da2985
    // is passed for both postponed authentication and distrusting.
    // Thus, it is only stored for postponed distrusting.
    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QStringLiteral("329e98e30385dda31c977f256ff54d6dd3c5f6e70616a4b1d9ec9a12cab21ca1"),
                                                     { keyOwnerAlice, keyOwnerBobTrustedKeys });

    QXmppTrustMessageKeyOwner keyOwnerBobDistrustedKeys;
    keyOwnerBobDistrustedKeys.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBobDistrustedKeys.setDistrustedKeys({ QStringLiteral("b03ea296e80405e2b13ec7431325f8dcb48628729677930511d853fb8469b310"),
                                                  QStringLiteral("5f9b49d43e6b11e69e404f1eaa104a023e0a5191987b7c7e88789f693058d643") });

    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QStringLiteral("20be62c03430aae1fbca36f94408883fe9ef622054b0d097b4a07c22929cf505"),
                                                     { keyOwnerBobDistrustedKeys });

    QXmppTrustMessageKeyOwner keyOwnerCarol;
    keyOwnerCarol.setJid(QStringLiteral("carol@example.net"));
    keyOwnerCarol.setTrustedKeys({ QStringLiteral("59c2fe70432911e2be769aa0dd77776a672dcf03fc87632ac17704cc57fa2e95"),
                                   QStringLiteral("6c7dd1df5cf437decad5f5301b7f9b741ad53ee0df5e0b906a91ee7647dae671") });
    keyOwnerCarol.setDistrustedKeys({ QStringLiteral("3740764ad1ca935fec970835af3c9b4c5ce3760ec50a173eddc5e64d4feb4bc8"),
                                      QStringLiteral("c2c10ddf65070a236362a4c6fc9eb7858e0dbbcb594f8d8d8b5171ae0c914398") });

    m_trustStorage.addKeysForPostponedTrustDecisions(ns_ox,
                                                     QStringLiteral("20be62c03430aae1fbca36f94408883fe9ef622054b0d097b4a07c22929cf505"),
                                                     { keyOwnerCarol });

    QMultiHash<QString, QString> trustedKeys = { { QStringLiteral("alice@example.org"),
                                                   QStringLiteral("5a5e7765c85bb40b4265008744e883db45bb527293420590ae34c7015370d627") },
                                                 { QStringLiteral("alice@example.org"),
                                                   QStringLiteral("411d398eb69becf16448b86d773c97acf7c2aa190d0980ab9560136813137a71") },
                                                 { QStringLiteral("bob@example.com"),
                                                   QStringLiteral("1a04ea7912e9d4cf8c11e9f3150ca6da8a9eafd3df1ee912e1bac90d09790111") } };
    QMultiHash<QString, QString> distrustedKeys = { { QStringLiteral("alice@example.org"),
                                                      QStringLiteral("981f7c861755a6cfbeb2452ecb84c6cbf569e914172c93b82467fce850148f27") },
                                                    { QStringLiteral("alice@example.org"),
                                                      QStringLiteral("ef2d6dd0b9e63417972646b8dd77a314f2eb2ad425485013ad89b2ef11da2985") } };

    auto future = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                                { QStringLiteral("329e98e30385dda31c977f256ff54d6dd3c5f6e70616a4b1d9ec9a12cab21ca1") });
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    true,
                    trustedKeys),
                std::pair(
                    false,
                    distrustedKeys) }));

    distrustedKeys = { { QStringLiteral("alice@example.org"),
                         QStringLiteral("981f7c861755a6cfbeb2452ecb84c6cbf569e914172c93b82467fce850148f27") },
                       { QStringLiteral("alice@example.org"),
                         QStringLiteral("ef2d6dd0b9e63417972646b8dd77a314f2eb2ad425485013ad89b2ef11da2985") },
                       { QStringLiteral("bob@example.com"),
                         QStringLiteral("b03ea296e80405e2b13ec7431325f8dcb48628729677930511d853fb8469b310") },
                       { QStringLiteral("bob@example.com"),
                         QStringLiteral("5f9b49d43e6b11e69e404f1eaa104a023e0a5191987b7c7e88789f693058d643") } };

    future = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                           { QStringLiteral("329e98e30385dda31c977f256ff54d6dd3c5f6e70616a4b1d9ec9a12cab21ca1"),
                                                             QStringLiteral("20be62c03430aae1fbca36f94408883fe9ef622054b0d097b4a07c22929cf505") });
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    true,
                    trustedKeys),
                std::pair(
                    false,
                    distrustedKeys) }));

    // Retrieve all keys.
    future = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    true,
                    trustedKeys),
                std::pair(
                    false,
                    distrustedKeys) }));

    keyOwnerBobTrustedKeys.setTrustedKeys({ QStringLiteral("b03ea296e80405e2b13ec7431325f8dcb48628729677930511d853fb8469b310") });

    // Invert the trust in Bob's key
    // b03ea296e80405e2b13ec7431325f8dcb48628729677930511d853fb8469b310 for the
    // sending endpoint with the key
    // 20be62c03430aae1fbca36f94408883fe9ef622054b0d097b4a07c22929cf505.
    m_trustStorage.addKeysForPostponedTrustDecisions(
        ns_omemo,
        QStringLiteral("20be62c03430aae1fbca36f94408883fe9ef622054b0d097b4a07c22929cf505"),
        { keyOwnerBobTrustedKeys });

    trustedKeys = { { QStringLiteral("bob@example.com"),
                      QStringLiteral("b03ea296e80405e2b13ec7431325f8dcb48628729677930511d853fb8469b310") } };
    distrustedKeys = { { QStringLiteral("bob@example.com"),
                         QStringLiteral("5f9b49d43e6b11e69e404f1eaa104a023e0a5191987b7c7e88789f693058d643") } };

    future = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                           { QStringLiteral("20be62c03430aae1fbca36f94408883fe9ef622054b0d097b4a07c22929cf505") });
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    true,
                    trustedKeys),
                std::pair(
                    false,
                    distrustedKeys) }));

    m_trustStorage.removeKeysForPostponedTrustDecisions(ns_omemo,
                                                        { QStringLiteral("329e98e30385dda31c977f256ff54d6dd3c5f6e70616a4b1d9ec9a12cab21ca1") });

    future = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    true,
                    trustedKeys),
                std::pair(
                    false,
                    distrustedKeys) }));

    // Remove all OMEMO keys including those stored for sender key
    // 20be62c03430aae1fbca36f94408883fe9ef622054b0d097b4a07c22929cf505.
    m_trustStorage.removeKeysForPostponedTrustDecisions(ns_omemo);

    future = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QVERIFY(result.isEmpty());

    trustedKeys = { { QStringLiteral("carol@example.net"),
                      QStringLiteral("59c2fe70432911e2be769aa0dd77776a672dcf03fc87632ac17704cc57fa2e95") },
                    { QStringLiteral("carol@example.net"),
                      QStringLiteral("6c7dd1df5cf437decad5f5301b7f9b741ad53ee0df5e0b906a91ee7647dae671") } };
    distrustedKeys = { { QStringLiteral("carol@example.net"),
                         QStringLiteral("3740764ad1ca935fec970835af3c9b4c5ce3760ec50a173eddc5e64d4feb4bc8") },
                       { QStringLiteral("carol@example.net"),
                         QStringLiteral("c2c10ddf65070a236362a4c6fc9eb7858e0dbbcb594f8d8d8b5171ae0c914398") } };

    // remaining OX keys
    future = m_trustStorage.keysForPostponedTrustDecisions(ns_ox);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    true,
                    trustedKeys),
                std::pair(
                    false,
                    distrustedKeys) }));

    m_trustStorage.removeKeysForPostponedTrustDecisions();

    // no OX keys
    future = m_trustStorage.keysForPostponedTrustDecisions(ns_ox);
    QVERIFY(future.isFinished());
    result = future.result();
    QVERIFY(result.isEmpty());

    m_trustStorage.removeKeys();
}

QTEST_MAIN(tst_QXmppTrustMemoryStorage)
#include "tst_qxmpptrustmemorystorage.moc"
