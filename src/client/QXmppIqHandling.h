// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPIQHANDLING_H
#define QXMPPIQHANDLING_H

#include "QXmppClient.h"
#include "QXmppE2eeMetadata.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppIq.h"

#include <QDomElement>

namespace QXmpp {

namespace Private {

    QXMPP_EXPORT void sendIqReply(QXmppClient *client,
                                  const QString &requestId,
                                  const QString &requestFrom,
                                  const std::optional<QXmppE2eeMetadata> &e2eeMetadata,
                                  QXmppIq &&iq);

    QXMPP_EXPORT std::tuple<bool, QString, QString> checkIsIqRequest(const QDomElement &el);

    template<typename... VariantTypes>
    void processHandleIqResult(QXmppClient *client,
                               const QString &requestId,
                               const QString &requestFrom,
                               const std::optional<QXmppE2eeMetadata> &e2eeMetadata,
                               std::variant<VariantTypes...> &&result)
    {
        std::visit([&](auto &&value) {
            using T = std::decay_t<decltype(value)>;

            static_assert(
                std::is_base_of_v<QXmppIq, T> || std::is_same_v<QXmppStanza::Error, T>,
                "QXmppIq based type or QXmppStanza::Error required");

            if constexpr (std::is_base_of_v<QXmppIq, T>) {
                sendIqReply(client, requestId, requestFrom, e2eeMetadata, std::move(value));
            } else if constexpr (std::is_same_v<QXmppStanza::Error, T>) {
                QXmppIq iq;
                iq.setType(QXmppIq::Error);
                iq.setError(value);
                sendIqReply(client, requestId, requestFrom, e2eeMetadata, std::move(iq));
            }
        },
                   std::move(result));
    }

    template<typename T>
    void processHandleIqResult(QXmppClient *client,
                               const QString &requestId,
                               const QString &requestFrom,
                               const std::optional<QXmppE2eeMetadata> &e2eeMetadata,
                               QXmppTask<T> future)
    {
        future.then(client, [client, requestId, requestFrom, e2eeMetadata](T result) {
            processHandleIqResult(client, requestId, requestFrom, e2eeMetadata, result);
        });
    }

    template<typename T, typename = std::enable_if_t<std::is_base_of_v<QXmppIq, T>, void>>
    void processHandleIqResult(QXmppClient *client,
                               const QString &requestId,
                               const QString &requestFrom,
                               const std::optional<QXmppE2eeMetadata> &e2eeMetadata,
                               T &&result)
    {
        sendIqReply(client, requestId, requestFrom, e2eeMetadata, std::move(result));
    }

    template<typename Handler, typename IqType>
    auto invokeIqHandler(Handler handler, IqType &&iq)
    {
        if constexpr (std::is_invocable<Handler, IqType &&>()) {
            return handler(std::move(iq));
        } else {
            return handler->handleIq(std::move(iq));
        }
    }

