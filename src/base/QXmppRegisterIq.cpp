/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *  Linus Jahn
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

#include "QXmppRegisterIq.h"

#include "QXmppBitsOfBinaryDataList.h"
#include "QXmppConstants_p.h"

#include <QDomElement>
#include <QSharedData>

#define ELEMENT_REGISTERED QStringLiteral("registered")
#define ELEMENT_REMOVE QStringLiteral("remove")

class QXmppRegisterIqPrivate : public QSharedData
{
public:
    QXmppRegisterIqPrivate();

    QXmppDataForm form;
    QString email;
    QString instructions;
    QString password;
    QString username;
    bool isRegistered;
    bool isRemove;
    QXmppBitsOfBinaryDataList bitsOfBinaryData;
};

QXmppRegisterIqPrivate::QXmppRegisterIqPrivate()
    : isRegistered(false),
      isRemove(false)
{
}

QXmppRegisterIq::QXmppRegisterIq()
    : d(new QXmppRegisterIqPrivate)
{
}

QXmppRegisterIq::QXmppRegisterIq(const QXmppRegisterIq &other) = default;

QXmppRegisterIq::~QXmppRegisterIq() = default;

QXmppRegisterIq &QXmppRegisterIq::operator=(const QXmppRegisterIq &other) = default;

/// Constructs a regular change password request.
///
/// \param username The username of the account of which the password should be
/// changed.
/// \param newPassword The new password that should be set.
/// \param to Optional JID of the registration service. If this is omitted, the
/// IQ is automatically addressed to the local server.
///
/// \since QXmpp 1.2

QXmppRegisterIq QXmppRegisterIq::createChangePasswordRequest(const QString &username, const QString &newPassword, const QString &to)
{
    QXmppRegisterIq iq;
    iq.setType(QXmppIq::Set);
    iq.setTo(to);
    iq.setUsername(username);
    iq.setPassword(newPassword);
    return iq;
}

/// Constructs a regular unregistration request.
///
/// \param to Optional JID of the registration service. If this is omitted, the
/// IQ is automatically addressed to the local server.
///
/// \since QXmpp 1.2

QXmppRegisterIq QXmppRegisterIq::createUnregistrationRequest(const QString &to)
{
    QXmppRegisterIq iq;
    iq.setType(QXmppIq::Set);
    iq.setTo(to);
    iq.setIsRemove(true);
    return iq;
}

/// Returns the email for this registration IQ.

QString QXmppRegisterIq::email() const
{
    return d->email;
}

/// Sets the \a email for this registration IQ.

void QXmppRegisterIq::setEmail(const QString &email)
{
    d->email = email;
}

/// Returns the QXmppDataForm for this registration IQ.

QXmppDataForm QXmppRegisterIq::form() const
{
    return d->form;
}

/// Sets the QXmppDataForm for this registration IQ.
///
/// \param form

void QXmppRegisterIq::setForm(const QXmppDataForm &form)
{
    d->form = form;
}

/// Returns the instructions for this registration IQ.

QString QXmppRegisterIq::instructions() const
{
    return d->instructions;
}

/// Sets the \a instructions for this registration IQ.

void QXmppRegisterIq::setInstructions(const QString &instructions)
{
    d->instructions = instructions;
}

/// Returns the password for this registration IQ.

QString QXmppRegisterIq::password() const
{
    return d->password;
}

/// Sets the \a password for this registration IQ.

void QXmppRegisterIq::setPassword(const QString &password)
{
    d->password = password;
}

/// Returns the username for this registration IQ.

QString QXmppRegisterIq::username() const
{
    return d->username;
}

/// Sets the \a username for this registration IQ.

void QXmppRegisterIq::setUsername(const QString &username)
{
    d->username = username;
}

///
/// Returns whether the account is registered.
///
/// By default this is set to false.
///
/// \since QXmpp 1.2
///
bool QXmppRegisterIq::isRegistered() const
{
    return d->isRegistered;
}

