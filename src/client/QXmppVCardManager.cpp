// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppVCardManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppUtils.h"
#include "QXmppVCardIq.h"

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

/// This function requests the server for vCard of the specified jid.
/// Once received the signal vCardReceived() is emitted.
///
/// \param jid Jid of the specific entry in the roster
///
QString QXmppVCardManager::requestVCard(const QString &jid)
{
    QXmppVCardIq request(jid);
    if (client()->sendPacket(request)) {
        return request.id();
    } else {
        return QString();
    }
}

/// Returns the vCard of the connected client.
///
/// \return QXmppVCard
///
const QXmppVCardIq &QXmppVCardManager::clientVCard() const
{
    return d->clientVCard;
}

/// Sets the vCard of the connected client.
///
/// \param clientVCard QXmppVCard
///
void QXmppVCardManager::setClientVCard(const QXmppVCardIq &clientVCard)
{
    d->clientVCard = clientVCard;
    d->clientVCard.setTo("");
    d->clientVCard.setFrom("");
    d->clientVCard.setType(QXmppIq::Set);
    client()->sendPacket(d->clientVCard);
}

/// This function requests the server for vCard of the connected user itself.
/// Once received the signal clientVCardReceived() is emitted. Received vCard
/// can be get using clientVCard().
QString QXmppVCardManager::requestClientVCard()
{
    return requestVCard();
}

/// Returns true if vCard of the connected client has been
/// received else false.
///
/// \return bool
///
bool QXmppVCardManager::isClientVCardReceived() const
{
    return d->isClientVCardReceived;
}

/// \cond
QStringList QXmppVCardManager::discoveryFeatures() const
{
    // XEP-0054: vcard-temp
    return QStringList() << ns_vcard;
}

bool QXmppVCardManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == "iq" && QXmppVCardIq::isVCard(element)) {
        QXmppVCardIq vCardIq;
        vCardIq.parse(element);

        if (vCardIq.from().isEmpty() || vCardIq.from() == client()->configuration().jidBare()) {
            d->clientVCard = vCardIq;
            d->isClientVCardReceived = true;
            Q_EMIT clientVCardReceived();
        }

        Q_EMIT vCardReceived(vCardIq);

        return true;
    }

    return false;
}
/// \endcond
