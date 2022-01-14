// SPDX-FileCopyrightText: 2011 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPASSWORDCHECKER_H
#define QXMPPPASSWORDCHECKER_H

#include "QXmppGlobal.h"

#include <QObject>

/// \brief The QXmppPasswordRequest class represents a password request.
///
class QXMPP_EXPORT QXmppPasswordRequest
{
public:
    /// This enum is used to describe request types.
    enum Type {
        CheckPassword = 0
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
        TemporaryError
    };

    QXmppPasswordReply(QObject *parent = nullptr);

    QByteArray digest() const;
    void setDigest(const QByteArray &digest);

    QString password() const;
    void setPassword(const QString &password);

    QXmppPasswordReply::Error error() const;
    void setError(QXmppPasswordReply::Error error);

    bool isFinished() const;

public Q_SLOTS:
    void finish();
    void finishLater();

Q_SIGNALS:
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
