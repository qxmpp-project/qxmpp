// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPDIALBACK_H
#define QXMPPDIALBACK_H

#include "QXmppStanza.h"

/// \brief The QXmppDialback class represents a stanza used for the Server
/// Dialback protocol as specified by \xep{0220}: Server Dialback.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppDialback : public QXmppStanza
{
public:
    /// This enum is used to describe a dialback command.
    enum Command {
        Result,  ///< A dialback command between the originating server
                 ///< and the receiving server.
        Verify   ///< A dialback command between the receiving server
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
    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;

    static bool isDialback(const QDomElement &element);
    /// \endcond

private:
    Command m_command;
    QString m_key;
    QString m_type;
};

#endif
