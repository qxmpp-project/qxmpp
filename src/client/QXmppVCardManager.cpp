// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppVCardManager.h"

#include "QXmppAccountMigrationManager.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppError.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppTask.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"
#include "QXmppVCardIq.h"

#include "StringLiterals.h"

using namespace QXmpp::Private;

namespace QXmpp::Private {

struct VCardData {
    QXmppVCardIq vCard;

    static std::variant<VCardData, QXmppError> fromDom(const QDomElement &el)
    {
        Q_ASSERT(el.tagName() == u"vcard");
        Q_ASSERT(el.namespaceURI() == ns_qxmpp_export);

        auto vCardEl = firstChildElement(el, u"vCard", ns_vcard);
        if (vCardEl.isNull()) {
            return QXmppError { u"Missing required <vCard/> element."_s, {} };
        }

        VCardData d;
        // element needs to be the parent of <vCard/>
        d.vCard.parseElementFromChild(el);
        return d;
    }

    void toXml(QXmlStreamWriter &writer) const
    {
        writer.writeStartElement(QSL65("vcard"));
        vCard.toXmlElementFromChild(&writer);
        writer.writeEndElement();
    }
};

void serializeVCardData(const VCardData &data, QXmlStreamWriter &writer)
{
    data.toXml(writer);
}

}  // namespace QXmpp::Private

class QXmppVCardManagerPrivate
{
public:
    QXmppVCardIq clientVCard;
    bool isClientVCardReceived = false;
};

QXmppVCardManager::QXmppVCardManager()
    : d(std::make_unique<QXmppVCardManagerPrivate>())
{
    QXmppExportData::registerExtension<VCardData, VCardData::fromDom, serializeVCardData>(u"vcard", ns_qxmpp_export);
}

QXmppVCardManager::~QXmppVCardManager() = default;

///
/// Fetches the VCard of a bare JID.
///
/// \since QXmpp 1.8
///
QXmppTask<QXmppVCardManager::VCardIqResult> QXmppVCardManager::fetchVCard(const QString &bareJid)
{
    return chainIq<VCardIqResult>(client()->sendIq(QXmppVCardIq(bareJid)), this);
}

///
/// Sets the VCard of the currently connected account.
///
/// \since QXmpp 1.8
///
QXmppTask<QXmppVCardManager::Result> QXmppVCardManager::setVCard(const QXmppVCardIq &vCard)
{
    auto vCardIq = vCard;
    vCardIq.setTo(client()->configuration().jidBare());
    vCardIq.setFrom({});
    vCardIq.setType(QXmppIq::Set);
    return client()->sendGenericIq(std::move(vCardIq));
}

///
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
const QXmppVCardIq &QXmppVCardManager::clientVCard() const
{
    return d->clientVCard;
}

/// Sets the vCard of the connected client.
void QXmppVCardManager::setClientVCard(const QXmppVCardIq &clientVCard)
{
    d->clientVCard = clientVCard;
    d->clientVCard.setTo({});
    d->clientVCard.setFrom({});
    d->clientVCard.setType(QXmppIq::Set);
    client()->sendPacket(d->clientVCard);
}

///
/// This function requests the server for vCard of the connected user itself.
/// Once received the signal clientVCardReceived() is emitted. Received vCard
/// can be get using clientVCard().
///
QString QXmppVCardManager::requestClientVCard()
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

bool QXmppVCardManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == u"iq" && QXmppVCardIq::isVCard(element)) {
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

void QXmppVCardManager::onRegistered(QXmppClient *client)
{
    if (auto manager = client->findExtension<QXmppAccountMigrationManager>()) {
        using DataResult = std::variant<VCardData, QXmppError>;

        auto importData = [this](const VCardData &data) {
            return setVCard(data.vCard);
        };

        auto exportData = [this]() {
            return chain<DataResult>(fetchVCard(this->client()->configuration().jidBare()), this, [](auto &&result) {
                return mapSuccess(std::move(result), [](QXmppVCardIq &&iq) -> VCardData { return { iq }; });
            });
        };

        manager->registerExportData<VCardData>(importData, exportData);
    }
}

void QXmppVCardManager::onUnregistered(QXmppClient *client)
{
    if (auto manager = client->findExtension<QXmppAccountMigrationManager>()) {
        manager->unregisterExportData<QXmppVCardIq>();
    }
}
/// \endcond
