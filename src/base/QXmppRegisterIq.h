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

#ifndef QXMPPREGISTERIQ_H
#define QXMPPREGISTERIQ_H

#include "QXmppDataForm.h"
#include "QXmppIq.h"

class QXmppBitsOfBinaryDataList;
class QXmppRegisterIqPrivate;

/// \brief The QXmppRegisterIq class represents a registration IQ
/// as defined by \xep{0077}: In-Band Registration.
///
/// It is used to create an account on the server.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppRegisterIq : public QXmppIq
{
public:
    QXmppRegisterIq();
    QXmppRegisterIq(const QXmppRegisterIq &other);
    ~QXmppRegisterIq();

    QXmppRegisterIq &operator=(const QXmppRegisterIq &other);

    static QXmppRegisterIq createChangePasswordRequest(const QString &username, const QString &newPassword, const QString &to = {});
    static QXmppRegisterIq createUnregistrationRequest(const QString &to = {});

    QString email() const;
    void setEmail(const QString &email);

    QXmppDataForm form() const;
    void setForm(const QXmppDataForm &form);

    QString instructions() const;
    void setInstructions(const QString &instructions);

    QString password() const;
    void setPassword(const QString &username);

    QString username() const;
    void setUsername(const QString &username);

    bool isRegistered() const;
    void setIsRegistered(bool isRegistered);

    bool isRemove() const;
    void setIsRemove(bool isRemove);

    QXmppBitsOfBinaryDataList bitsOfBinaryData() const;
    QXmppBitsOfBinaryDataList &bitsOfBinaryData();
    void setBitsOfBinaryData(const QXmppBitsOfBinaryDataList &bitsOfBinaryData);

    /// \cond
    static bool isRegisterIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppRegisterIqPrivate> d;
};

#endif
