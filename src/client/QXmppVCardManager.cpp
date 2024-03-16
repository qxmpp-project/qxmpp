// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppVCardManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppTask.h"
#include "QXmppUtils.h"
#include "QXmppVCardIq.h"

using namespace QXmpp::Private;

class QXmppVCardManagerPrivate
{
public:
    QXmppVCardIq clientVCard;
    bool isClientVCardReceived;
};

QXmppVCardManager::QXmppVCardManager()
    : d(std::make_unique<QXmppVCardManagerPrivate>())
{
    d->isClientVCardReceived = false;
}

QXmppVCardManager::~QXmppVCardManager() = default;

///
/// This function requests the server for vCard of the specified jid.
/// Once received the signal vCardReceived() is emitted.
///
/// \param jid Jid of the specific entry in the roster
///
QXmppTask<QXmppVCardManager::IqResult> QXmppVCardManager::requestVCard(const QString &jid)
{
    QXmppVCardIq iq(jid);

    return chainIq(client()->sendIq(std::move(iq)), this, [this](QXmppVCardIq &&vcard) -> QXmppVCardManager::IqResult {
        handleReceivedVCard(vcard);
        return std::move(vcard);
    });
}

/// Returns the vCard of the connected client.
const QXmppVCardIq &QXmppVCardManager::clientVCard() const
{
    return d->clientVCard;
}

/// Sets the vCard of the connected client.
QXmppTask<QXmppVCardManager::Result> QXmppVCardManager::setClientVCard(const QXmppVCardIq &clientVCard)
{
    auto iq = clientVCard;
    iq.setTo({});
    iq.setFrom({});
    iq.setType(QXmppIq::Set);

    return chainIq(client()->sendIq(std::move(iq)), this, [this](QXmppVCardIq &&vcard) -> QXmppVCardManager::Result {
        handleReceivedVCard(std::move(vcard));
        return QXmpp::Success {};
    });
}

///
/// This function requests the server for vCard of the connected user itself.
/// Once received the signal clientVCardReceived() is emitted. Received vCard
/// can be get using clientVCard().
///
QXmppTask<QXmppVCardManager::IqResult> QXmppVCardManager::requestClientVCard()
{
    return requestVCard();
}

/// Returns true if vCard of the connected client has been received else false.
bool QXmppVCardManager::isClientVCardReceived() const
{
    return d->isClientVCardReceived;
}

/// \cond
QStringList QXmppVCardManager::discoveryFeatures() const
{
    return {
        // XEP-0054: vcard-temp
        ns_vcard.toString(),
    };
}
/// \endcond

void QXmppVCardManager::handleReceivedVCard(const QXmppVCardIq &vcard)
{
    if (vcard.from().isEmpty() || vcard.from() == client()->configuration().jidBare()) {
        d->clientVCard = vcard;
        d->isClientVCardReceived = true;
        Q_EMIT clientVCardReceived();
    }

    Q_EMIT vCardReceived(vcard);
}
