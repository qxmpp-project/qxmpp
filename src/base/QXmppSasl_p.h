/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
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

#ifndef QXMPPSASL_P_H
#define QXMPPSASL_P_H

#include <QByteArray>
#include <QMap>

#include "QXmppGlobal.h"
#include "QXmppLogger.h"
#include "QXmppStanza.h"

class QXmppSaslClientPrivate;
class QXmppSaslServerPrivate;

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

class QXMPP_AUTOTEST_EXPORT QXmppSaslClient : public QXmppLoggable
{
public:
    QXmppSaslClient(QObject *parent = 0);
    virtual ~QXmppSaslClient();

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
    static QXmppSaslClient* create(const QString &mechanism, QObject *parent = 0);

private:
    QXmppSaslClientPrivate *d;
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

    QXmppSaslServer(QObject *parent = 0);
    virtual ~QXmppSaslServer();

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

    static QXmppSaslServer* create(const QString &mechanism, QObject *parent = 0);

private:
    QXmppSaslServerPrivate *d;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslDigestMd5
{
public:
    static void setNonce(const QByteArray &nonce);

    // message parsing and serialization
    static QMap<QByteArray, QByteArray> parseMessage(const QByteArray &ba);
    static QByteArray serializeMessage(const QMap<QByteArray, QByteArray> &map);
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslAuth : public QXmppStanza
{
public:
    QXmppSaslAuth(const QString &mechanism = QString(), const QByteArray &value = QByteArray());

    QString mechanism() const;
    void setMechanism(const QString &mechanism);

    QByteArray value() const;
    void setValue(const QByteArray &value);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_mechanism;
    QByteArray m_value;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslChallenge : public QXmppStanza
{
public:
    QXmppSaslChallenge(const QByteArray &value = QByteArray());

    QByteArray value() const;
    void setValue(const QByteArray &value);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QByteArray m_value;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslFailure : public QXmppStanza
{
public:
    QXmppSaslFailure(const QString &condition = QString());

    QString condition() const;
    void setCondition(const QString &condition);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_condition;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslResponse : public QXmppStanza
{
public:
    QXmppSaslResponse(const QByteArray &value = QByteArray());

    QByteArray value() const;
    void setValue(const QByteArray &value);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QByteArray m_value;
};

class QXMPP_AUTOTEST_EXPORT QXmppSaslSuccess : public QXmppStanza
{
public:
    QXmppSaslSuccess();

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond
};

class QXmppSaslClientAnonymous : public QXmppSaslClient
{
public:
    QXmppSaslClientAnonymous(QObject *parent = 0);
    QString mechanism() const;
    bool respond(const QByteArray &challenge, QByteArray &response);

private:
    int m_step;
};

class QXmppSaslClientDigestMd5 : public QXmppSaslClient
{
public:
    QXmppSaslClientDigestMd5(QObject *parent = 0);
    QString mechanism() const;
    bool respond(const QByteArray &challenge, QByteArray &response);

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
    QXmppSaslClientFacebook(QObject *parent = 0);
    QString mechanism() const;
    bool respond(const QByteArray &challenge, QByteArray &response);

private:
    int m_step;
};

class QXmppSaslClientGoogle : public QXmppSaslClient
{
public:
    QXmppSaslClientGoogle(QObject *parent = 0);
    QString mechanism() const;
    bool respond(const QByteArray &challenge, QByteArray &response);

private:
    int m_step;
};

class QXmppSaslClientPlain : public QXmppSaslClient
{
public:
    QXmppSaslClientPlain(QObject *parent = 0);
    QString mechanism() const;
    bool respond(const QByteArray &challenge, QByteArray &response);

private:
    int m_step;
};

class QXmppSaslClientWindowsLive : public QXmppSaslClient
{
public:
    QXmppSaslClientWindowsLive(QObject *parent = 0);
    QString mechanism() const;
    bool respond(const QByteArray &challenge, QByteArray &response);

private:
    int m_step;
};

class QXmppSaslServerAnonymous : public QXmppSaslServer
{
public:
    QXmppSaslServerAnonymous(QObject *parent = 0);
    QString mechanism() const;

    Response respond(const QByteArray &challenge, QByteArray &response);

private:
    int m_step;
};

class QXmppSaslServerDigestMd5 : public QXmppSaslServer
{
public:
    QXmppSaslServerDigestMd5(QObject *parent = 0);
    QString mechanism() const;

    Response respond(const QByteArray &challenge, QByteArray &response);

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
    QXmppSaslServerFacebook(QObject *parent = 0);
    QString mechanism() const;

    Response respond(const QByteArray &challenge, QByteArray &response);

private:
    int m_step;
};

class QXmppSaslServerPlain : public QXmppSaslServer
{
public:
    QXmppSaslServerPlain(QObject *parent = 0);
    QString mechanism() const;

    Response respond(const QByteArray &challenge, QByteArray &response);

private:
    int m_step;
};

#endif
