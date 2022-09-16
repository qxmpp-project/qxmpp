// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QCAINITIALIZER_P_H
#define QCAINITIALIZER_P_H

#include "QXmppGlobal.h"

#include <memory>

namespace QCA {
class Initializer;
}

namespace QXmpp::Private {

// export required for tests
class QXMPP_EXPORT QcaInitializer
{
public:
    QcaInitializer();

private:
    static std::shared_ptr<QCA::Initializer> createInitializer();
    std::shared_ptr<QCA::Initializer> d;
};

}  // namespace QXmpp::Private

#endif  // QCAINITIALIZER_P_H