    template<typename IqType, typename Handler>
    bool handleIqType(Handler handler,
                      QXmppClient *client,
                      const QDomElement &element,
                      const std::optional<QXmppE2eeMetadata> &e2eeMetadata,
                      const QString &tagName,
                      const QString &xmlNamespace)
    {
        if (IqType::checkIqType(tagName, xmlNamespace)) {
            IqType iq;
            iq.parse(element);
            iq.setE2eeMetadata(e2eeMetadata);

            auto id = iq.id(), from = iq.from();

            processHandleIqResult(client, id, from, e2eeMetadata,
                                  invokeIqHandler(std::forward<Handler>(handler), std::move(iq)));
            return true;
        }
        return false;
    }

}  // namespace Private

///
/// Parses IQ requests, calls a handler and sends an IQ result or error.
///
/// It is the easiest to explain this function with a few examples.
///
/// ```
/// auto handled = QXmpp::handleIqElements<QXmppVersionIq>(element, e2eeMetadata, client, [](QXmppVersionIq iq) -> std::variant<QXmppVersionIq, QXmppStanza::Error> {
///     if (iq.type() == QXmppIq::Get) {
///         QXmppVersionIq response;
///         response.setName("MyApp");
///         response.setVersion("1.0");
///         // id, to and type of the IQ are set automatically.
///         return response;
///     } else if (iq.type() == QXmppIq::Set) {
///         return QXmppStanza::Error(QXmppStanza::Error::Cancel, QXmppStanza::Error::BadRequest, "IQ must be of type 'get'.");
///     }
/// });
/// ```
///
/// It is also possible to handle multiple IQ types.
/// ```
/// auto handled = QXmpp::handleIqElements<QXmppVersionIq, QXmppVCardIq>(
///     element, e2eeMetadata, client, [](std::variant<QXmppVersionIq, QXmppVCardIq> iqVariant) {
///     // ...
/// });
/// ```
/// It doesn't need to be a std::variant, it's only important that the object is callable with all
/// the IQ types. You can for example use different lambdas per type using this 'overloaded' helper.
/// ```
/// template<class... Ts>
/// struct overloaded : Ts... {
///     using Ts::operator()...;
/// };
/// // explicit deduction guide (not needed as of C++20)
/// template<class... Ts>
/// overloaded(Ts...) -> overloaded<Ts...>;
///
/// auto handled = QXmpp::handleIqElements<QXmppVersionIq, QXmppVCardIq>(
///     element, e2eeMetadata, client, overloaded {
///         [](QXmppVersionIq iq) {
///             // ...
///         },
///         [](QXmppVCardIq iq) {
///             // ...
///         }
///     });
/// ```
///
/// And another option is to an object with `handleIq()` functions.
/// ```
/// auto handled = QXmpp::handleIqElements<QXmppVersionIq, QXmppVCardIq>(element, e2eeMetadata, client, this);
/// // will call this->handleIq(QXmppVersionIq) or this->handleIq(QXmppVCardIq)
/// ```
///
/// The return type of the handler function can be:
///  1. an QXmppIq based type
///  2. a std::variant of QXmppIq based types (multiple are possible) and optionally also QXmppStanza::Error
///  3. a QXmppTask of 1. or 2.
///
/// You don't need to set the values for id or the to address on the IQ result because that's done
/// automatically. Unless you want to return an error IQ you also don't need to set the IQ type.
///
/// If you return an QXmppStanza::Error, a normal QXmppIq with the error will be sent.
///
/// The provided optional QXmppE2eeMetadata is set on the parsed IQ and used to correctly encrypt
/// the IQ response using QXmppClient::reply().
///
/// \param element The DOM element that might contain an IQ.
/// \param e2eeMetadata The end-to-end encryption metadata that is used to encrypt the response
/// correctly and to be set on the parsed IQ.
/// \param client The client that should be used to send the response.
/// \param handler Function that can handle the IQ types from the template parameter or an object
/// that has handleIq() functions for each of the IQ types.
/// \return Whether the IQ could be parsed, handled and a response was or will be sent.
/// \since QXmpp 1.5
///
template<typename... IqTypes, typename Handler>
bool handleIqRequests(const QDomElement &element,
                      const std::optional<QXmppE2eeMetadata> &e2eeMetadata,
                      QXmppClient *client,
                      Handler handler)
{
    if (auto [isRequest, tagName, xmlns] = Private::checkIsIqRequest(element); isRequest) {
        return (Private::handleIqType<IqTypes>(handler, client, element, e2eeMetadata, tagName, xmlns) || ...);
    }
    return false;
}

///
/// Parses IQ requests, calls a handler and sends an IQ result or error.
///
/// It is the easiest to explain this function with a few examples.
///
/// ```
/// auto handled = QXmpp::handleIqElements<QXmppVersionIq>(element, client, [](QXmppVersionIq iq) -> std::variant<QXmppVersionIq, QXmppStanza::Error> {
///     if (iq.type() == QXmppIq::Get) {
///         QXmppVersionIq response;
///         response.setName("MyApp");
///         response.setVersion("1.0");
///         // id, to and type of the IQ are set automatically.
///         return response;
///     } else if (iq.type() == QXmppIq::Set) {
///         return QXmppStanza::Error(QXmppStanza::Error::Cancel, QXmppStanza::Error::BadRequest, "IQ must be of type 'get'.");
///     }
/// });
/// ```
///
/// It is also possible to handle multiple IQ types.
/// ```
/// auto handled = QXmpp::handleIqElements<QXmppVersionIq, QXmppVCardIq>(
///     element, client, [](std::variant<QXmppVersionIq, QXmppVCardIq> iqVariant) {
///     // ...
/// });
/// ```
/// It doesn't need to be a std::variant, it's only important that the object is callable with all
/// the IQ types. You can for example use different lambdas per type using this 'overloaded' helper.
/// ```
/// template<class... Ts>
/// struct overloaded : Ts... {
///     using Ts::operator()...;
/// };
/// // explicit deduction guide (not needed as of C++20)
/// template<class... Ts>
/// overloaded(Ts...) -> overloaded<Ts...>;
///
/// auto handled = QXmpp::handleIqElements<QXmppVersionIq, QXmppVCardIq>(
///     element, client, overloaded {
///         [](QXmppVersionIq iq) {
///             // ...
///         },
///         [](QXmppVCardIq iq) {
///             // ...
///         }
///     });
/// ```
///
/// And another option is to an object with `handleIq()` functions.
/// ```
/// auto handled = QXmpp::handleIqElements<QXmppVersionIq, QXmppVCardIq>(element, client, this);
/// // will call this->handleIq(QXmppVersionIq) or this->handleIq(QXmppVCardIq)
/// ```
///
/// The return type of the handler function can be:
///  1. an QXmppIq based type
///  2. a std::variant of QXmppIq based types (multiple are possible) and optionally also QXmppStanza::Error
///  3. a QXmppTask of 1. or 2.
///
/// You don't need to set the values for id or the to address on the IQ result because that's done
/// automatically. Unless you want to return an error IQ you also don't need to set the IQ type.
///
/// If you return an QXmppStanza::Error, a normal QXmppIq with the error will be sent.
///
/// \param element The DOM element that might contain an IQ.
/// \param client The client that should be used to send the response.
/// \param handler Function that can handle the IQ types from the template parameter or an object
/// that has handleIq() functions for each of the IQ types.
/// \return Whether the IQ could be parsed, handled and a response was or will be sent.
/// \since QXmpp 1.5
///
template<typename... IqTypes, typename Handler>
bool handleIqRequests(const QDomElement &element, QXmppClient *client, Handler handler)
{
    return handleIqRequests<IqTypes...>(element, std::nullopt, client, std::forward<Handler>(handler));
}

}  // namespace QXmpp

#endif  // QXMPPIQHANDLING_H
