/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
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

#ifndef QXMPPPASSWORDCHECKER_H
#define QXMPPPASSWORDCHECKER_H

#include <QObject>

#include "QXmppGlobal.h"

/// \brief The QXmppPasswordRequest class represents a password request.
///
class QXMPP_EXPORT QXmppPasswordRequest
{
public:
    /// This enum is used to describe request types.
    enum Type {
        CheckPassword = 0,
    };

    QString domain() const;
    void setDomain(const QString &domain);

    QString password() const;
    void setPassword(const QString &password);

    QString username() const;
    void setUsername(const QString &username);

private:
    QString m_domain;
    QString m_password;
    QString m_username;
};

/// \brief The QXmppPasswordReply class represents a password reply.
///
class QXMPP_EXPORT QXmppPasswordReply : public QObject
{
    Q_OBJECT

public:
    /// This enum is used to describe authentication errors.
    enum Error {
        NoError = 0,
        AuthorizationError,
        TemporaryError,
    };

    QXmppPasswordReply(QObject *parent = 0);

    QByteArray digest() const;
    void setDigest(const QByteArray &digest);

    QString password() const;
    void setPassword(const QString &password);

    QXmppPasswordReply::Error error() const;
    void setError(QXmppPasswordReply::Error error);

    bool isFinished() const;

public slots:
    void finish();
    void finishLater();

signals:
    /// This signal is emitted when the reply has finished.
    void finished();

private:
    QByteArray m_digest;
    QString m_password;
    QXmppPasswordReply::Error m_error;
    bool m_isFinished;
};

/// \brief The QXmppPasswordChecker class represents an abstract password checker.
///

class QXMPP_EXPORT QXmppPasswordChecker
{
public:
    virtual QXmppPasswordReply *checkPassword(const QXmppPasswordRequest &request);
    virtual QXmppPasswordReply *getDigest(const QXmppPasswordRequest &request);
    virtual bool hasGetPassword() const;

protected:
    virtual QXmppPasswordReply::Error getPassword(const QXmppPasswordRequest &request, QString &password);
};

#endif
