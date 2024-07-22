// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2019 Melvin Keskin <melvo@olomono.de>
// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPURI_H
#define QXMPPURI_H

#include <QXmppError.h>
#include <QXmppMessage.h>

#include <variant>

class QUrlQuery;

struct QXmppUriPrivate;

namespace QXmpp::Uri {

struct Command {
    /// the command node
    QString node;
    /// the ad-hoc commands action type
    QString action;

    /// Default comparison operator.
    bool operator==(const Command &) const = default;
};

struct Invite {
    /// the JID of the invitee
    QString inviteeJid;
    /// the password required to enter a multi-user chat room
    QString password;

    /// Default comparison operator.
    bool operator==(const Invite &) const = default;
};

struct Join {
    /// the password required to enter a multi-user chat room
    QString password;

    /// Default comparison operator.
    bool operator==(const Join &) const = default;
};

struct Login {
    /// the password required to connect to the account
    QString password;

    /// Default comparison operator.
    bool operator==(const Login &) const = default;
};

struct Message {
    /// a subject for the message per the "jabber:client" schema
    QString subject;
    /// a body for the message per the "jabber:client" schema
    QString body;
    /// a Thread ID for the message per the "jabber:client" schema
    QString thread;
    /// a from address for the message per the "jabber:client" schema
    QString id;
    /// an ID for the message per the "jabber:client" schema
    QString from;
    /// the message type per the "jabber:client" schema
    std::optional<QXmppMessage::Type> type;

    /// Default comparison operator.
    bool operator==(const Message &) const = default;
};

struct Unregister {
    /// Default comparison operator.
    bool operator==(const Unregister &) const = default;
};

struct Unsubscribe {
    /// Default comparison operator.
    bool operator==(const Unsubscribe &) const = default;
};

struct Register {
    /// Default comparison operator.
    bool operator==(const Register &) const = default;
};

struct Remove {
    /// Default comparison operator.
    bool operator==(const Remove &) const = default;
};

struct Roster {
    /// the user-assigned group for the roster item
    QString name;
    /// the user-assigned name for the roster item
    QString group;

    /// Default comparison operator.
    bool operator==(const Roster &) const = default;
};

struct Subscribe {
    /// Default comparison operator.
    bool operator==(const Subscribe &) const = default;
};

struct TrustMessage {
    /// encryption of the keys to trust or distrust
    QString encryption;
    /// list of Base16 encoded key identifiers to be trusted
    QList<QString> trustKeyIds;
    /// list of Base16 encoded key identifiers to be distrusted
    QList<QString> distrustKeyIds;

    /// Default comparison operator.
    bool operator==(const TrustMessage &) const = default;
};

struct CustomQuery {
    /// query name as string
    QString query;
    /// list of parameters as key-value pairs
    QList<std::pair<QString, QString>> parameters;

    /// Default comparison operator.
    bool operator==(const CustomQuery &) const = default;
};

}  // namespace QXmpp::Uri

class QXMPP_EXPORT QXmppUri
{
public:
    QXmppUri();
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppUri)

    static std::variant<QXmppUri, QXmppError> fromString(const QString &);

    QString toString();

    QString jid() const;
    void setJid(const QString &jid);

    std::any query() const;
    /// Sets a "command" query.
    void setQuery(QXmpp::Uri::Command &&q) { setQuery(std::any(std::move(q))); }
    /// Sets a MUC invite query.
    void setQuery(QXmpp::Uri::Invite &&q) { setQuery(std::any(std::move(q))); }
    /// Sets a MUC join query.
    void setQuery(QXmpp::Uri::Join &&q) { setQuery(std::any(std::move(q))); }
    /// Sets a login query.
    void setQuery(QXmpp::Uri::Login &&q) { setQuery(std::any(std::move(q))); }
    /// Sets a message query.
    void setQuery(QXmpp::Uri::Message &&q) { setQuery(std::any(std::move(q))); }
    /// Sets a unregister query.
    void setQuery(QXmpp::Uri::Unregister &&q) { setQuery(std::any(std::move(q))); }
    /// Sets a register query.
    void setQuery(QXmpp::Uri::Register &&q) { setQuery(std::any(std::move(q))); }
    /// Sets a remove query.
    void setQuery(QXmpp::Uri::Remove &&q) { setQuery(std::any(std::move(q))); }
    /// Sets a roster query.
    void setQuery(QXmpp::Uri::Roster &&q) { setQuery(std::any(std::move(q))); }
    /// Sets a subscribe query.
    void setQuery(QXmpp::Uri::Subscribe &&q) { setQuery(std::any(std::move(q))); }
    /// Sets a trust message query.
    void setQuery(QXmpp::Uri::TrustMessage &&q) { setQuery(std::any(std::move(q))); }
    /// Sets a query with custom name and key-value pairs.
    void setQuery(QXmpp::Uri::CustomQuery &&q) { setQuery(std::any(std::move(q))); }
    /// Removes any query from the URI.
    void resetQuery() { setQuery(std::any()); }

private:
    void setQuery(std::any &&);

    QSharedDataPointer<QXmppUriPrivate> d;
};

#endif  // QXMPPURI_H
