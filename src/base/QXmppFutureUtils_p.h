// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPFUTUREUTILS_P_H
#define QXMPPFUTUREUTILS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "QXmppIq.h"
#include "QXmppPromise.h"
#include "QXmppSendResult.h"

#include <memory>
#include <variant>

#include <QFutureWatcher>
#include <QObject>

namespace QXmpp::Private {

// helper for std::visit
template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// Variation of std::visit allowing to forward unhandled types
template<typename ReturnType, typename T, typename Visitor>
auto visitForward(T variant, Visitor visitor)
{
    return std::visit([&](auto &&value) -> ReturnType {
        using ValueType = std::decay_t<decltype(value)>;
        if constexpr (std::is_invocable_v<Visitor, ValueType>) {
            return visitor(std::move(value));
        } else {
            return value;
        }
    },
                      std::forward<T>(variant));
}

template<typename F, typename Ret, typename A, typename... Rest>
A lambda_helper(Ret (F::*)(A, Rest...));

template<typename F, typename Ret, typename A, typename... Rest>
A lambda_helper(Ret (F::*)(A, Rest...) const);

template<typename F>
struct first_argument {
    using type = decltype(lambda_helper(&F::operator()));
};

template<typename F>
using first_argument_t = typename first_argument<F>::type;

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
template<typename T>
QFuture<T> makeReadyFuture(T &&value) { return QtFuture::makeReadyValueFuture(std::move(value)); }
#elif QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
using QtFuture::makeReadyFuture;
#else
template<typename T>
QFuture<T> makeReadyFuture(T &&value)
{
    QFutureInterface<T> interface(QFutureInterfaceBase::Started);
    interface.reportResult(std::move(value));
    interface.reportFinished();
    return interface.future();
}

inline QFuture<void> makeReadyFuture()
{
    using State = QFutureInterfaceBase::State;
    return QFutureInterface<void>(State(State::Started | State::Finished)).future();
}
#endif

template<typename T>
QXmppTask<T> makeReadyTask(T &&value)
{
    QXmppPromise<T> promise;
    promise.finish(std::move(value));
    return promise.task();
}

inline QXmppTask<void> makeReadyTask()
{
    QXmppPromise<void> promise;
    promise.finish();
    return promise.task();
}

template<typename T, typename Handler>
void await(const QFuture<T> &future, QObject *context, Handler handler)
{
    auto *watcher = new QFutureWatcher<T>(context);
    QObject::connect(watcher, &QFutureWatcherBase::finished,
                     context, [watcher, handler = std::move(handler)]() mutable {
                         handler(watcher->result());
                         watcher->deleteLater();
                     });
    watcher->setFuture(future);
}

template<typename Handler>
void await(const QFuture<void> &future, QObject *context, Handler handler)
{
    auto *watcher = new QFutureWatcher<void>(context);
    QObject::connect(watcher, &QFutureWatcherBase::finished,
                     context, [watcher, handler = std::move(handler)]() mutable {
                         handler();
                         watcher->deleteLater();
                     });
    watcher->setFuture(future);
}

template<typename Result, typename Input, typename Converter>
auto chain(QXmppTask<Input> &&source, QObject *context, Converter task) -> QXmppTask<Result>
{
    QXmppPromise<Result> promise;

    source.then(context, [=](Input &&input) mutable {
        promise.finish(task(std::move(input)));
    });
    return promise.task();
}

template<typename IqType, typename Input, typename Converter>
auto parseIq(Input &&sendResult, Converter convert) -> decltype(convert({}))
{
    using Result = decltype(convert({}));
    return std::visit(overloaded {
                          [convert = std::move(convert)](const QDomElement &element) -> Result {
                              IqType iq;
                              iq.parse(element);
                              return convert(std::move(iq));
                          },
                          [](QXmppError &&error) -> Result {
                              return error;
                          },
                      },
                      std::move(sendResult));
}

template<typename IqType, typename Result, typename Input>
auto parseIq(Input &&sendResult) -> Result
{
    return parseIq<IqType>(std::move(sendResult), [](IqType &&iq) -> Result {
        // no conversion
        return iq;
    });
}

template<typename Input, typename Converter>
auto chainIq(QXmppTask<Input> &&input, QObject *context, Converter convert) -> QXmppTask<decltype(convert({}))>
{
    using Result = decltype(convert({}));
    using IqType = std::decay_t<first_argument_t<Converter>>;
    return chain<Result>(std::move(input), context, [convert = std::move(convert)](Input &&input) -> Result {
        return parseIq<IqType>(std::move(input), convert);
    });
}

template<typename Result, typename Input>
auto chainIq(QXmppTask<Input> &&input, QObject *context) -> QXmppTask<Result>
{
    // IQ type is first std::variant parameter
    using IqType = std::decay_t<decltype(std::get<0>(Result {}))>;
    return chain<Result>(std::move(input), context, [](Input &&sendResult) mutable {
        return parseIq<IqType, Result>(sendResult);
    });
}

template<typename T, typename Err, typename Function>
auto mapSuccess(std::variant<T, Err> var, Function lambda)
{
    using MapResult = std::decay_t<decltype(lambda({}))>;
    using MappedVariant = std::variant<MapResult, Err>;
    return std::visit(overloaded {
                          [lambda = std::move(lambda)](T val) -> MappedVariant {
                              return lambda(std::move(val));
                          },
                          [](Err err) -> MappedVariant {
                              return err;
                          } },
                      std::move(var));
}

template<typename T, typename Err>
auto mapToSuccess(std::variant<T, Err> var)
{
    return mapSuccess(std::move(var), [](T) {
        return Success();
    });
}

template<typename T, typename Err>
auto chainSuccess(QXmppTask<std::variant<T, Err>> &&source, QObject *context) -> QXmppTask<std::variant<QXmpp::Success, QXmppError>>
{
    return chain<std::variant<QXmpp::Success, QXmppError>>(std::move(source), context, mapToSuccess<T, Err>);
}

template<typename Input, typename Converter>
auto chainMapSuccess(QXmppTask<Input> &&source, QObject *context, Converter convert)
{
    return chain<std::variant<decltype(convert({})), QXmppError>>(std::move(source), context, [convert](Input &&input) {
        return mapSuccess(std::move(input), convert);
    });
}

}  // namespace QXmpp::Private

#endif  // QXMPPFUTUREUTILS_P_H
