// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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
    m_trustStorage.addOwnKey(ns_ox, QByteArray::fromBase64(QByteArrayLiteral("aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=")));
    m_trustStorage.addOwnKey(ns_omemo, QByteArray::fromBase64(QByteArrayLiteral("IhpPjiKLchgrAG5cpSfTvdzPjZ5v6vTOluHEUehkgCA=")));

    // own OX key
    auto future = m_trustStorage.ownKey(ns_ox);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(result, QByteArray::fromBase64(QByteArrayLiteral("aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=")));

    // own OMEMO key
    future = m_trustStorage.ownKey(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QByteArray::fromBase64(QByteArrayLiteral("IhpPjiKLchgrAG5cpSfTvdzPjZ5v6vTOluHEUehkgCA=")));

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
        { QByteArray::fromBase64(QByteArrayLiteral("WaAnpWyW1hnFooH3oJo9Ba5XYoksnLPeJRTAjxPbv38=")),
          QByteArray::fromBase64(QByteArrayLiteral("/1eK3R2LtjPBT3el8f0q4DvzqUJSfFy5fkKkKPNFNYw=")) });

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=")) },
        QXmppTrustStorage::ManuallyDistrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("Ciemp4ZNzRJxnRD+k28vAie0kXJrwl4IrbfDy7n6OxE=")) },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("rvSXBRd+EICMhQvVgcREQJxxP+T4EBmai4mYHBfJQGg=")) },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("tCP1CI3pqSTVGzFYFyPYUMfMZ9Ck/msmfD0wH/VtJBM=")),
          QByteArray::fromBase64(QByteArrayLiteral("2fhJtrgoMJxfLI3084/YkYh9paqiSiLFDVL2m0qAgX4=")) },
        QXmppTrustStorage::ManuallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("YjVI04NcbTPvXLaA95RO84HPcSvyOgEZ2r5cTyUs0C8=")) },
        QXmppTrustStorage::Authenticated);

    m_trustStorage.addKeys(
        ns_ox,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=")),
          QByteArray::fromBase64(QByteArrayLiteral("IhpPjiKLchgrAG5cpSfTvdzPjZ5v6vTOluHEUehkgCA=")) },
        QXmppTrustStorage::Authenticated);

    QMultiHash<QString, QByteArray> automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                      QByteArray::fromBase64(QByteArrayLiteral("WaAnpWyW1hnFooH3oJo9Ba5XYoksnLPeJRTAjxPbv38=")) },
                                                                    { QStringLiteral("alice@example.org"),
                                                                      QByteArray::fromBase64(QByteArrayLiteral("/1eK3R2LtjPBT3el8f0q4DvzqUJSfFy5fkKkKPNFNYw=")) } };
    QMultiHash<QString, QByteArray> manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=")) } };
    QMultiHash<QString, QByteArray> automaticallyTrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                   QByteArray::fromBase64(QByteArrayLiteral("Ciemp4ZNzRJxnRD+k28vAie0kXJrwl4IrbfDy7n6OxE=")) },
                                                                 { QStringLiteral("bob@example.com"),
                                                                   QByteArray::fromBase64(QByteArrayLiteral("rvSXBRd+EICMhQvVgcREQJxxP+T4EBmai4mYHBfJQGg=")) } };
    QMultiHash<QString, QByteArray> manuallyTrustedKeys = { { QStringLiteral("bob@example.com"),
                                                              QByteArray::fromBase64(QByteArrayLiteral("tCP1CI3pqSTVGzFYFyPYUMfMZ9Ck/msmfD0wH/VtJBM=")) },
                                                            { QStringLiteral("bob@example.com"),
                                                              QByteArray::fromBase64(QByteArrayLiteral("2fhJtrgoMJxfLI3084/YkYh9paqiSiLFDVL2m0qAgX4=")) } };
    QMultiHash<QString, QByteArray> authenticatedKeys = { { QStringLiteral("bob@example.com"),
                                                            QByteArray::fromBase64(QByteArrayLiteral("YjVI04NcbTPvXLaA95RO84HPcSvyOgEZ2r5cTyUs0C8=")) } };

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
                              { QByteArray::fromBase64(QByteArrayLiteral("WaAnpWyW1hnFooH3oJo9Ba5XYoksnLPeJRTAjxPbv38=")),
                                QByteArray::fromBase64(QByteArrayLiteral("Ciemp4ZNzRJxnRD+k28vAie0kXJrwl4IrbfDy7n6OxE=")) });

    automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                      QByteArray::fromBase64(QByteArrayLiteral("/1eK3R2LtjPBT3el8f0q4DvzqUJSfFy5fkKkKPNFNYw=")) } };
    automaticallyTrustedKeys = { { QStringLiteral("bob@example.com"),
                                   QByteArray::fromBase64(QByteArrayLiteral("rvSXBRd+EICMhQvVgcREQJxxP+T4EBmai4mYHBfJQGg=")) } };

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
                            QByteArray::fromBase64(QByteArrayLiteral("aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=")) },
                          { QStringLiteral("alice@example.org"),
                            QByteArray::fromBase64(QByteArrayLiteral("IhpPjiKLchgrAG5cpSfTvdzPjZ5v6vTOluHEUehkgCA=")) } };

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
        { QByteArray::fromBase64(QByteArrayLiteral("AZ/cF4OrUOILKO1gQBf62pQevOhBJ2NyHnXLwM4FDZU=")) },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("AZ/cF4OrUOILKO1gQBf62pQevOhBJ2NyHnXLwM4FDZU=")),
          QByteArray::fromBase64(QByteArrayLiteral("JU4pT7Ivpigtl+7QE87Bkq4r/C/mhI1FCjY5Wmjbtwg=")) },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=")) },
        QXmppTrustStorage::ManuallyTrusted);

    m_trustStorage.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("9E51lG3vVmUn8CM7/AIcmIlLP2HPl6Ao0/VSf4VT/oA=")) },
        QXmppTrustStorage::AutomaticallyTrusted);

    auto future = m_trustStorage.trustLevel(
        ns_omemo,
        QByteArray::fromBase64(QByteArrayLiteral("AZ/cF4OrUOILKO1gQBf62pQevOhBJ2NyHnXLwM4FDZU=")));
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(result, QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage.setTrustLevel(
        ns_omemo,
        { { QStringLiteral("alice@example.org"),
            QByteArray::fromBase64(QByteArrayLiteral("AZ/cF4OrUOILKO1gQBf62pQevOhBJ2NyHnXLwM4FDZU=")) },
          { QStringLiteral("bob@example.com"),
            QByteArray::fromBase64(QByteArrayLiteral("9E51lG3vVmUn8CM7/AIcmIlLP2HPl6Ao0/VSf4VT/oA=")) } },
        QXmppTrustStorage::Authenticated);

    future = m_trustStorage.trustLevel(
        ns_omemo,
        QByteArray::fromBase64(QByteArrayLiteral("AZ/cF4OrUOILKO1gQBf62pQevOhBJ2NyHnXLwM4FDZU=")));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::Authenticated);

    future = m_trustStorage.trustLevel(
        ns_omemo,
        QByteArray::fromBase64(QByteArrayLiteral("9E51lG3vVmUn8CM7/AIcmIlLP2HPl6Ao0/VSf4VT/oA=")));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::Authenticated);

    // Set the trust level of a key that is not stored yet.
    // It is added to the storage automatically.
    m_trustStorage.setTrustLevel(
        ns_omemo,
        { { QStringLiteral("alice@example.org"),
            QByteArray::fromBase64(QByteArrayLiteral("9w6oPjKyGSALd9gHq7sNOdOAkD5bHUVOKACNs89FjkA=")) } },
        QXmppTrustStorage::ManuallyTrusted);

    future = m_trustStorage.trustLevel(
        ns_omemo,
        QByteArray::fromBase64(QByteArrayLiteral("9w6oPjKyGSALd9gHq7sNOdOAkD5bHUVOKACNs89FjkA=")));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::ManuallyTrusted);

    // Try to retrieve the trust level of a key that is not stored yet.
    // The default value is returned.
    future = m_trustStorage.trustLevel(
        ns_omemo,
        QByteArray::fromBase64(QByteArrayLiteral("WXL4EDfzUGbVPQWjT9pmBeiCpCBzYZv3lUAaj+UbPyE=")));
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
        QByteArray::fromBase64(QByteArrayLiteral("AZ/cF4OrUOILKO1gQBf62pQevOhBJ2NyHnXLwM4FDZU=")));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::ManuallyDistrusted);

    future = m_trustStorage.trustLevel(
        ns_omemo,
        QByteArray::fromBase64(QByteArrayLiteral("9E51lG3vVmUn8CM7/AIcmIlLP2HPl6Ao0/VSf4VT/oA=")));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::ManuallyDistrusted);

    // Verify that the default trust level is returned for an unknown key.
    future = m_trustStorage.trustLevel(
        ns_omemo,
        QByteArray::fromBase64(QByteArrayLiteral("wE06Gwf8f4DvDLFDoaCsGs8ibcUjf84WIOA2FAjPI3o=")));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::AutomaticallyDistrusted);

    m_trustStorage.removeKeys();
}

