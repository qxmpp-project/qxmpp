// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppSasl_p.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include <QDomElement>
#include <QMessageAuthenticationCode>
#include <QStringBuilder>
#include <QUrlQuery>
#include <QXmlStreamWriter>
#include <QtEndian>

using namespace QXmpp::Private;

static QByteArray forcedNonce;

constexpr auto SASL_ERROR_CONDITIONS = to_array<QStringView>({
    u"aborted",
    u"account-disabled",
    u"credentials-expired",
    u"encryption-required",
    u"incorrect-encoding",
    u"invalid-authzid",
    u"invalid-mechanism",
    u"malformed-request",
    u"mechanism-too-weak",
    u"not-authorized",
    u"temporary-auth-failure",
});

// When adding new algorithms, also add them to QXmppSaslClient::availableMechanisms().
static const QMap<QString, QCryptographicHash::Algorithm> SCRAM_ALGORITHMS = {
    { QStringLiteral("SCRAM-SHA-1"), QCryptographicHash::Sha1 },
    { QStringLiteral("SCRAM-SHA-256"), QCryptographicHash::Sha256 },
    { QStringLiteral("SCRAM-SHA-512"), QCryptographicHash::Sha512 },
    { QStringLiteral("SCRAM-SHA3-512"), QCryptographicHash::RealSha3_512 },
};

// Calculate digest response for use with XMPP/SASL.

static QByteArray calculateDigest(const QByteArray &method, const QByteArray &digestUri, const QByteArray &secret, const QByteArray &nonce, const QByteArray &cnonce, const QByteArray &nc)
{
    const QByteArray A1 = secret + ':' + nonce + ':' + cnonce;
    const QByteArray A2 = method + ':' + digestUri;

    QByteArray HA1 = QCryptographicHash::hash(A1, QCryptographicHash::Md5).toHex();
    QByteArray HA2 = QCryptographicHash::hash(A2, QCryptographicHash::Md5).toHex();
    const QByteArray KD = HA1 + ':' + nonce + ':' + nc + ':' + cnonce + ":auth:" + HA2;
    return QCryptographicHash::hash(KD, QCryptographicHash::Md5).toHex();
}

// Perform PBKFD2 key derivation, code taken from Qt 5.12

static QByteArray deriveKeyPbkdf2(QCryptographicHash::Algorithm algorithm,
                                  const QByteArray &data, const QByteArray &salt,
                                  int iterations, quint64 dkLen)
{
    QByteArray key;
    quint32 currentIteration = 1;
    QMessageAuthenticationCode hmac(algorithm, data);
    QByteArray index(4, Qt::Uninitialized);
    while (quint64(key.length()) < dkLen) {
        hmac.addData(salt);
        qToBigEndian(currentIteration, reinterpret_cast<uchar *>(index.data()));
        hmac.addData(index);
        QByteArray u = hmac.result();
        hmac.reset();
        QByteArray tkey = u;
        for (int iter = 1; iter < iterations; iter++) {
            hmac.addData(u);
            u = hmac.result();
            hmac.reset();
            std::transform(tkey.cbegin(), tkey.cend(), u.cbegin(), tkey.begin(),
                           std::bit_xor<char>());
        }
        key += tkey;
        currentIteration++;
    }
    return key.left(dkLen);
}

static QByteArray generateNonce()
{
    if (!forcedNonce.isEmpty()) {
        return forcedNonce;
    }

    QByteArray nonce = QXmppUtils::generateRandomBytes(32);

    // The random data can the '=' char is not valid as it is a delimiter,
    // so to be safe, base64 the nonce
    return nonce.toBase64();
}

static QMap<char, QByteArray> parseGS2(const QByteArray &ba)
{
    QMap<char, QByteArray> map;
    const auto keyValuePairs = ba.split(u',');
    for (const auto &keyValue : keyValuePairs) {
        if (keyValue.size() >= 2 && keyValue[1] == '=') {
            map[keyValue[0]] = keyValue.mid(2);
        }
    }
    return map;
}

void QXmppSaslAuth::parse(const QDomElement &element)
{
    mechanism = element.attribute(QStringLiteral("mechanism"));
    value = QByteArray::fromBase64(element.text().toLatin1());
}

