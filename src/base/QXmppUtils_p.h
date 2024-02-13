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
#include <QDomElement>

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
inline auto toString60(QStringView s)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return s;
#else
    return s.toString();
#endif
}
inline auto toString65(QStringView s)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    return s;
#else
    return s.toString();
#endif
}

// XML streams
void writeOptionalXmlAttribute(QXmlStreamWriter *stream, QStringView name, QStringView value);
void writeXmlTextElement(QXmlStreamWriter *stream, QStringView name, QStringView value);

//
// DOM
//

bool isIqType(const QDomElement &, QStringView tagName, QStringView xmlns);
QDomElement firstChildElement(const QDomElement &, QStringView tagName = {}, QStringView xmlNs = {});
QDomElement nextSiblingElement(const QDomElement &, QStringView tagName = {}, QStringView xmlNs = {});

struct DomChildElements
{
    QDomElement parent;
    QStringView tagName;
    QStringView namespaceUri;

    struct EndIterator
    {
    };
    struct Iterator
    {
        Iterator operator++()
        {
            el = nextSiblingElement(el, tagName, namespaceUri);
            return *this;
        }
        bool operator!=(EndIterator) const { return !el.isNull(); }
        const QDomElement &operator*() const { return el; }

        QDomElement el;
        QStringView tagName;
        QStringView namespaceUri;
    };

    Iterator begin() const { return { firstChildElement(parent, tagName, namespaceUri), tagName, namespaceUri }; }
    EndIterator end() const { return {}; }
};

inline DomChildElements iterChildElements(const QDomElement &el, QStringView tagName = {}, QStringView namespaceUri = {}) { return DomChildElements { el, tagName, namespaceUri }; }

QXMPP_EXPORT QByteArray generateRandomBytes(uint32_t minimumByteCount, uint32_t maximumByteCount);
QXMPP_EXPORT void generateRandomBytes(uint8_t *bytes, uint32_t byteCount);
float calculateProgress(qint64 transferred, qint64 total);

}  // namespace QXmpp::Private

#endif  // QXMPPUTILS_P_H
