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


#ifndef QXMPPREGISTERIQ_H
#define QXMPPREGISTERIQ_H

#include "QXmppDataForm.h"
#include "QXmppIq.h"

/// \brief The QXmppRegisterIq class represents a registration IQ
/// as defined by XEP-0077: In-Band Registration.
///
/// It is used to create an account on the server.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppRegisterIq : public QXmppIq
{
public:
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

    /// \cond
    static bool isRegisterIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QXmppDataForm m_form;
    QString m_email;
    QString m_instructions;
    QString m_password;
    QString m_username;
};

#endif
