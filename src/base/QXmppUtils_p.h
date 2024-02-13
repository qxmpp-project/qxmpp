// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPUTILS_P_H
#define QXMPPUTILS_P_H

#include "QXmppGlobal.h"

#include <array>
#include <stdint.h>

#include <QByteArray>

class QDomElement;
class QXmlStreamWriter;

namespace QXmpp::Private {

// std::array helper
namespace detail {
    template<class T, std::size_t N, std::size_t... I>
    constexpr std::array<std::remove_cv_t<T>, N>
    to_array_impl(T (&&a)[N], std::index_sequence<I...>)
    {
        return { { std::move(a[I])... } };
    }
}  // namespace detail

template<class T, std::size_t N>
constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&&a)[N])
{
    return detail::to_array_impl(std::move(a), std::make_index_sequence<N> {});
}

// Helper for Q(Any)StringView overloads that were added later
inline auto toString65(QStringView s)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    return s;
#else
    return s.toString();
#endif
}

// XML streams
void writeOptionalXmlAttribute(QXmlStreamWriter *stream, const QString &name, const QString &value);
void writeXmlTextElement(QXmlStreamWriter *stream, const QString &name, const QString &value);

// DOM
QDomElement firstChildElement(const QDomElement &, QStringView tagName, QStringView xmlNs = {});
QDomElement firstChildElement(const QDomElement &, QStringView tagName, const char *xmlNs);

QXMPP_EXPORT QByteArray generateRandomBytes(uint32_t minimumByteCount, uint32_t maximumByteCount);
QXMPP_EXPORT void generateRandomBytes(uint8_t *bytes, uint32_t byteCount);
float calculateProgress(qint64 transferred, qint64 total);

}  // namespace QXmpp::Private

#endif  // QXMPPUTILS_P_H
