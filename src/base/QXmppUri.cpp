// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2019 Melvin Keskin <melvo@olomono.de>
// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppUri.h"

#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <array>
#include <ranges>

#include <QUrlQuery>

using namespace QXmpp::Private;
using namespace QXmpp::Uri;

using std::ranges::transform;

constexpr QStringView SCHEME = u"xmpp";
constexpr QChar QUERY_ITEM_DELIMITER = u';';
constexpr QChar QUERY_ITEM_KEY_DELIMITER = u'=';

// QXmppMessage types as strings
constexpr std::array<QStringView, 5> MESSAGE_TYPES = {
    u"error",
    u"normal",
    u"chat",
    u"groupchat",
    u"headline"
};

// Adds a key-value pair to a query if the value is not empty.
static void addKeyValuePairToQuery(QUrlQuery &query, const QString &key, QStringView value)
{
    if (!value.isEmpty()) {
        query.addQueryItem(key, value.toString());
    }
}

// Extracts the fully-encoded value of a query's key-value pair.
static QString queryItemValue(const QUrlQuery &query, const QString &key)
{
    return query.queryItemValue(key, QUrl::FullyDecoded);
}

///
/// \namespace QXmpp::Uri
///
/// Contains URI classes that can be serialized to URI queries (see QXmppUri).
///
/// \since QXmpp 1.8
///

namespace QXmpp::Uri {

///
/// \struct Command
///
/// A "command" query from \xep{0050, Ad-Hoc Commands}.
///
/// \since QXmpp 1.8
///

///
/// \struct Invite
///
/// An "invite" query from \xep{0045, Multi-User Chat}.
///
/// \since QXmpp 1.8
///

///
/// \struct Join
///
/// A "join" query from \xep{0045, Multi-User Chat}.
///
/// \since QXmpp 1.8
///

///
/// \struct Login
///
/// A "login" query, not formally specified.
///
/// Used in the wild, e.g. by Kaidan.
///
/// \since QXmpp 1.8
///

///
/// \struct Message
///
/// A "message" query defined in \xep{0147, XMPP URI Scheme Query Components}.
///
/// \since QXmpp 1.8
///

///
/// \struct Unregister
///
/// An "unregister" query defined in \xep{0077, In-Band Registration}.
///
/// \since QXmpp 1.8
///

///
/// \struct Unsubscribe
///
/// An "unsubscribe" query defined in \xep{0147, XMPP URI Scheme Query Components}.
///
/// \since QXmpp 1.8
///

///
/// \struct Register
///
/// A "register" query defined in \xep{0077, In-Band Registration}.
///
/// \since QXmpp 1.8
///

///
/// \struct Remove
///
/// A "remove" query defined in \xep{0147, XMPP URI Scheme Query Components}.
///
/// \since QXmpp 1.8
///

///
/// \struct Roster
///
/// A "roster" query defined in \xep{0147, XMPP URI Scheme Query Components}.
///
/// \since QXmpp 1.8
///

///
/// \struct Subscribe
///
/// A "subscribe" query defined in \xep{0147, XMPP URI Scheme Query Components}.
///
/// \since QXmpp 1.8
///

///
/// \struct TrustMessage
///
/// A "trust-message" query defined in \xep{0434, Trust Messages (TM)}.
///
/// \since QXmpp 1.8
///

///
/// \struct CustomQuery
///
/// A query with a custom name and custom key-value pairs.
///
/// Queries will be parsed into this type if they are unknown.
///
/// \since QXmpp 1.8
///

}  // namespace QXmpp::Uri

static void serializeUrlQuery(const Command &command, QUrlQuery &query)
{
    query.addQueryItem(u"command"_s, {});

    addKeyValuePairToQuery(query, u"node"_s, command.node);
    addKeyValuePairToQuery(query, u"action"_s, command.action);
}

static void serializeUrlQuery(const Invite &invite, QUrlQuery &query)
{
    query.addQueryItem(u"invite"_s, {});

    addKeyValuePairToQuery(query, u"jid"_s, invite.inviteeJid);
    addKeyValuePairToQuery(query, u"password"_s, invite.password);
}

static void serializeUrlQuery(const Join &join, QUrlQuery &query)
{
    query.addQueryItem(u"join"_s, {});

    addKeyValuePairToQuery(query, u"password"_s, join.password);
}

static void serializeUrlQuery(const Login &login, QUrlQuery &query)
{
    query.addQueryItem(u"login"_s, {});

    addKeyValuePairToQuery(query, u"password"_s, login.password);
}

static void serializeUrlQuery(const Message &message, QUrlQuery &query)
{
    query.addQueryItem(u"message"_s, {});

    addKeyValuePairToQuery(query, u"from"_s, message.from);
    addKeyValuePairToQuery(query, u"id"_s, message.id);
    if (message.type) {
        addKeyValuePairToQuery(query, u"type"_s, MESSAGE_TYPES.at(size_t(*message.type)));
    }
    addKeyValuePairToQuery(query, QStringLiteral("subject"), message.subject);
    addKeyValuePairToQuery(query, QStringLiteral("body"), message.body);
    addKeyValuePairToQuery(query, QStringLiteral("thread"), message.thread);
}

