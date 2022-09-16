// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QcaInitializer_p.h"
#include <QtCrypto>

using namespace QCA;

namespace QXmpp::Private {

/// \cond
QcaInitializer::QcaInitializer()
    : d(createInitializer())
{
}

std::shared_ptr<Initializer> QcaInitializer::createInitializer()
{
    static std::weak_ptr<Initializer> initializer;
    if (initializer.expired()) {
        auto newInitializer = std::make_shared<Initializer>();
        initializer = newInitializer;
        return newInitializer;
    }
    return initializer.lock();
}
/// \endcond

}  // namespace QXmpp::Private