void QXmppSaslAuth::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("auth"));
    writer->writeDefaultNamespace(toString65(ns_xmpp_sasl));
    writer->writeAttribute(QSL65("mechanism"), mechanism);
    if (!value.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        writer->writeCharacters(value.toBase64());
#else
        writer->writeCharacters(QString::fromUtf8(value.toBase64()));
#endif
    }
    writer->writeEndElement();
}

void QXmppSaslChallenge::parse(const QDomElement &element)
{
    value = QByteArray::fromBase64(element.text().toLatin1());
}

void QXmppSaslChallenge::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("challenge"));
    writer->writeDefaultNamespace(toString65(ns_xmpp_sasl));
    if (!value.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        writer->writeCharacters(value.toBase64());
#else
        writer->writeCharacters(QString::fromUtf8(value.toBase64()));
#endif
    }
    writer->writeEndElement();
}

void QXmppSaslFailure::parse(const QDomElement &element)
{
    auto errorConditionString = element.firstChildElement().tagName();
    condition = enumFromString<SaslErrorCondition>(SASL_ERROR_CONDITIONS, errorConditionString);

    // RFC3920 defines the error condition as "not-authorized", but
    // some broken servers use "bad-auth" instead. We tolerate this
    // by remapping the error to "not-authorized".
    if (!condition && errorConditionString == u"bad-auth") {
        condition = SaslErrorCondition::NotAuthorized;
    }

    text = element.firstChildElement(QStringLiteral("text")).text();
}

void QXmppSaslFailure::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("failure"));
    writer->writeDefaultNamespace(toString65(ns_xmpp_sasl));
    if (condition) {
        writer->writeEmptyElement(toString65(SASL_ERROR_CONDITIONS.at(size_t(*condition))));
    }

    if (!text.isEmpty()) {
        writer->writeStartElement(QStringLiteral("text"));
        writer->writeAttribute(QStringLiteral("xml:lang"), QStringLiteral("en"));
        writer->writeCharacters(text);
        writer->writeEndElement();
    }

    writer->writeEndElement();
}

void QXmppSaslResponse::parse(const QDomElement &element)
{
    value = QByteArray::fromBase64(element.text().toLatin1());
}

void QXmppSaslResponse::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("response"));
    writer->writeDefaultNamespace(toString65(ns_xmpp_sasl));
    if (!value.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        writer->writeCharacters(value.toBase64());
#else
        writer->writeCharacters(QString::fromUtf8(value.toBase64()));
#endif
    }
    writer->writeEndElement();
}

void QXmppSaslSuccess::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("success"));
    writer->writeDefaultNamespace(toString65(ns_xmpp_sasl));
    writer->writeEndElement();
}

class QXmppSaslClientPrivate
{
public:
    QString host;
    QString serviceType;
    QString username;
    QString password;
};

QXmppSaslClient::QXmppSaslClient(QObject *parent)
    : QXmppLoggable(parent),
      d(std::make_unique<QXmppSaslClientPrivate>())
{
}

QXmppSaslClient::~QXmppSaslClient() = default;

///
/// Returns a list of supported mechanisms.
///
QStringList QXmppSaslClient::availableMechanisms()
{
    return {
        QStringLiteral("SCRAM-SHA3-512"),
        QStringLiteral("SCRAM-SHA-512"),
        QStringLiteral("SCRAM-SHA-256"),
        QStringLiteral("SCRAM-SHA-1"),
        QStringLiteral("DIGEST-MD5"),
        QStringLiteral("PLAIN"),
        QStringLiteral("ANONYMOUS"),
        QStringLiteral("X-FACEBOOK-PLATFORM"),
        QStringLiteral("X-MESSENGER-OAUTH2"),
        QStringLiteral("X-OAUTH2"),
    };
}

///
/// Creates an SASL client for the given mechanism.
///
QXmppSaslClient *QXmppSaslClient::create(const QString &mechanism, QObject *parent)
{
    if (mechanism == u"PLAIN") {
        return new QXmppSaslClientPlain(parent);
    } else if (mechanism == u"DIGEST-MD5") {
        return new QXmppSaslClientDigestMd5(parent);
    } else if (mechanism == u"ANONYMOUS") {
        return new QXmppSaslClientAnonymous(parent);
    } else if (SCRAM_ALGORITHMS.contains(mechanism)) {
        return new QXmppSaslClientScram(SCRAM_ALGORITHMS.value(mechanism), parent);
    } else if (mechanism == u"X-FACEBOOK-PLATFORM") {
        return new QXmppSaslClientFacebook(parent);
    } else if (mechanism == u"X-MESSENGER-OAUTH2") {
        return new QXmppSaslClientWindowsLive(parent);
    } else if (mechanism == u"X-OAUTH2") {
        return new QXmppSaslClientGoogle(parent);
    } else {
        return nullptr;
    }
}

