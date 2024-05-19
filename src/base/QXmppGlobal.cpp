// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppGlobal_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <array>

using namespace QXmpp::Private;

/// \cond
constexpr auto ENCRYPTION_NAMESPACES = to_array<QStringView>({
    {},
    {},
    ns_otr,
    ns_legacy_openpgp,
    ns_ox,
    ns_omemo,
    ns_omemo_1,
    ns_omemo_2,
});

constexpr auto ENCRYPTION_NAMES = to_array<QStringView>({
    {},
    {},
    u"OTR",
    u"Legacy OpenPGP",
    u"OpenPGP for XMPP (OX)",
    u"OMEMO",
    u"OMEMO 1",
    u"OMEMO 2",
});

std::optional<QXmpp::EncryptionMethod> QXmpp::Private::encryptionFromString(QStringView str)
{
    auto itr = std::find(ENCRYPTION_NAMESPACES.begin(), ENCRYPTION_NAMESPACES.end(), str);
    if (itr != ENCRYPTION_NAMESPACES.end()) {
        return QXmpp::EncryptionMethod(std::distance(ENCRYPTION_NAMESPACES.begin(), itr));
    }
    return {};
}

QStringView QXmpp::Private::encryptionToString(EncryptionMethod encryption)
{
    return ENCRYPTION_NAMESPACES[std::size_t(encryption)];
}

QStringView QXmpp::Private::encryptionToName(EncryptionMethod encryption)
{
    return ENCRYPTION_NAMES[std::size_t(encryption)];
}
/// \endcond