void tst_QXmppTrustMemoryStorage::testKeysForPostponedTrustDecisions()
{
    QXmppTrustMessageKeyOwner keyOwnerAlice;
    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    keyOwnerAlice.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("Wl53ZchbtAtCZQCHROiD20W7UnKTQgWQrjTHAVNw1ic=")),
                                   QByteArray::fromBase64(QByteArrayLiteral("QR05jrab7PFkSLhtdzyXrPfCqhkNCYCrlWATaBMTenE=")),
                                   QByteArray::fromBase64(QByteArrayLiteral("7y1t0LnmNBeXJka43XejFPLrKtQlSFATrYmy7xHaKYU=")) });
    keyOwnerAlice.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("mB98hhdVps++skUuy4TGy/Vp6RQXLJO4JGf86FAUjyc=")),
                                      QByteArray::fromBase64(QByteArrayLiteral("7y1t0LnmNBeXJka43XejFPLrKtQlSFATrYmy7xHaKYU=")) });

    QXmppTrustMessageKeyOwner keyOwnerBobTrustedKeys;
    keyOwnerBobTrustedKeys.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBobTrustedKeys.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("GgTqeRLp1M+MEenzFQym2oqer9PfHukS4brJDQl5ARE=")) });

    // The key 7y1t0LnmNBeXJka43XejFPLrKtQlSFATrYmy7xHaKYU=
    // is passed for both postponed authentication and distrusting.
    // Thus, it is only stored for postponed distrusting.
    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("Mp6Y4wOF3aMcl38lb/VNbdPF9ucGFqSx2eyaEsqyHKE=")),
                                                     { keyOwnerAlice, keyOwnerBobTrustedKeys });

    QXmppTrustMessageKeyOwner keyOwnerBobDistrustedKeys;
    keyOwnerBobDistrustedKeys.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBobDistrustedKeys.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("sD6ilugEBeKxPsdDEyX43LSGKHKWd5MFEdhT+4RpsxA=")),
                                                  QByteArray::fromBase64(QByteArrayLiteral("X5tJ1D5rEeaeQE8eqhBKAj4KUZGYe3x+iHifaTBY1kM=")) });

    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("IL5iwDQwquH7yjb5RAiIP+nvYiBUsNCXtKB8IpKc9QU=")),
                                                     { keyOwnerBobDistrustedKeys });

    QXmppTrustMessageKeyOwner keyOwnerCarol;
    keyOwnerCarol.setJid(QStringLiteral("carol@example.net"));
    keyOwnerCarol.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("WcL+cEMpEeK+dpqg3Xd3amctzwP8h2MqwXcEzFf6LpU=")),
                                   QByteArray::fromBase64(QByteArrayLiteral("bH3R31z0N97K1fUwG3+bdBrVPuDfXguQapHudkfa5nE=")) });
    keyOwnerCarol.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("N0B2StHKk1/slwg1rzybTFzjdg7FChc+3cXmTU/rS8g=")),
                                      QByteArray::fromBase64(QByteArrayLiteral("wsEN32UHCiNjYqTG/J63hY4Nu8tZT42Ni1FxrgyRQ5g=")) });

    m_trustStorage.addKeysForPostponedTrustDecisions(ns_ox,
                                                     QByteArray::fromBase64(QByteArrayLiteral("IL5iwDQwquH7yjb5RAiIP+nvYiBUsNCXtKB8IpKc9QU=")),
                                                     { keyOwnerCarol });

    QMultiHash<QString, QByteArray> trustedKeys = { { QStringLiteral("alice@example.org"),
                                                      QByteArray::fromBase64(QByteArrayLiteral("Wl53ZchbtAtCZQCHROiD20W7UnKTQgWQrjTHAVNw1ic=")) },
                                                    { QStringLiteral("alice@example.org"),
                                                      QByteArray::fromBase64(QByteArrayLiteral("QR05jrab7PFkSLhtdzyXrPfCqhkNCYCrlWATaBMTenE=")) },
                                                    { QStringLiteral("bob@example.com"),
                                                      QByteArray::fromBase64(QByteArrayLiteral("GgTqeRLp1M+MEenzFQym2oqer9PfHukS4brJDQl5ARE=")) } };
    QMultiHash<QString, QByteArray> distrustedKeys = { { QStringLiteral("alice@example.org"),
                                                         QByteArray::fromBase64(QByteArrayLiteral("mB98hhdVps++skUuy4TGy/Vp6RQXLJO4JGf86FAUjyc=")) },
                                                       { QStringLiteral("alice@example.org"),
                                                         QByteArray::fromBase64(QByteArrayLiteral("7y1t0LnmNBeXJka43XejFPLrKtQlSFATrYmy7xHaKYU=")) } };

    auto future = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                                { QByteArray::fromBase64(QByteArrayLiteral("Mp6Y4wOF3aMcl38lb/VNbdPF9ucGFqSx2eyaEsqyHKE=")) });
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
                         QByteArray::fromBase64(QByteArrayLiteral("mB98hhdVps++skUuy4TGy/Vp6RQXLJO4JGf86FAUjyc=")) },
                       { QStringLiteral("alice@example.org"),
                         QByteArray::fromBase64(QByteArrayLiteral("7y1t0LnmNBeXJka43XejFPLrKtQlSFATrYmy7xHaKYU=")) },
                       { QStringLiteral("bob@example.com"),
                         QByteArray::fromBase64(QByteArrayLiteral("sD6ilugEBeKxPsdDEyX43LSGKHKWd5MFEdhT+4RpsxA=")) },
                       { QStringLiteral("bob@example.com"),
                         QByteArray::fromBase64(QByteArrayLiteral("X5tJ1D5rEeaeQE8eqhBKAj4KUZGYe3x+iHifaTBY1kM=")) } };

    future = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                           { QByteArray::fromBase64(QByteArrayLiteral("Mp6Y4wOF3aMcl38lb/VNbdPF9ucGFqSx2eyaEsqyHKE=")),
                                                             QByteArray::fromBase64(QByteArrayLiteral("IL5iwDQwquH7yjb5RAiIP+nvYiBUsNCXtKB8IpKc9QU=")) });
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

    keyOwnerBobTrustedKeys.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("sD6ilugEBeKxPsdDEyX43LSGKHKWd5MFEdhT+4RpsxA=")) });

    // Invert the trust in Bob's key
    // sD6ilugEBeKxPsdDEyX43LSGKHKWd5MFEdhT+4RpsxA= for the
    // sending endpoint with the key
    // IL5iwDQwquH7yjb5RAiIP+nvYiBUsNCXtKB8IpKc9QU=.
    m_trustStorage.addKeysForPostponedTrustDecisions(
        ns_omemo,
        QByteArray::fromBase64(QByteArrayLiteral("IL5iwDQwquH7yjb5RAiIP+nvYiBUsNCXtKB8IpKc9QU=")),
        { keyOwnerBobTrustedKeys });

    trustedKeys = { { QStringLiteral("bob@example.com"),
                      QByteArray::fromBase64(QByteArrayLiteral("sD6ilugEBeKxPsdDEyX43LSGKHKWd5MFEdhT+4RpsxA=")) } };
    distrustedKeys = { { QStringLiteral("bob@example.com"),
                         QByteArray::fromBase64(QByteArrayLiteral("X5tJ1D5rEeaeQE8eqhBKAj4KUZGYe3x+iHifaTBY1kM=")) } };

    future = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                           { QByteArray::fromBase64(QByteArrayLiteral("IL5iwDQwquH7yjb5RAiIP+nvYiBUsNCXtKB8IpKc9QU=")) });
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
                                                        { QByteArray::fromBase64(QByteArrayLiteral("Mp6Y4wOF3aMcl38lb/VNbdPF9ucGFqSx2eyaEsqyHKE=")) });

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
    // IL5iwDQwquH7yjb5RAiIP+nvYiBUsNCXtKB8IpKc9QU=.
    m_trustStorage.removeKeysForPostponedTrustDecisions(ns_omemo);

    future = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QVERIFY(result.isEmpty());

    trustedKeys = { { QStringLiteral("carol@example.net"),
                      QByteArray::fromBase64(QByteArrayLiteral("WcL+cEMpEeK+dpqg3Xd3amctzwP8h2MqwXcEzFf6LpU=")) },
                    { QStringLiteral("carol@example.net"),
                      QByteArray::fromBase64(QByteArrayLiteral("bH3R31z0N97K1fUwG3+bdBrVPuDfXguQapHudkfa5nE=")) } };
    distrustedKeys = { { QStringLiteral("carol@example.net"),
                         QByteArray::fromBase64(QByteArrayLiteral("N0B2StHKk1/slwg1rzybTFzjdg7FChc+3cXmTU/rS8g=")) },
                       { QStringLiteral("carol@example.net"),
                         QByteArray::fromBase64(QByteArrayLiteral("wsEN32UHCiNjYqTG/J63hY4Nu8tZT42Ni1FxrgyRQ5g=")) } };

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