/// Returns the host.
QString QXmppSaslClient::host() const
{
    return d->host;
}

/// Sets the host.
void QXmppSaslClient::setHost(const QString &host)
{
    d->host = host;
}

/// Returns the service type, e.g. "xmpp".
QString QXmppSaslClient::serviceType() const
{
    return d->serviceType;
}

/// Sets the service type, e.g. "xmpp".
void QXmppSaslClient::setServiceType(const QString &serviceType)
{
    d->serviceType = serviceType;
}

/// Returns the username.
QString QXmppSaslClient::username() const
{
    return d->username;
}

/// Sets the username.
void QXmppSaslClient::setUsername(const QString &username)
{
    d->username = username;
}

/// Returns the password.
QString QXmppSaslClient::password() const
{
    return d->password;
}

/// Sets the password.
void QXmppSaslClient::setPassword(const QString &password)
{
    d->password = password;
}

QXmppSaslClientAnonymous::QXmppSaslClientAnonymous(QObject *parent)
    : QXmppSaslClient(parent), m_step(0)
{
}

QString QXmppSaslClientAnonymous::mechanism() const
{
    return QStringLiteral("ANONYMOUS");
}

bool QXmppSaslClientAnonymous::respond(const QByteArray &challenge, QByteArray &response)
{
    Q_UNUSED(challenge);
    if (m_step == 0) {
        response = QByteArray();
        m_step++;
        return true;
    } else {
        warning(QStringLiteral("QXmppSaslClientAnonymous : Invalid step"));
        return false;
    }
}

QXmppSaslClientDigestMd5::QXmppSaslClientDigestMd5(QObject *parent)
    : QXmppSaslClient(parent), m_nc(QByteArrayLiteral("00000001")), m_step(0)
{
    m_cnonce = generateNonce();
}

QString QXmppSaslClientDigestMd5::mechanism() const
{
    return QStringLiteral("DIGEST-MD5");
}

bool QXmppSaslClientDigestMd5::respond(const QByteArray &challenge, QByteArray &response)
{
    Q_UNUSED(challenge);
    const QByteArray digestUri = QStringLiteral("%1/%2").arg(serviceType(), host()).toUtf8();

    if (m_step == 0) {
        response = QByteArray();
        m_step++;
        return true;
    } else if (m_step == 1) {
        const QMap<QByteArray, QByteArray> input = QXmppSaslDigestMd5::parseMessage(challenge);

        if (!input.contains(QByteArrayLiteral("nonce"))) {
            warning(QStringLiteral("QXmppSaslClientDigestMd5 : Invalid input on step 1"));
            return false;
        }

        // determine realm
        const QByteArray realm = input.value(QByteArrayLiteral("realm"));

        // determine quality of protection
        const QList<QByteArray> qops = input.value(QByteArrayLiteral("qop"), QByteArrayLiteral("auth")).split(',');
        if (!qops.contains(QByteArrayLiteral("auth"))) {
            warning(QStringLiteral("QXmppSaslClientDigestMd5 : Invalid quality of protection"));
            return false;
        }

        m_nonce = input.value(QByteArrayLiteral("nonce"));
        m_secret = QCryptographicHash::hash(
            username().toUtf8() + QByteArrayLiteral(":") + realm + QByteArrayLiteral(":") + password().toUtf8(),
            QCryptographicHash::Md5);

        // Build response
        QMap<QByteArray, QByteArray> output;
        output[QByteArrayLiteral("username")] = username().toUtf8();
        if (!realm.isEmpty()) {
            output[QByteArrayLiteral("realm")] = realm;
        }
        output[QByteArrayLiteral("nonce")] = m_nonce;
        output[QByteArrayLiteral("qop")] = QByteArrayLiteral("auth");
        output[QByteArrayLiteral("cnonce")] = m_cnonce;
        output[QByteArrayLiteral("nc")] = m_nc;
        output[QByteArrayLiteral("digest-uri")] = digestUri;
        output[QByteArrayLiteral("response")] = calculateDigest(QByteArrayLiteral("AUTHENTICATE"), digestUri, m_secret, m_nonce, m_cnonce, m_nc);
        output[QByteArrayLiteral("charset")] = QByteArrayLiteral("utf-8");

        response = QXmppSaslDigestMd5::serializeMessage(output);
        m_step++;
        return true;
    } else if (m_step == 2) {
        const QMap<QByteArray, QByteArray> input = QXmppSaslDigestMd5::parseMessage(challenge);

        // check new challenge
        if (input.value(QByteArrayLiteral("rspauth")) != calculateDigest(QByteArray(), digestUri, m_secret, m_nonce, m_cnonce, m_nc)) {
            warning(QStringLiteral("QXmppSaslClientDigestMd5 : Invalid challenge on step 2"));
            return false;
        }

        response = QByteArray();
        m_step++;
        return true;
    } else {
        warning(QStringLiteral("QXmppSaslClientDigestMd5 : Invalid step"));
        return false;
    }
}

