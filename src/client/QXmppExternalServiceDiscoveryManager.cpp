// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppExternalServiceDiscoveryManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppExternalServiceDiscoveryIq.h"
#include "QXmppIqHandling.h"

using namespace QXmpp::Private;

///
/// \brief The QXmppExternalServiceDiscoveryManager class makes it possible to
/// discover information about external services from providers
/// as defined by \xep{0215, External Service Discovery}.
///
/// To make use of this manager, you need to instantiate it and load it into
/// the QXmppClient instance as follows:
///
/// \code
/// auto *manager = client->addNewExtension<QXmppExternalServiceDiscoveryManager>();
/// \endcode
///
/// \ingroup Managers
///
/// \since QXmpp 1.6
///
QXmppExternalServiceDiscoveryManager::QXmppExternalServiceDiscoveryManager()
{
}

QXmppExternalServiceDiscoveryManager::~QXmppExternalServiceDiscoveryManager() = default;

///
/// Requests external services from the specified XMPP entity.
///
/// \param jid  The target entity's JID.
/// \param node The target node (optional).
///
/// \since QXmpp 1.6
///
QXmppTask<QXmppExternalServiceDiscoveryManager::ServicesResult> QXmppExternalServiceDiscoveryManager::requestServices(const QString &jid, const QString &node)
{
    QXmppExternalServiceDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setTo(jid);

    return chainIq(client()->sendIq(std::move(request)), this, [](QXmppExternalServiceDiscoveryIq &&iq) -> ServicesResult {
        return iq.externalServices();
    });
}

/// \cond
QStringList QXmppExternalServiceDiscoveryManager::discoveryFeatures() const
{
    return { ns_external_service_discovery };
}
/// \endcond