static void serializeUrlQuery(const Unregister &, QUrlQuery &query)
{
    query.addQueryItem(u"unregister"_s, {});
}

static void serializeUrlQuery(const Unsubscribe &, QUrlQuery &query)
{
    query.addQueryItem(u"unsubscribe"_s, {});
}

static void serializeUrlQuery(const Register &, QUrlQuery &query)
{
    query.addQueryItem(u"register"_s, {});
}

static void serializeUrlQuery(const Remove &, QUrlQuery &query)
{
    query.addQueryItem(u"remove"_s, {});
}

static void serializeUrlQuery(const Roster &roster, QUrlQuery &query)
{
    query.addQueryItem(u"roster"_s, {});

    addKeyValuePairToQuery(query, u"name"_s, roster.name);
    addKeyValuePairToQuery(query, u"group"_s, roster.group);
}

static void serializeUrlQuery(const Subscribe &, QUrlQuery &query)
{
    query.addQueryItem(u"subscribe"_s, {});
}

static void serializeUrlQuery(const TrustMessage &trustMessage, QUrlQuery &query)
{
    query.addQueryItem(u"trust-message"_s, {});

    addKeyValuePairToQuery(query, QStringLiteral("encryption"), trustMessage.encryption);

    for (auto &identifier : trustMessage.trustKeyIds) {
        addKeyValuePairToQuery(query, QStringLiteral("trust"), identifier);
    }

    for (auto &identifier : trustMessage.distrustKeyIds) {
        addKeyValuePairToQuery(query, QStringLiteral("distrust"), identifier);
    }
}

static void serializeUrlQuery(const CustomQuery &custom, QUrlQuery &query)
{
    query.addQueryItem(custom.query, {});
    for (const auto &[key, value] : custom.parameters) {
        query.addQueryItem(key, value);
    }
}

Command parseCommandQuery(const QUrlQuery &q)
{
    return {
        queryItemValue(q, u"node"_s),
        queryItemValue(q, u"action"_s),
    };
}

Invite parseInviteQuery(const QUrlQuery &q)
{
    return {
        queryItemValue(q, u"jid"_s),
        queryItemValue(q, u"password"_s),
    };
}

Join parseJoinQuery(const QUrlQuery &q)
{
    return {
        queryItemValue(q, u"password"_s),
    };
}

Login parseLoginQuery(const QUrlQuery &q)
{
    return {
        queryItemValue(q, u"login"_s),
    };
}

Message parseMessageQuery(const QUrlQuery &q)
{
    return {
        queryItemValue(q, u"subject"_s),
        queryItemValue(q, u"body"_s),
        queryItemValue(q, u"thread"_s),
        queryItemValue(q, u"id"_s),
        queryItemValue(q, u"from"_s),
        enumFromString<QXmppMessage::Type>(MESSAGE_TYPES, queryItemValue(q, u"type"_s)),
    };
}

Roster parseRosterQuery(const QUrlQuery &q)
{
    return Roster {
        queryItemValue(q, u"name"_s),
        queryItemValue(q, u"group"_s),
    };
}

TrustMessage parseTrustMessageQuery(const QUrlQuery &q)
{
    return TrustMessage {
        queryItemValue(q, QStringLiteral("encryption")),
        q.allQueryItemValues(QStringLiteral("trust"), QUrl::FullyDecoded),
        q.allQueryItemValues(QStringLiteral("distrust"), QUrl::FullyDecoded),
    };
}

CustomQuery parseCustomQuery(const QUrlQuery &q)
{
    auto queryItems = q.queryItems();
    auto queryName = queryItems.first().first;
    queryItems.pop_front();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return CustomQuery { queryName, queryItems };
#else
    QList<std::pair<QString, QString>> queryItemsStdPair;
    queryItemsStdPair.reserve(queryItems.size());
    transform(queryItems, std::back_inserter(queryItemsStdPair), [](const auto &pair) { return std::pair { pair.first, pair.second }; });
    return CustomQuery { queryName, queryItemsStdPair };
#endif
}

struct QXmppUriPrivate : QSharedData {
    QString jid;
    std::any query;
};

///
/// \class QXmppUri
///
/// This class represents an XMPP URI as specified by RFC 5122 -  Internationalized Resource
/// Identifiers (IRIs) and Uniform Resource Identifiers (URIs) for the Extensible Messaging and
/// Presence Protocol (XMPP) and XEP-0147: XMPP URI Scheme Query Components.
///
/// A QUrlQuery is used by this class to represent a query (component) of an XMPP URI. A query
/// conisists of query items which can be the query type or a key-value pair.
///
/// A query type is used to perform an action while the key-value pairs are used to define its
/// behavior.
///
/// Example:
/// xmpp:alice@example.org?message;subject=Hello;body=world
///
/// query (component): message;subject=Hello;body=world
/// query items: message, subject=Hello, body=world
/// query type: message
/// key-value pair 1: subject=Hello
/// key-value pair 2: body=world
///
/// \since QXmpp 1.8
///