QXmppSaslClientFacebook::QXmppSaslClientFacebook(QObject *parent)
    : QXmppSaslClient(parent), m_step(0)
{
}

QString QXmppSaslClientFacebook::mechanism() const
{
    return QStringLiteral("X-FACEBOOK-PLATFORM");
}

bool QXmppSaslClientFacebook::respond(const QByteArray &challenge, QByteArray &response)
{
    if (m_step == 0) {
        // no initial response
        response = QByteArray();
        m_step++;
        return true;
    } else if (m_step == 1) {
        // parse request
        QUrlQuery requestUrl(QString::fromUtf8(challenge));
        if (!requestUrl.hasQueryItem(QStringLiteral("method")) || !requestUrl.hasQueryItem(QStringLiteral("nonce"))) {
            warning(QStringLiteral("QXmppSaslClientFacebook : Invalid challenge, nonce or method missing"));
            return false;
        }

        // build response
        QUrlQuery responseUrl;
        responseUrl.addQueryItem(QStringLiteral("access_token"), password());
        responseUrl.addQueryItem(QStringLiteral("api_key"), username());
        responseUrl.addQueryItem(QStringLiteral("call_id"), QString());
        responseUrl.addQueryItem(QStringLiteral("method"), requestUrl.queryItemValue(QStringLiteral("method")));
        responseUrl.addQueryItem(QStringLiteral("nonce"), requestUrl.queryItemValue(QStringLiteral("nonce")));
        responseUrl.addQueryItem(QStringLiteral("v"), QStringLiteral("1.0"));

        response = responseUrl.query().toUtf8();

        m_step++;
        return true;
    } else {
        warning(QStringLiteral("QXmppSaslClientFacebook : Invalid step"));
        return false;
    }
}

QXmppSaslClientGoogle::QXmppSaslClientGoogle(QObject *parent)
    : QXmppSaslClient(parent), m_step(0)
{
}

QString QXmppSaslClientGoogle::mechanism() const
{
    return QStringLiteral("X-OAUTH2");
}

bool QXmppSaslClientGoogle::respond(const QByteArray &challenge, QByteArray &response)
{
    Q_UNUSED(challenge);
    if (m_step == 0) {
        // send initial response
        response = QString(u'\0' % username() % u'\0' % password()).toUtf8();
        m_step++;
        return true;
    } else {
        warning(QStringLiteral("QXmppSaslClientGoogle : Invalid step"));
        return false;
    }
}

QXmppSaslClientPlain::QXmppSaslClientPlain(QObject *parent)
    : QXmppSaslClient(parent), m_step(0)
{
}

QString QXmppSaslClientPlain::mechanism() const
{
    return QStringLiteral("PLAIN");
}

bool QXmppSaslClientPlain::respond(const QByteArray &challenge, QByteArray &response)
{
    Q_UNUSED(challenge);
    if (m_step == 0) {
        response = QString(u'\0' % username() % u'\0' % password()).toUtf8();
        m_step++;
        return true;
    } else {
        warning(QStringLiteral("QXmppSaslClientPlain : Invalid step"));
        return false;
    }
}

