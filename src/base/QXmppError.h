// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPERROR_H
#define QXMPPERROR_H

#include "QXmppGlobal.h"

#include <any>
#include <optional>

class QFileDevice;
class QIODevice;
class QNetworkReply;

struct QXMPP_EXPORT QXmppError
{
    QString description;
    std::any error;

    static QXmppError fromIoDevice(const QIODevice &device);
    static QXmppError fromNetworkReply(const QNetworkReply &reply);
    static QXmppError fromFileDevice(const QFileDevice &file);

    bool isFileError() const;
    bool isNetworkError() const;
    bool isStanzaError() const;

    template<typename T>
    bool holdsType() const
    {
        return error.type().hash_code() == typeid(T).hash_code();
    }
    template<typename T>
    std::optional<T> value() const
    {
        // any_cast always checks this, to avoid an additional check we use exceptions
        try {
            return std::any_cast<T>(error);
        } catch (std::bad_any_cast) {
            return {};
        }
    }
    template<typename T>
    std::optional<T> takeValue()
    {
        // we can't use unchecked any_cast with moving because we can't access the error after a
        // failed any_cast
        if (error.type().hash_code() == typeid(T).hash_code()) {
            auto value = std::any_cast<T>(std::move(error));
            error = std::any();
            return value;
        }
        return {};
    }
};

#endif  // QXMPPERROR_H
