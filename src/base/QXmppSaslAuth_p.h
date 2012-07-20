/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#ifndef QXMPPSASLAUTH_P_H
#define QXMPPSASLAUTH_P_H

#include "QXmppStanza.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.  It exists for the convenience
// of the QXmppSaslClient class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

class QXmppSaslStanza : public QXmppStanza
{
public:
    QXmppSaslStanza(const QString &type, const QByteArray &value = QByteArray());

    QString type() const;
    void setType(const QString &type);

    QByteArray value() const;
    void setValue(const QByteArray &value);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_type;
    QByteArray m_value;
};

class QXmppSaslAuth : public QXmppSaslStanza
{
public:
    QXmppSaslAuth(const QString &mechanism = QString(), const QByteArray &value = QByteArray());

    QString mechanism() const;
    void setMechanism(const QString &mechanism);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_mechanism;
};

class QXmppSaslChallenge : public QXmppSaslStanza
{
public:
    QXmppSaslChallenge(const QByteArray &value = QByteArray());
};

class QXmppSaslFailure : public QXmppSaslStanza
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

class QXmppSaslResponse : public QXmppSaslStanza
{
public:
    QXmppSaslResponse(const QByteArray &value = QByteArray());
};

class QXmppSaslSuccess : public QXmppSaslStanza
{
public:
    QXmppSaslSuccess();
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
    QXmppSaslDigestMd5 m_saslDigest;
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

class QXmppSaslClientPlain : public QXmppSaslClient
{
public:
    QXmppSaslClientPlain(QObject *parent = 0);
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
    QXmppSaslDigestMd5 m_saslDigest;
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