QXmppSaslClientScram::QXmppSaslClientScram(QCryptographicHash::Algorithm algorithm, QObject *parent)
    : QXmppSaslClient(parent),
      m_algorithm(algorithm),
      m_step(0),
      m_dklen(QCryptographicHash::hashLength(algorithm))
{
    const auto itr = std::find(SCRAM_ALGORITHMS.cbegin(), SCRAM_ALGORITHMS.cend(), algorithm);
    Q_ASSERT(itr != SCRAM_ALGORITHMS.cend());

    m_nonce = generateNonce();
}

QString QXmppSaslClientScram::mechanism() const
{
    return SCRAM_ALGORITHMS.key(m_algorithm);
}

bool QXmppSaslClientScram::respond(const QByteArray &challenge, QByteArray &response)
{
    Q_UNUSED(challenge);
    if (m_step == 0) {
        m_gs2Header = QByteArrayLiteral("n,,");
        m_clientFirstMessageBare = QByteArrayLiteral("n=") + username().toUtf8() + QByteArrayLiteral(",r=") + m_nonce;

        response = m_gs2Header + m_clientFirstMessageBare;
        m_step++;
        return true;
    } else if (m_step == 1) {
        // validate input
        const QMap<char, QByteArray> input = parseGS2(challenge);
        const QByteArray nonce = input.value('r');
        const QByteArray salt = QByteArray::fromBase64(input.value('s'));
        const int iterations = input.value('i').toInt();
        if (!nonce.startsWith(m_nonce) || salt.isEmpty() || iterations < 1) {
            return false;
        }

        // calculate proofs
        const QByteArray clientFinalMessageBare = QByteArrayLiteral("c=") + m_gs2Header.toBase64() + QByteArrayLiteral(",r=") + nonce;
        const QByteArray saltedPassword = deriveKeyPbkdf2(m_algorithm, password().toUtf8(), salt,
                                                          iterations, m_dklen);
        const QByteArray clientKey = QMessageAuthenticationCode::hash(QByteArrayLiteral("Client Key"), saltedPassword, m_algorithm);
        const QByteArray storedKey = QCryptographicHash::hash(clientKey, m_algorithm);
        const QByteArray authMessage = m_clientFirstMessageBare + QByteArrayLiteral(",") + challenge + QByteArrayLiteral(",") + clientFinalMessageBare;
        QByteArray clientProof = QMessageAuthenticationCode::hash(authMessage, storedKey, m_algorithm);
        std::transform(clientProof.cbegin(), clientProof.cend(), clientKey.cbegin(),
                       clientProof.begin(), std::bit_xor<char>());

        const QByteArray serverKey = QMessageAuthenticationCode::hash(QByteArrayLiteral("Server Key"), saltedPassword, m_algorithm);
        m_serverSignature = QMessageAuthenticationCode::hash(authMessage, serverKey, m_algorithm);

        response = clientFinalMessageBare + QByteArrayLiteral(",p=") + clientProof.toBase64();
        m_step++;
        return true;
    } else if (m_step == 2) {
        const QMap<char, QByteArray> input = parseGS2(challenge);
        response = QByteArray();
        m_step++;
        return QByteArray::fromBase64(input.value('v')) == m_serverSignature;
    } else {
        warning(QStringLiteral("QXmppSaslClientPlain : Invalid step"));
        return false;
    }
}

QXmppSaslClientWindowsLive::QXmppSaslClientWindowsLive(QObject *parent)
    : QXmppSaslClient(parent), m_step(0)
{
}

QString QXmppSaslClientWindowsLive::mechanism() const
{
    return QStringLiteral("X-MESSENGER-OAUTH2");
}

bool QXmppSaslClientWindowsLive::respond(const QByteArray &challenge, QByteArray &response)
{
    Q_UNUSED(challenge);
    if (m_step == 0) {
        // send initial response
        response = QByteArray::fromBase64(password().toLatin1());
        m_step++;
        return true;
    } else {
        warning(QStringLiteral("QXmppSaslClientWindowsLive : Invalid step"));
        return false;
    }
}

class QXmppSaslServerPrivate
{
public:
    QString username;
    QString password;
    QByteArray passwordDigest;
    QString realm;
};

QXmppSaslServer::QXmppSaslServer(QObject *parent)
    : QXmppLoggable(parent),
      d(std::make_unique<QXmppSaslServerPrivate>())
{
}

QXmppSaslServer::~QXmppSaslServer() = default;

