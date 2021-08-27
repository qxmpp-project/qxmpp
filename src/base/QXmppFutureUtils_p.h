/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
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

#include <QXmppIq.h>

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

template<typename T, typename Handler>
void awaitLast(const QFuture<T> &future, QObject *context, Handler handler)
{
    auto *watcher = new QFutureWatcher<T>(context);
    QObject::connect(watcher, &QFutureWatcherBase::finished,
                     context, [watcher, handler { std::move(handler) }]() {
        auto future = watcher->future();
        handler(future.resultAt(future.resultCount() - 1));
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

template<typename T, typename Handler>
void await(const QFuture<T> &future, QObject *context, Handler handler)
{
    auto *watcher = new QFutureWatcher<T>(context);
    QObject::connect(watcher, &QFutureWatcherBase::finished,
                     context, [watcher, handler { std::move(handler) }]() {
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
                     context, [watcher, handler { std::move(handler) }]() {
        handler();
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

template<typename Result, typename Input, typename Converter>
auto chain(QFuture<Input> &&source, QObject *context, Converter task) -> QFuture<Result>
{
    auto resultInterface = std::make_shared<QFutureInterface<Result>>(QFutureInterfaceBase::Started);

    auto *watcher = new QFutureWatcher<Input>(context);
    QObject::connect(watcher, &QFutureWatcherBase::finished, context, [=]() {
        resultInterface->reportResult(task(watcher->result()));
        resultInterface->reportFinished();
        watcher->deleteLater();
    });
    watcher->setFuture(source);
    return resultInterface->future();
}

template<typename IqType, typename Input, typename Converter>
auto parseIq(Input &&sendResult, Converter convert) -> decltype(convert({}))
{
    using Result = decltype(convert({}));
    return std::visit(overloaded {
                          [convert { std::move(convert) }](const QDomElement &element) -> Result {
                              IqType iq;
                              iq.parse(element);
                              if (iq.type() == QXmppIq::Error) {
                                  return iq.error();
                              }
                              return convert(std::move(iq));
                          },
                          [](QXmpp::PacketState) -> Result {
                              using Error = QXmppStanza::Error;
                              return Error(Error::Wait, Error::UndefinedCondition,
                                           QStringLiteral("Couldn't send request: lost connection."));
                          },
                      },
                      sendResult);
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
auto chainIq(QFuture<Input> &&input, QObject *context, Converter convert) -> QFuture<decltype(convert({}))>
{
    using Result = decltype(convert({}));
    using IqType = std::decay_t<first_argument_t<Converter>>;
    return chain<Result>(std::move(input), context, [convert { std::move(convert) }](Input &&input) -> Result {
        return parseIq<IqType>(std::move(input), convert);
    });
}

template<typename Result, typename Input>
auto chainIq(QFuture<Input> &&input, QObject *context) -> QFuture<Result>
{
    // IQ type is first std::variant parameter
    using IqType = std::decay_t<decltype(std::get<0>(Result {}))>;
    return chain<Result>(std::move(input), context, [](Input &&sendResult) {
        return parseIq<IqType, Result>(sendResult);
    });
}

}

#endif  // QXMPPFUTUREUTILS_P_H
