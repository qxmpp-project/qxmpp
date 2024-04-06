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
#include <QUuid>

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

namespace QXmpp::Private::Sasl {

enum class ErrorCondition {
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

QString errorConditionToString(ErrorCondition);
std::optional<ErrorCondition> errorConditionFromString(QStringView);

struct Auth {
    static std::optional<Auth> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *writer) const;

    QString mechanism;
    QByteArray value;
};

struct Challenge {
    static std::optional<Challenge> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *writer) const;

    QByteArray value;
};

struct Failure {
    static std::optional<Failure> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *writer) const;

    std::optional<ErrorCondition> condition;
    QString text;
};

struct Response {
    static std::optional<Response> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *writer) const;

    QByteArray value;
};

struct Success {
    static std::optional<Success> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *writer) const;
};

}  // namespace QXmpp::Private::Sasl

namespace QXmpp::Private {

struct Bind2Feature {
    static std::optional<Bind2Feature> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    std::vector<QString> features;
};

struct Bind2Request {
    static std::optional<Bind2Request> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    QString tag;
    // bind2 extensions may be added here (like carbons, sm, csi)
};

struct Bind2Bound {
    static std::optional<Bind2Bound> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    // extensions may be added here (mam, sm, etc.)
};

}  // namespace QXmpp::Private

namespace QXmpp::Private::Sasl2 {

struct StreamFeature {
    static std::optional<StreamFeature> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    QList<QString> mechanisms;
    std::optional<Bind2Feature> bind2Feature;
    bool streamResumptionAvailable = false;
};

struct UserAgent {
    static std::optional<UserAgent> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    QUuid id;
    QString software;
    QString device;
};

struct Authenticate {
    static std::optional<Authenticate> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    QString mechanism;
    QByteArray initialResponse;
    std::optional<UserAgent> userAgent;
    std::optional<Bind2Request> bindRequest;
};

struct Challenge {
    static std::optional<Challenge> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    QByteArray data;
};

struct Response {
    static std::optional<Response> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    QByteArray data;
};

struct Success {
    static std::optional<Success> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    std::optional<QByteArray> additionalData;
    QString authorizationIdentifier;
    // extensions
    std::optional<Bind2Bound> bound;
};

struct Failure {
    static std::optional<Failure> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    Sasl::ErrorCondition condition;
    QString text;
    // extensions
};

struct Continue {
    static std::optional<Continue> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    QByteArray additionalData;
    std::vector<QString> tasks;
    QString text;
};

struct Abort {
    static std::optional<Abort> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    QString text;
};

}  // namespace QXmpp::Private::Sasl2

class QXMPP_AUTOTEST_EXPORT QXmppSaslClient : public QXmppLoggable
{
    Q_OBJECT
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
    virtual std::optional<QByteArray> respond(const QByteArray &challenge) = 0;

    static QStringList availableMechanisms();
    static std::unique_ptr<QXmppSaslClient> create(const QString &mechanism, QObject *parent = nullptr);

private:
    friend class QXmpp::Private::SaslManager;

    const std::unique_ptr<QXmppSaslClientPrivate> d;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslServer : public QXmppLoggable
{
    Q_OBJECT
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

    static std::unique_ptr<QXmppSaslServer> create(const QString &mechanism, QObject *parent = nullptr);

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

class QXmppSaslClientAnonymous : public QXmppSaslClient
{
    Q_OBJECT
public:
    QXmppSaslClientAnonymous(QObject *parent = nullptr);
    QString mechanism() const override;
    std::optional<QByteArray> respond(const QByteArray &challenge) override;

private:
    int m_step;
};

class QXmppSaslClientDigestMd5 : public QXmppSaslClient
{
    Q_OBJECT
public:
    QXmppSaslClientDigestMd5(QObject *parent = nullptr);
    QString mechanism() const override;
    std::optional<QByteArray> respond(const QByteArray &challenge) override;

private:
    QByteArray m_cnonce;
    QByteArray m_nc;
    QByteArray m_nonce;
    QByteArray m_secret;
    int m_step;
};

class QXmppSaslClientFacebook : public QXmppSaslClient
{
    Q_OBJECT
public:
    QXmppSaslClientFacebook(QObject *parent = nullptr);
    QString mechanism() const override;
    std::optional<QByteArray> respond(const QByteArray &challenge) override;

private:
    int m_step;
};

class QXmppSaslClientGoogle : public QXmppSaslClient
{
    Q_OBJECT
public:
    QXmppSaslClientGoogle(QObject *parent = nullptr);
    QString mechanism() const override;
    std::optional<QByteArray> respond(const QByteArray &challenge) override;

private:
    int m_step;
};

class QXmppSaslClientPlain : public QXmppSaslClient
{
    Q_OBJECT
public:
    QXmppSaslClientPlain(QObject *parent = nullptr);
    QString mechanism() const override;
    std::optional<QByteArray> respond(const QByteArray &challenge) override;

private:
    int m_step;
};

class QXmppSaslClientScram : public QXmppSaslClient
{
    Q_OBJECT
public:
    QXmppSaslClientScram(QCryptographicHash::Algorithm algorithm, QObject *parent = nullptr);
    QString mechanism() const override;
    std::optional<QByteArray> respond(const QByteArray &challenge) override;

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
    Q_OBJECT
public:
    QXmppSaslClientWindowsLive(QObject *parent = nullptr);
    QString mechanism() const override;
    std::optional<QByteArray> respond(const QByteArray &challenge) override;

private:
    int m_step;
};

class QXmppSaslServerAnonymous : public QXmppSaslServer
{
    Q_OBJECT
public:
    QXmppSaslServerAnonymous(QObject *parent = nullptr);
    QString mechanism() const override;

    Response respond(const QByteArray &challenge, QByteArray &response) override;

private:
    int m_step;
};

class QXmppSaslServerDigestMd5 : public QXmppSaslServer
{
    Q_OBJECT
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

class QXmppSaslServerPlain : public QXmppSaslServer
{
    Q_OBJECT
public:
    QXmppSaslServerPlain(QObject *parent = nullptr);
    QString mechanism() const override;

    Response respond(const QByteArray &challenge, QByteArray &response) override;

private:
    int m_step;
};

#endif