/// Creates an SASL server for the given mechanism.
QXmppSaslServer *QXmppSaslServer::create(const QString &mechanism, QObject *parent)
{
    if (mechanism == QStringLiteral("PLAIN")) {
        return new QXmppSaslServerPlain(parent);
    } else if (mechanism == QStringLiteral("DIGEST-MD5")) {
        return new QXmppSaslServerDigestMd5(parent);
    } else if (mechanism == QStringLiteral("ANONYMOUS")) {
        return new QXmppSaslServerAnonymous(parent);
    } else {
        return nullptr;
    }
}

/// Returns the username.
QString QXmppSaslServer::username() const
{
    return d->username;
}

/// Sets the username.
void QXmppSaslServer::setUsername(const QString &username)
{
    d->username = username;
}

/// Returns the password.
QString QXmppSaslServer::password() const
{
    return d->password;
}

/// Sets the password.
void QXmppSaslServer::setPassword(const QString &password)
{
    d->password = password;
}

/// Returns the password digest.
QByteArray QXmppSaslServer::passwordDigest() const
{
    return d->passwordDigest;
}

/// Sets the password digest.
void QXmppSaslServer::setPasswordDigest(const QByteArray &digest)
{
    d->passwordDigest = digest;
}

/// Returns the realm.
QString QXmppSaslServer::realm() const
{
    return d->realm;
}

/// Sets the realm.
void QXmppSaslServer::setRealm(const QString &realm)
{
    d->realm = realm;
}

QXmppSaslServerAnonymous::QXmppSaslServerAnonymous(QObject *parent)
    : QXmppSaslServer(parent), m_step(0)
{
}

QString QXmppSaslServerAnonymous::mechanism() const
{
    return QStringLiteral("ANONYMOUS");
}

QXmppSaslServer::Response QXmppSaslServerAnonymous::respond(const QByteArray &request, QByteArray &response)
{
    Q_UNUSED(request);
    if (m_step == 0) {
        m_step++;
        response = QByteArray();
        return Succeeded;
    } else {
        warning(QStringLiteral("QXmppSaslServerAnonymous : Invalid step"));
        return Failed;
    }
}

QXmppSaslServerDigestMd5::QXmppSaslServerDigestMd5(QObject *parent)
    : QXmppSaslServer(parent), m_step(0)
{
    m_nonce = generateNonce();
}

QString QXmppSaslServerDigestMd5::mechanism() const
{
    return QStringLiteral("DIGEST-MD5");
}

QXmppSaslServer::Response QXmppSaslServerDigestMd5::respond(const QByteArray &request, QByteArray &response)
{
    if (m_step == 0) {
        QMap<QByteArray, QByteArray> output;
        output[QByteArrayLiteral("nonce")] = m_nonce;
        if (!realm().isEmpty()) {
            output[QByteArrayLiteral("realm")] = realm().toUtf8();
        }
        output[QByteArrayLiteral("qop")] = QByteArrayLiteral("auth");
        output[QByteArrayLiteral("charset")] = QByteArrayLiteral("utf-8");
        output[QByteArrayLiteral("algorithm")] = QByteArrayLiteral("md5-sess");

        m_step++;
        response = QXmppSaslDigestMd5::serializeMessage(output);
        return Challenge;
    } else if (m_step == 1) {
        const QMap<QByteArray, QByteArray> input = QXmppSaslDigestMd5::parseMessage(request);
        const QByteArray realm = input.value(QByteArrayLiteral("realm"));
        const QByteArray digestUri = input.value(QByteArrayLiteral("digest-uri"));

        if (input.value(QByteArrayLiteral("qop")) != QByteArrayLiteral("auth")) {
            warning(QStringLiteral("QXmppSaslServerDigestMd5 : Invalid quality of protection"));
            return Failed;
        }

        setUsername(QString::fromUtf8(input.value(QByteArrayLiteral("username"))));
        if (password().isEmpty() && passwordDigest().isEmpty()) {
            return InputNeeded;
        }

        m_nc = input.value(QByteArrayLiteral("nc"));
        m_cnonce = input.value(QByteArrayLiteral("cnonce"));
        if (!password().isEmpty()) {
            m_secret = QCryptographicHash::hash(
                username().toUtf8() + QByteArrayLiteral(":") + realm + QByteArrayLiteral(":") + password().toUtf8(),
                QCryptographicHash::Md5);
        } else {
            m_secret = passwordDigest();
        }

        if (input.value(QByteArrayLiteral("response")) != calculateDigest(QByteArrayLiteral("AUTHENTICATE"), digestUri, m_secret, m_nonce, m_cnonce, m_nc)) {
            return Failed;
        }

        QMap<QByteArray, QByteArray> output;
        output[QByteArrayLiteral("rspauth")] = calculateDigest(QByteArray(), digestUri, m_secret, m_nonce, m_cnonce, m_nc);

        m_step++;
        response = QXmppSaslDigestMd5::serializeMessage(output);
        return Challenge;
    } else if (m_step == 2) {
        m_step++;
        response = QByteArray();
        return Succeeded;
    } else {
        warning(QStringLiteral("QXmppSaslServerDigestMd5 : Invalid step"));
        return Failed;
    }
}