///
/// Creates an empty XMPP URI
///
QXmppUri::QXmppUri()
    : d(new QXmppUriPrivate)
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppUri)

///
/// Parses an XMPP URI.
///
/// \return Parsed URI or an error if the string could not be parsed.
///
std::variant<QXmppUri, QXmppError> QXmppUri::fromString(const QString &input)
{
    QUrl url(input);
    if (!url.isValid()) {
        return QXmppError { u"Invalid URI"_s, {} };
    }
    if (url.scheme() != SCHEME) {
        return QXmppError { u"Wrong URI scheme (is '%1', must be xmpp)"_s.arg(url.scheme()), {} };
    }

    QXmppUri uri;
    uri.setJid(url.path());

    if (url.hasQuery()) {
        QUrlQuery urlQuery;
        urlQuery.setQueryDelimiters(QUERY_ITEM_KEY_DELIMITER, QUERY_ITEM_DELIMITER);
        urlQuery.setQuery(url.query(QUrl::FullyEncoded));

        // Check if there is at least one query item.
        if (!urlQuery.isEmpty()) {
            auto queryItems = urlQuery.queryItems();
            Q_ASSERT(!queryItems.isEmpty());
            auto [queryString, queryValue] = queryItems.first();

            if (!queryValue.isEmpty()) {
                // invalid XMPP URI: first query query pair must have only key, no value
                return QXmppError { u"Invalid URI query: got key-value pair (instead of key only) for first query parameter."_s, {} };
            }

            if (queryString == u"command") {
                uri.d->query = parseCommandQuery(urlQuery);
            } else if (queryString == u"invite") {
                uri.d->query = parseInviteQuery(urlQuery);
            } else if (queryString == u"join") {
                uri.d->query = parseJoinQuery(urlQuery);
            } else if (queryString == u"login") {
                uri.d->query = parseLoginQuery(urlQuery);
            } else if (queryString == u"message") {
                uri.d->query = parseMessageQuery(urlQuery);
            } else if (queryString == u"register") {
                uri.d->query = Register();
            } else if (queryString == u"remove") {
                uri.d->query = Remove();
            } else if (queryString == u"roster") {
                uri.d->query = parseRosterQuery(urlQuery);
            } else if (queryString == u"subscribe") {
                uri.d->query = Subscribe();
            } else if (queryString == u"trust-message") {
                uri.d->query = parseTrustMessageQuery(urlQuery);
            } else if (queryString == u"unregister") {
                uri.d->query = Unregister();
            } else if (queryString == u"unsubscribe") {
                uri.d->query = Unsubscribe();
            } else {
                uri.d->query = parseCustomQuery(urlQuery);
            }
        }
    }

    return uri;
}

template<typename T>
bool serialize(const std::any &query, QUrlQuery &urlQuery)
{
    if (query.type() == typeid(T)) {
        serializeUrlQuery(std::any_cast<T>(query), urlQuery);
        return true;
    }
    return false;
}

///
/// Serializes the URI to a string.
///
QString QXmppUri::toString()
{
    QUrl url;
    url.setScheme(SCHEME.toString());
    url.setPath(d->jid);

    // add URI query
    QUrlQuery urlQuery;
    urlQuery.setQueryDelimiters(QUERY_ITEM_KEY_DELIMITER, QUERY_ITEM_DELIMITER);

    if (d->query.has_value()) {
        serialize<Command>(d->query, urlQuery) ||
            serialize<Invite>(d->query, urlQuery) ||
            serialize<Join>(d->query, urlQuery) ||
            serialize<Login>(d->query, urlQuery) ||
            serialize<Message>(d->query, urlQuery) ||
            serialize<Unregister>(d->query, urlQuery) ||
            serialize<Unsubscribe>(d->query, urlQuery) ||
            serialize<Register>(d->query, urlQuery) ||
            serialize<Remove>(d->query, urlQuery) ||
            serialize<Roster>(d->query, urlQuery) ||
            serialize<Subscribe>(d->query, urlQuery) ||
            serialize<TrustMessage>(d->query, urlQuery) ||
            serialize<CustomQuery>(d->query, urlQuery);
    }

    url.setQuery(urlQuery);

    return QString::fromUtf8(url.toEncoded(QUrl::FullyEncoded));
}

///
/// Returns the JID this URI is about.
///
/// This can also be e.g. a MUC room in case of a Join action.
///
QString QXmppUri::jid() const
{
    return d->jid;
}

///
/// Sets the JID this URI links to.
///
void QXmppUri::setJid(const QString &jid)
{
    d->jid = jid;
}

///
/// Returns the query of the URI.
///
/// It may be empty (has_value() returns false). Possible URI types are available in the namespace
/// QXmpp::Uri.
///
std::any QXmppUri::query() const
{
    return d->query;
}

void QXmppUri::setQuery(std::any &&query)
{
    d->query = std::move(query);
}
