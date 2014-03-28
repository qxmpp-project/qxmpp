/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
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

#ifndef QXMPPDIALBACK_H
#define QXMPPDIALBACK_H

#include "QXmppStanza.h"

/// \brief The QXmppDialback class represents a stanza used for the Server
/// Dialback protocol as specified by XEP-0220: Server Dialback.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppDialback : public QXmppStanza
{
public:
    /// This enum is used to describe a dialback command.
    enum Command {
        Result, ///< A dialback command between the originating server
                ///< and the receiving server.
        Verify, ///< A dialback command between the receiving server
                ///< and the authoritative server.
    };

    QXmppDialback();

    Command command() const;
    void setCommand(Command command);

    QString key() const;
    void setKey(const QString &key);

    QString type() const;
    void setType(const QString &type);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

    static bool isDialback(const QDomElement &element);
    /// \endcond

private:
    Command m_command;
    QString m_key;
    QString m_type;
};

#endif
