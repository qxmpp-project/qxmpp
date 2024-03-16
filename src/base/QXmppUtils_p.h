// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPUTILS_P_H
#define QXMPPUTILS_P_H

#include "QXmppGlobal.h"

#include <array>
#include <functional>
#include <optional>
#include <stdint.h>

#include <QByteArray>
#include <QDomElement>

class QDomElement;
class QXmlStreamWriter;
class QXmppNonza;

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

// QStringLiteral for Qt < 6.5, otherwise uses string view
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#define QSL65(text) u"" text
#else
#define QSL65(text) QStringLiteral(text)
#endif

// Enum parsing
template<typename Enum, std::size_t N>
std::optional<Enum> enumFromString(const std::array<QStringView, N> &values, QStringView str)
{
    auto itr = std::find(values.begin(), values.end(), str);
    if (itr != values.end()) {
        return Enum(std::distance(values.begin(), itr));
    }
    return {};
}

// XML streams
void writeOptionalXmlAttribute(QXmlStreamWriter *stream, QStringView name, QStringView value);
void writeXmlTextElement(QXmlStreamWriter *stream, QStringView name, QStringView value);
void writeXmlTextElement(QXmlStreamWriter *writer, QStringView name, QStringView xmlns, QStringView value);
void writeOptionalXmlTextElement(QXmlStreamWriter *writer, QStringView name, QStringView value);
void writeEmptyElement(QXmlStreamWriter *writer, QStringView name, QStringView xmlns);

// Base64
std::optional<QByteArray> parseBase64(const QString &);
inline QString serializeBase64(const QByteArray &data) { return QString::fromUtf8(data.toBase64()); }

//
// DOM
//

bool isIqType(const QDomElement &, QStringView tagName, QStringView xmlns);
QDomElement firstChildElement(const QDomElement &, QStringView tagName = {}, QStringView xmlNs = {});
QDomElement nextSiblingElement(const QDomElement &, QStringView tagName = {}, QStringView xmlNs = {});

struct DomChildElements {
    QDomElement parent;
    QStringView tagName;
    QStringView namespaceUri;

    struct EndIterator { };
    struct Iterator {
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

QByteArray serializeXml(const void *packet, void (*toXml)(const void *, QXmlStreamWriter *));
template<typename T>
inline QByteArray serializeXml(const T &packet)
{
    return serializeXml(&packet, [](const void *packet, QXmlStreamWriter *w) {
        std::invoke(&T::toXml, reinterpret_cast<const T *>(packet), w);
    });
}

QXMPP_EXPORT QByteArray generateRandomBytes(uint32_t minimumByteCount, uint32_t maximumByteCount);
QXMPP_EXPORT void generateRandomBytes(uint8_t *bytes, uint32_t byteCount);
float calculateProgress(qint64 transferred, qint64 total);

QXMPP_EXPORT std::pair<QString, int> parseHostAddress(const QString &address);

}  // namespace QXmpp::Private

#endif  // QXMPPUTILS_P_H