///
/// Sets whether the account is registered.
///
/// By default this is set to false.
///
/// \since QXmpp 1.2
///
void QXmppRegisterIq::setIsRegistered(bool isRegistered)
{
    d->isRegistered = isRegistered;
}

///
/// Returns whether to remove (unregister) the account.
///
/// By default this is set to false.
///
/// \since QXmpp 1.2
///
bool QXmppRegisterIq::isRemove() const
{
    return d->isRemove;
}

///
/// Sets whether to remove (unregister) the account.
///
/// By default this is set to false.
///
/// \since QXmpp 1.2
///
void QXmppRegisterIq::setIsRemove(bool isRemove)
{
    d->isRemove = isRemove;
}

///
/// Returns a list of data packages attached using \xep{0231}: Bits of Binary.
///
/// This could be used to resolve a \c cid: URL of an CAPTCHA field of the
/// form.
///
/// \since QXmpp 1.2
///
QXmppBitsOfBinaryDataList QXmppRegisterIq::bitsOfBinaryData() const
{
    return d->bitsOfBinaryData;
}

///
/// Returns a list of data attached using \xep{0231}: Bits of Binary.
///
/// This could be used to resolve a \c cid: URL of an CAPTCHA field of the
/// form.
///
/// \since QXmpp 1.2
///
QXmppBitsOfBinaryDataList &QXmppRegisterIq::bitsOfBinaryData()
{
    return d->bitsOfBinaryData;
}

///
/// Sets a list of \xep{0231}: Bits of Binary attachments to be included.
///
/// \since QXmpp 1.2
///
void QXmppRegisterIq::setBitsOfBinaryData(const QXmppBitsOfBinaryDataList &bitsOfBinaryData)
{
    d->bitsOfBinaryData = bitsOfBinaryData;
}

/// \cond
bool QXmppRegisterIq::isRegisterIq(const QDomElement &element)
{
    return (element.firstChildElement(QStringLiteral("query")).namespaceURI() == ns_register);
}

void QXmppRegisterIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(QStringLiteral("query"));
    d->instructions = queryElement.firstChildElement(QStringLiteral("instructions")).text();
    d->username = queryElement.firstChildElement(QStringLiteral("username")).text();
    d->password = queryElement.firstChildElement(QStringLiteral("password")).text();
    d->email = queryElement.firstChildElement(QStringLiteral("email")).text();
    d->form.parse(queryElement.firstChildElement(QStringLiteral("x")));
    d->isRegistered = !queryElement.firstChildElement(ELEMENT_REGISTERED).isNull();
    d->isRemove = !queryElement.firstChildElement(ELEMENT_REMOVE).isNull();
    d->bitsOfBinaryData.parse(queryElement);
}

void QXmppRegisterIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("query"));
    writer->writeDefaultNamespace(ns_register);

    if (!d->instructions.isEmpty())
        writer->writeTextElement(QStringLiteral("instructions"), d->instructions);

    if (d->isRegistered)
        writer->writeEmptyElement(ELEMENT_REGISTERED);
    if (d->isRemove)
        writer->writeEmptyElement(ELEMENT_REMOVE);

    if (!d->username.isEmpty())
        writer->writeTextElement(QStringLiteral("username"), d->username);
    else if (!d->username.isNull())
        writer->writeEmptyElement(QStringLiteral("username"));

    if (!d->password.isEmpty())
        writer->writeTextElement(QStringLiteral("password"), d->password);
    else if (!d->password.isNull())
        writer->writeEmptyElement(QStringLiteral("password"));

    if (!d->email.isEmpty())
        writer->writeTextElement(QStringLiteral("email"), d->email);
    else if (!d->email.isNull())
        writer->writeEmptyElement(QStringLiteral("email"));

    d->form.toXml(writer);
    d->bitsOfBinaryData.toXml(writer);

    writer->writeEndElement();
}

/// \endcond
