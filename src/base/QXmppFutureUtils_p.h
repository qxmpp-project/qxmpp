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
#include "QXmppSendResult.h"

#include <memory>
#include <variant>

#include <QFutureWatcher>
#include <QObject>

namespace QXmpp::Private {

// helper for std::visit
template<class... Ts>
struct overloaded : Ts...
{
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
struct first_argument
{
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
                     context, [watcher, handler = std::move(handler)]() mutable {
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
auto chain(const QFuture<Input> &source, QObject *context, Converter task) -> QFuture<Result>
{
    QFutureInterface<Result> resultInterface(QFutureInterfaceBase::Started);

    auto *watcher = new QFutureWatcher<Input>(context);
    QObject::connect(watcher, &QFutureWatcherBase::finished, context, [=]() mutable {
        resultInterface.reportResult(task(watcher->result()));
        resultInterface.reportFinished();
        watcher->deleteLater();
    });
    watcher->setFuture(source);
    return resultInterface.future();
}

template<typename IqType, typename Input, typename Converter>
auto parseIq(Input &&sendResult, Converter convert) -> decltype(convert({}))
{
    using Result = decltype(convert({}));
    return std::visit(overloaded {
                          [convert = std::move(convert)](const QDomElement &element) -> Result {
                              IqType iq;
                              iq.parse(element);
                              if (iq.type() == QXmppIq::Error) {
                                  return iq.error();
                              }
                              return convert(std::move(iq));
                          },
                          [](QXmpp::SendError error) -> Result {
                              using Error = QXmppStanza::Error;
                              return Error(Error::Wait, Error::UndefinedCondition,
                                           QStringLiteral("Couldn't send request: ") + error.text);
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

template<typename T>
void reportFinishedResult(QFutureInterface<T> &interface, const T &result)
{
    interface.reportResult(result);
    interface.reportFinished();
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

}  // namespace QXmpp::Private

#endif  // QXMPPFUTUREUTILS_P_H
