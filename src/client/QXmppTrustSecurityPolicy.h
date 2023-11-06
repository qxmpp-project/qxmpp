// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QMetaType>

namespace QXmpp {

///
/// Security policy to decide which public long-term keys are used for encryption because they are
/// trusted
///
/// \since QXmpp 1.5
///
enum TrustSecurityPolicy {
    /// New keys must be trusted manually.
    NoSecurityPolicy,
    /// New keys are trusted automatically until the first authentication but automatically
    /// distrusted afterwards. \see \xep{0450, Automatic Trust Management (ATM)}
    Toakafa,
};

}  // namespace QXmpp

Q_DECLARE_METATYPE(QXmpp::TrustSecurityPolicy)
