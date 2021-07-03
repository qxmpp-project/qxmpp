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

template<typename T>
QFuture<T> makeReadyFuture(T &&value)
{
    QFutureInterface<T> interface(QFutureInterfaceBase::Started);
    interface.reportResult(value);
    interface.reportFinished();
    return interface.future();
}

template<typename Result, typename Input, typename Converter>
QFuture<Result> chain(QFuture<Input> &&source, QObject *context, Converter task)
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

template<typename Result, typename IqType, typename Input, typename Converter>
Result parseIq(Input &&sendResult, Converter convert)
{
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

template<typename Result, typename IqType, typename Input>
Result parseIq(Input &&sendResult)
{
    return parseIq<Result, IqType>(std::move(sendResult), [](IqType &&iq) {
        // no conversion
        return iq;
    });
}

template<typename Result, typename IqType, typename Input, typename Converter>
QFuture<Result> chainIq(QFuture<Input> &&input, QObject *context, Converter convert)
{
    return chain<Result>(std::move(input), context, [convert { std::move(convert) }](Input &&input) -> Result {
        return parseIq<Result, IqType>(std::move(input), convert);
    });
}

template<typename Result, typename IqType, typename Input>
QFuture<Result> chainIq(QFuture<Input> &&input, QObject *context)
{
    return chain<Result>(std::move(input), context, [](Input &&sendResult) {
        return parseIq<Result, IqType>(sendResult);
    });
}

}

#endif  // QXMPPFUTUREUTILS_P_H
