// SPDX-FileCopyrightText: 2023 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppTask.h"

#include <QDebug>

namespace QXmpp::Private {

struct TaskData
{
    QPointer<QObject> context;
    std::function<void(TaskPrivate &, void *)> continuation;
    void *result = nullptr;
    void (*freeResult)(void *);
    bool finished = false;

    ~TaskData()
    {
        if (freeResult) {
            freeResult(result);
        }
    }
};

}  // namespace QXmpp::Private

QXmpp::Private::TaskPrivate::TaskPrivate(void (*freeResult)(void *))
    : d(std::make_shared<QXmpp::Private::TaskData>())
{
    d->freeResult = freeResult;
}

QXmpp::Private::TaskPrivate::~TaskPrivate()
{
}

bool QXmpp::Private::TaskPrivate::isFinished() const
{
    return d->finished;
}

void QXmpp::Private::TaskPrivate::setFinished(bool finished)
{
    d->finished = finished;
}

bool QXmpp::Private::TaskPrivate::isContextAlive()
{
    return !d->context.isNull();
}

void QXmpp::Private::TaskPrivate::setContext(QObject *obj)
{
    d->context = obj;
}

void *QXmpp::Private::TaskPrivate::result() const
{
    return d->result;
}

void QXmpp::Private::TaskPrivate::setResult(void *result)
{
    if (d->freeResult) {
        d->freeResult(d->result);
    }
    d->result = result;
}

const std::function<void(QXmpp::Private::TaskPrivate &, void *)> QXmpp::Private::TaskPrivate::continuation() const
{
    return d->continuation;
}

void QXmpp::Private::TaskPrivate::setContinuation(std::function<void(TaskPrivate &, void *)> &&continuation)
{
    d->continuation = continuation;
}

void QXmpp::Private::TaskPrivate::invokeContinuation(void *result)
{
    d->continuation(*this, result);
}
