// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppGlobal_p.h"

#include <QList>

/// \cond
static const QStringList ENCRYPTION_NAMESPACES = {
    QString(),
    QString(),
    ns_otr,
    ns_legacy_openpgp,
    ns_ox,
    ns_omemo,
    ns_omemo_1,
    ns_omemo_2
};

static const QStringList ENCRYPTION_NAMES = {
    QString(),
    QString(),
    QStringLiteral("OTR"),
    QStringLiteral("Legacy OpenPGP"),
    QStringLiteral("OpenPGP for XMPP (OX)"),
    QStringLiteral("OMEMO"),
    QStringLiteral("OMEMO 1"),
    QStringLiteral("OMEMO 2")
};

std::optional<QXmpp::Encryption> QXmpp::Private::encryptionFromString(const QString &str)
{
    int index = ENCRYPTION_NAMESPACES.indexOf(str);
    if (index < 0) {
        return {};
    }
    return QXmpp::Encryption(index);
}

QString QXmpp::Private::encryptionToString(Encryption encryption)
{
    return ENCRYPTION_NAMESPACES.at(int(encryption));
}

QString QXmpp::Private::encryptionToName(Encryption encryption)
{
    return ENCRYPTION_NAMES.at(int(encryption));
}
/// \endcond
