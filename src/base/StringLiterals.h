// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef STRINGLITERALS_H
#define STRINGLITERALS_H

#include <QString>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using namespace Qt::Literals::StringLiterals;
#else
namespace QXmpp::Private {

template<std::size_t N>
struct StringLiteralData {
    char16_t data[N];
    std::size_t size = N;

    constexpr StringLiteralData(const char16_t (&str)[N])
    {
        std::ranges::copy(str, data);
    }
};

template<std::size_t N>
struct StaticStringData {
    QArrayData str = Q_STATIC_STRING_DATA_HEADER_INITIALIZER(N - 1);
    char16_t data[N];

    StaticStringData(const char16_t (&str)[N])
    {
        std::ranges::copy(str, data);
    }

    QStringData *data_ptr() const
    {
        return const_cast<QStringData *>(static_cast<const QStringData *>(&str));
    }
};

}  // namespace QXmpp::Private

template<QXmpp::Private::StringLiteralData str>
QString operator""_s()
{
    static const auto staticData = QXmpp::Private::StaticStringData<str.size>(str.data);
    return QString(QStringDataPtr { staticData.data_ptr() });
}
#endif

#endif  // STRINGLITERALS_H