QXmppSaslServerPlain::QXmppSaslServerPlain(QObject *parent)
    : QXmppSaslServer(parent), m_step(0)
{
}

QString QXmppSaslServerPlain::mechanism() const
{
    return QStringLiteral("PLAIN");
}

QXmppSaslServer::Response QXmppSaslServerPlain::respond(const QByteArray &request, QByteArray &response)
{
    if (m_step == 0) {
        if (request.isEmpty()) {
            response = QByteArray();
            return Challenge;
        }

        QList<QByteArray> auth = request.split('\0');
        if (auth.size() != 3) {
            warning(QStringLiteral("QXmppSaslServerPlain : Invalid input"));
            return Failed;
        }
        setUsername(QString::fromUtf8(auth[1]));
        setPassword(QString::fromUtf8(auth[2]));

        m_step++;
        response = QByteArray();
        return InputNeeded;
    } else {
        warning(QStringLiteral("QXmppSaslServerPlain : Invalid step"));
        return Failed;
    }
}

void QXmppSaslDigestMd5::setNonce(const QByteArray &nonce)
{
    forcedNonce = nonce;
}

QMap<QByteArray, QByteArray> QXmppSaslDigestMd5::parseMessage(const QByteArray &ba)
{
    QMap<QByteArray, QByteArray> map;
    int startIndex = 0;
    int pos = 0;
    while ((pos = ba.indexOf('=', startIndex)) >= 0) {
        // key get name and skip equals
        const QByteArray key = ba.mid(startIndex, pos - startIndex).trimmed();
        pos++;

        if (pos == ba.size()) {
            // end of the input
            map.insert(key, QByteArray());
            startIndex = pos;
        } else if (ba.at(pos) == '"') {
            // check whether string is quoted
            // skip opening quote
            pos++;
            int endPos = ba.indexOf('"', pos);
            // skip quoted quotes
            while (endPos >= 0 && ba.at(endPos - 1) == '\\') {
                endPos = ba.indexOf('"', endPos + 1);
            }
            if (endPos < 0) {
                qWarning("Unfinished quoted string");
                return map;
            }
            // unquote
            QByteArray value = ba.mid(pos, endPos - pos);
            value.replace("\\\"", "\"");
            value.replace("\\\\", "\\");
            map[key] = value;
            // skip closing quote and comma
            startIndex = endPos + 2;
        } else {
            // non-quoted string
            int endPos = ba.indexOf(',', pos);
            if (endPos < 0) {
                endPos = ba.size();
            }
            map[key] = ba.mid(pos, endPos - pos);
            // skip comma
            startIndex = endPos + 1;
        }
    }
    return map;
}

QByteArray QXmppSaslDigestMd5::serializeMessage(const QMap<QByteArray, QByteArray> &map)
{
    QByteArray ba;
    for (auto itr = map.begin(); itr != map.end(); itr++) {
        if (!ba.isEmpty()) {
            ba.append(',');
        }
        ba.append(itr.key() + QByteArrayLiteral("="));
        auto value = itr.value();
        const char *separators = "()<>@,;:\\\"/[]?={} \t";
        bool quote = false;
        for (const char *c = separators; *c; c++) {
            if (value.contains(*c)) {
                quote = true;
                break;
            }
        }
        if (quote) {
            value.replace('\\', QByteArrayLiteral("\\\\"));
            value.replace('\"', QByteArrayLiteral("\\\""));
            ba.append('"' + value + '"');
        } else {
            ba.append(value);
        }
    }
    return ba;
}
