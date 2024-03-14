// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSASL_P_H
#define QXMPPSASL_P_H

#include "QXmppGlobal.h"
#include "QXmppLogger.h"
#include "QXmppNonza.h"

#include <optional>

#include <QCryptographicHash>
#include <QMap>

class QDomElement;
class QXmlStreamWriter;
class QXmppSaslClientPrivate;
class QXmppSaslServerPrivate;

namespace QXmpp::Private {
class SaslManager;
}

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.  It exists for the convenience
// of the QXmppIncomingClient and QXmppOutgoingClient classes.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

namespace QXmpp::Private {

enum class SaslErrorCondition {
    Aborted,
    AccountDisabled,
    CredentialsExpired,
    EncryptionRequired,
    IncorrectEncoding,
    InvalidAuthzid,
    InvalidMechanism,
    MalformedRequest,
    MechanismTooWeak,
    NotAuthorized,
    TemporaryAuthFailure,
};

}

class QXMPP_AUTOTEST_EXPORT QXmppSaslClient : public QXmppLoggable
{
public:
    QXmppSaslClient(QObject *parent = nullptr);
    ~QXmppSaslClient() override;

    QString host() const;
    void setHost(const QString &host);

    QString serviceType() const;
    void setServiceType(const QString &serviceType);

    QString username() const;
    void setUsername(const QString &username);

    QString password() const;
    void setPassword(const QString &password);

    virtual QString mechanism() const = 0;
    virtual bool respond(const QByteArray &challenge, QByteArray &response) = 0;

    static QStringList availableMechanisms();
    static QXmppSaslClient *create(const QString &mechanism, QObject *parent = nullptr);

private:
    friend class QXmpp::Private::SaslManager;

    const std::unique_ptr<QXmppSaslClientPrivate> d;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslServer : public QXmppLoggable
{
public:
    enum Response {
        Challenge = 0,
        Succeeded = 1,
        Failed = 2,
        InputNeeded = 3
    };

    QXmppSaslServer(QObject *parent = nullptr);
    ~QXmppSaslServer() override;

    QString username() const;
    void setUsername(const QString &username);

    QString password() const;
    void setPassword(const QString &password);

    QByteArray passwordDigest() const;
    void setPasswordDigest(const QByteArray &digest);

    QString realm() const;
    void setRealm(const QString &realm);

    virtual QString mechanism() const = 0;
    virtual Response respond(const QByteArray &challenge, QByteArray &response) = 0;

    static QXmppSaslServer *create(const QString &mechanism, QObject *parent = nullptr);

private:
    const std::unique_ptr<QXmppSaslServerPrivate> d;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslDigestMd5
{
public:
    static void setNonce(const QByteArray &nonce);

    // message parsing and serialization
    static QMap<QByteArray, QByteArray> parseMessage(const QByteArray &ba);
    static QByteArray serializeMessage(const QMap<QByteArray, QByteArray> &map);
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslAuth : public QXmppNonza
{
public:
    QXmppSaslAuth(QString mechanism = {}, QByteArray value = {}) : mechanism(mechanism), value(value) { }

    QString mechanism;
    QByteArray value;

    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslChallenge : public QXmppNonza
{
public:
    QXmppSaslChallenge(QByteArray value = {}) : value(value) { }

    QByteArray value;

    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslFailure : public QXmppNonza
{
public:
    static QString conditionToString(QXmpp::Private::SaslErrorCondition condition);

    QXmppSaslFailure(std::optional<QXmpp::Private::SaslErrorCondition> condition = {}, QString text = {}) : condition(condition), text(text) { }

    std::optional<QXmpp::Private::SaslErrorCondition> condition;
    QString text;

    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslResponse : public QXmppNonza
{
public:
    QXmppSaslResponse(QByteArray value = {}) : value(value) { }

    QByteArray value;

    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslSuccess : public QXmppNonza
{
public:
    void parse(const QDomElement &) override { }
    void toXml(QXmlStreamWriter *writer) const override;
};

class QXmppSaslClientAnonymous : public QXmppSaslClient
{
public:
    QXmppSaslClientAnonymous(QObject *parent = nullptr);
    QString mechanism() const override;
    bool respond(const QByteArray &challenge, QByteArray &response) override;

private:
    int m_step;
};

class QXmppSaslClientDigestMd5 : public QXmppSaslClient
{
public:
    QXmppSaslClientDigestMd5(QObject *parent = nullptr);
    QString mechanism() const override;
    bool respond(const QByteArray &challenge, QByteArray &response) override;

private:
    QByteArray m_cnonce;
    QByteArray m_nc;
    QByteArray m_nonce;
    QByteArray m_secret;
    int m_step;
};

class QXmppSaslClientFacebook : public QXmppSaslClient
{
public:
    QXmppSaslClientFacebook(QObject *parent = nullptr);
    QString mechanism() const override;
    bool respond(const QByteArray &challenge, QByteArray &response) override;

private:
    int m_step;
};

class QXmppSaslClientGoogle : public QXmppSaslClient
{
public:
    QXmppSaslClientGoogle(QObject *parent = nullptr);
    QString mechanism() const override;
    bool respond(const QByteArray &challenge, QByteArray &response) override;

private:
    int m_step;
};

class QXmppSaslClientPlain : public QXmppSaslClient
{
public:
    QXmppSaslClientPlain(QObject *parent = nullptr);
    QString mechanism() const override;
    bool respond(const QByteArray &challenge, QByteArray &response) override;

private:
    int m_step;
};

class QXmppSaslClientScram : public QXmppSaslClient
{
public:
    QXmppSaslClientScram(QCryptographicHash::Algorithm algorithm, QObject *parent = nullptr);
    QString mechanism() const override;
    bool respond(const QByteArray &challenge, QByteArray &response) override;

private:
    QCryptographicHash::Algorithm m_algorithm;
    int m_step;
    int m_dklen;
    QByteArray m_gs2Header;
    QByteArray m_clientFirstMessageBare;
    QByteArray m_serverSignature;
    QByteArray m_nonce;
};

class QXmppSaslClientWindowsLive : public QXmppSaslClient
{
public:
    QXmppSaslClientWindowsLive(QObject *parent = nullptr);
    QString mechanism() const override;
    bool respond(const QByteArray &challenge, QByteArray &response) override;

private:
    int m_step;
};

class QXmppSaslServerAnonymous : public QXmppSaslServer
{
public:
    QXmppSaslServerAnonymous(QObject *parent = nullptr);
    QString mechanism() const override;

    Response respond(const QByteArray &challenge, QByteArray &response) override;

private:
    int m_step;
};

class QXmppSaslServerDigestMd5 : public QXmppSaslServer
{
public:
    QXmppSaslServerDigestMd5(QObject *parent = nullptr);
    QString mechanism() const override;

    Response respond(const QByteArray &challenge, QByteArray &response) override;

private:
    QByteArray m_cnonce;
    QByteArray m_nc;
    QByteArray m_nonce;
    QByteArray m_secret;
    int m_step;
};

class QXmppSaslServerFacebook : public QXmppSaslServer
{
public:
    QXmppSaslServerFacebook(QObject *parent = nullptr);
    QString mechanism() const override;

    Response respond(const QByteArray &challenge, QByteArray &response) override;

private:
    int m_step;
};

class QXmppSaslServerPlain : public QXmppSaslServer
{
public:
    QXmppSaslServerPlain(QObject *parent = nullptr);
    QString mechanism() const override;

    Response respond(const QByteArray &challenge, QByteArray &response) override;

private:
    int m_step;
};

#endif
