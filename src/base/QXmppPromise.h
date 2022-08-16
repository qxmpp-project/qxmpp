// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPROMISE_H
#define QXMPPPROMISE_H

#include "QXmppTask.h"

///
/// \brief Create and update QXmppTask objects to communicate results of asynchronous operations.
///
/// Unlike QFuture, this is not thread-safe. This avoids the need to do mutex locking at every
/// access though.
///
/// \ingroup Core classes
///
/// \since QXmpp 1.5
///
template<typename T>
class QXmppPromise
{
public:
    template<typename U = T, std::enable_if_t<std::is_void_v<U>> * = nullptr>
    QXmppPromise()
        : d(QXmpp::Private::TaskPrivate(nullptr))
    {
    }

    template<typename U = T, std::enable_if_t<!std::is_void_v<U>> * = nullptr>
    QXmppPromise()
        : d(QXmpp::Private::TaskPrivate([](void *r) { delete static_cast<T *>(r); }))
    {
    }

    ///
    /// Report that the asynchronous operation has finished, and call the connected handler of the
    /// QXmppTask<T> belonging to this promise.
    ///
    /// \param value The result of the asynchronous computation
    ///
#ifdef QXMPP_DOC
    void reportFinished(T &&value)
#else
    template<typename U, std::enable_if_t<!std::is_void_v<T> && std::is_base_of_v<T, U>> * = nullptr>
    void finish(U &&value)
#endif
    {
        Q_ASSERT(!d.isFinished());
        d.setFinished(true);
        if (d.continuation()) {
            if (d.isContextAlive()) {
                d.invokeContinuation(&value);
            }
        } else {
            d.setResult(new U(std::move(value)));
        }
    }

    /// \cond
    template<typename U, std::enable_if_t<!std::is_void_v<T> && std::is_constructible_v<T, U> && !std::is_base_of_v<T, U>> * = nullptr>
    void finish(U &&value)
    {
        Q_ASSERT(!d.isFinished());
        d.setFinished(true);
        if (d.continuation()) {
            if (d.isContextAlive()) {
                T convertedValue { std::move(value) };
                d.invokeContinuation(&convertedValue);
            }
        } else {
            d.setResult(new T(std::move(value)));
        }
    }

    template<typename U = T, std::enable_if_t<std::is_void_v<U>> * = nullptr>
    void finish()
    {
        Q_ASSERT(!d.isFinished());
        d.setFinished(true);
        if (d.continuation()) {
            if (d.isContextAlive()) {
                d.invokeContinuation(nullptr);
            }
        }
    }
    /// \endcond

    ///
    /// Obtain a handle to this promise that allows to obtain the value that will be produced
    /// asynchronously.
    ///
    QXmppTask<T> task()
    {
        return QXmppTask<T>(d);
    }

private:
    QXmpp::Private::TaskPrivate d;
};

#endif  // QXMPPPROMISE_H
