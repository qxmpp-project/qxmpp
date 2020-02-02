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

static const QStringList REGISTER_TYPES = {
    QString(),
    QStringLiteral("registered"),
    QStringLiteral("remove")
};

class QXmppRegisterIqPrivate : public QSharedData
{
public:
    QXmppRegisterIqPrivate();

    QXmppDataForm form;
    QString email;
    QString instructions;
    QString password;
    QString username;
    QXmppRegisterIq::RegisterType registerType;
    QXmppBitsOfBinaryDataList bitsOfBinaryData;
};

QXmppRegisterIqPrivate::QXmppRegisterIqPrivate()
    : registerType(QXmppRegisterIq::None)
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
    iq.setRegisterType(QXmppRegisterIq::Remove);
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

/// Returns a list of data packages attached using XEP-0231: Bits of Binary.
///
/// This could be used to resolve a \c cid: URL of an CAPTCHA field of the
/// form.
///
/// \since QXmpp 1.2

QXmppBitsOfBinaryDataList QXmppRegisterIq::bitsOfBinaryData() const
{
    return d->bitsOfBinaryData;
}

/// Returns a list of data attached using XEP-0231: Bits of Binary.
///
/// This could be used to resolve a \c cid: URL of an CAPTCHA field of the
/// form.
///
/// \since QXmpp 1.2

QXmppBitsOfBinaryDataList &QXmppRegisterIq::bitsOfBinaryData()
{
    return d->bitsOfBinaryData;
}

/// Sets a list of XEP-0231: Bits of Binary attachments to be included.
///
/// \since QXmpp 1.2

void QXmppRegisterIq::setBitsOfBinaryData(const QXmppBitsOfBinaryDataList &bitsOfBinaryData)
{
    d->bitsOfBinaryData = bitsOfBinaryData;
}

/// Returns the type of the action or registration state.
///
/// \since QXmpp 1.2

QXmppRegisterIq::RegisterType QXmppRegisterIq::registerType() const
{
    return d->registerType;
}

/// Sets the type of the action or registration state.
///
/// \since QXmpp 1.2

void QXmppRegisterIq::setRegisterType(QXmppRegisterIq::RegisterType type)
{
    d->registerType = type;
}

/// \cond
bool QXmppRegisterIq::isRegisterIq(const QDomElement &element)
{
    return (element.firstChildElement("query").namespaceURI() == ns_register);
}

void QXmppRegisterIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    d->instructions = queryElement.firstChildElement("instructions").text();
    d->username = queryElement.firstChildElement("username").text();
    d->password = queryElement.firstChildElement("password").text();
    d->email = queryElement.firstChildElement("email").text();
    d->form.parse(queryElement.firstChildElement("x"));
    d->bitsOfBinaryData.parse(queryElement);

    d->registerType = QXmppRegisterIq::None;

    // The loop starts with the second element, because there cannot be an
    // empty element for QXmppRegisterIq::None
    for (int i = 1; i < REGISTER_TYPES.size(); i++) {
        if (!queryElement.firstChildElement(REGISTER_TYPES.at(i)).isNull()) {
            d->registerType = RegisterType(i);
            break;
        }
    }
}

void QXmppRegisterIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeDefaultNamespace(ns_register);

    if (!d->instructions.isEmpty())
        writer->writeTextElement("instructions", d->instructions);

    if (d->registerType != QXmppRegisterIq::None)
        writer->writeEmptyElement(REGISTER_TYPES.at(int(d->registerType)));

    if (!d->username.isEmpty())
        writer->writeTextElement("username", d->username);
    else if (!d->username.isNull())
        writer->writeEmptyElement("username");

    if (!d->password.isEmpty())
        writer->writeTextElement("password", d->password);
    else if (!d->password.isNull())
        writer->writeEmptyElement("password");

    if (!d->email.isEmpty())
        writer->writeTextElement("email", d->email);
    else if (!d->email.isNull())
        writer->writeEmptyElement("email");

    d->form.toXml(writer);
    d->bitsOfBinaryData.toXml(writer);

    writer->writeEndElement();
}

/// \endcond
