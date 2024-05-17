// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <algorithm>
#include <functional>

namespace QXmpp::Private {

template<typename OutputVector, typename InputVector, typename Converter>
auto transform(const InputVector &input, Converter convert)
{
    OutputVector output;
    if constexpr (std::ranges::sized_range<InputVector>) {
        output.reserve(input.size());
    }
    for (const auto &value : input) {
        output.push_back(std::invoke(convert, value));
    }
    return output;
}

template<typename Vec, typename T>
auto contains(const Vec &vec, const T &value)
{
    return std::find(std::begin(vec), std::end(vec), value) != std::end(vec);
}

}  // namespace QXmpp::Private

#endif  // ALGORITHMS_H
