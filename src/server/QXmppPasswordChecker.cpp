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

#include <QCryptographicHash>
#include <QString>
#include <QTimer>

#include "QXmppPasswordChecker.h"

/// Returns the requested domain.

QString QXmppPasswordRequest::domain() const
{
    return m_domain;
}

/// Sets the requested \a domain.
///
/// \param domain

void QXmppPasswordRequest::setDomain(const QString &domain)
{
    m_domain = domain;
}

/// Returns the given password.

QString QXmppPasswordRequest::password() const
{
    return m_password;
}

/// Sets the given \a password.

void QXmppPasswordRequest::setPassword(const QString &password)
{
    m_password = password;
}

/// Returns the requested username.

QString QXmppPasswordRequest::username() const
{
    return m_username;
}

/// Sets the requested \a username.
///
/// \param username

void QXmppPasswordRequest::setUsername(const QString &username)
{
    m_username = username;
}

/// Constructs a new QXmppPasswordReply.
///
/// \param parent

QXmppPasswordReply::QXmppPasswordReply(QObject *parent)
    : QObject(parent),
    m_error(QXmppPasswordReply::NoError),
    m_isFinished(false)
{
}

/// Returns the received MD5 digest.

QByteArray QXmppPasswordReply::digest() const
{
    return m_digest;
}

/// Sets the received MD5 digest.
///
/// \param digest

void QXmppPasswordReply::setDigest(const QByteArray &digest)
{
    m_digest = digest;
}

/// Returns the error that was found during the processing of this request.
///
/// If no error was found, returns NoError.

QXmppPasswordReply::Error QXmppPasswordReply::error() const
{
    return m_error;
}

/// Returns the error that was found during the processing of this request.
///
void QXmppPasswordReply::setError(QXmppPasswordReply::Error error)
{
    m_error = error;
}

/// Mark reply as finished.

void QXmppPasswordReply::finish()
{
    m_isFinished = true;
    emit finished();
}

/// Delay marking reply as finished.

void QXmppPasswordReply::finishLater()
{
    QTimer::singleShot(0, this, SLOT(finish()));
}

/// Returns true when the reply has finished.

bool QXmppPasswordReply::isFinished() const
{
    return m_isFinished;
}

/// Returns the received password.

QString QXmppPasswordReply::password() const
{
    return m_password;
}

/// Sets the received password.
///
/// \param password

void QXmppPasswordReply::setPassword(const QString &password)
{
    m_password = password;
}

/// Checks that the given credentials are valid.
///
/// The base implementation requires that you reimplement getPassword().
///
/// \param request

QXmppPasswordReply *QXmppPasswordChecker::checkPassword(const QXmppPasswordRequest &request)
{
    QXmppPasswordReply *reply = new QXmppPasswordReply;

    QString secret;
    QXmppPasswordReply::Error error = getPassword(request, secret);
    if (error == QXmppPasswordReply::NoError) {
        if (request.password() != secret)
            reply->setError(QXmppPasswordReply::AuthorizationError);
    } else {
        reply->setError(error);
    }

    // reply is finished
    reply->finishLater();
    return reply;
}

/// Retrieves the MD5 digest for the given username.
///
/// Reimplement this method if your backend natively supports
/// retrieving MD5 digests.
///
/// \param request

QXmppPasswordReply *QXmppPasswordChecker::getDigest(const QXmppPasswordRequest &request)
{
    QXmppPasswordReply *reply = new QXmppPasswordReply;

    QString secret;
    QXmppPasswordReply::Error error = getPassword(request, secret);
    if (error == QXmppPasswordReply::NoError) {
        reply->setDigest(QCryptographicHash::hash(
            (request.username() + ":" + request.domain() + ":" + secret).toUtf8(),
            QCryptographicHash::Md5));
    } else {
        reply->setError(error);
    }

    // reply is finished
    reply->finishLater();
    return reply;
}

/// Retrieves the password for the given username.
///
/// The simplest way to write a password checker is to reimplement this method.
///
/// \param request
/// \param password

QXmppPasswordReply::Error QXmppPasswordChecker::getPassword(const QXmppPasswordRequest &request, QString &password)
{
    Q_UNUSED(request);
    Q_UNUSED(password);
    return QXmppPasswordReply::TemporaryError;
}

/// Returns true if the getPassword() method is implemented.
///

bool QXmppPasswordChecker::hasGetPassword() const
{
    return false;
}

