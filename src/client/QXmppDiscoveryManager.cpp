// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppDiscoveryManager.h"

#include "QXmppClient.h"
#include "QXmppClient_p.h"
#include "QXmppConstants_p.h"
#include "QXmppDataForm.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppIqHandling.h"

#include "StringLiterals.h"

#include <QCoreApplication>
#include <QDomElement>

using namespace QXmpp::Private;

class QXmppDiscoveryManagerPrivate
{
public:
    QString clientCapabilitiesNode;
    QString clientCategory;
    QString clientType;
    QString clientName;
    QXmppDataForm clientInfoForm;
};

///
/// \typedef QXmppDiscoveryManager::InfoResult
///
/// Contains the discovery information result in the form of an QXmppDiscoveryIq
/// or (in case the request did not succeed) a QXmppStanza::Error.
///
/// \since QXmpp 1.5
///

///
/// \typedef QXmppDiscoveryManager::ItemsResult
///
/// Contains a list of service discovery items or (in case the request did not
/// succeed) a QXmppStanza::Error.
///
/// \since QXmpp 1.5
///

QXmppDiscoveryManager::QXmppDiscoveryManager()
    : d(new QXmppDiscoveryManagerPrivate)
{
    d->clientCapabilitiesNode = u"https://github.com/qxmpp-project/qxmpp"_s;
    d->clientCategory = u"client"_s;
#if defined Q_OS_ANDROID || defined Q_OS_BLACKBERRY || defined Q_OS_IOS || defined Q_OS_WP
    d->clientType = u"phone"_s;
#else
    d->clientType = u"pc"_s;
#endif
    if (qApp->applicationName().isEmpty() && qApp->applicationVersion().isEmpty()) {
        d->clientName = u"%1 %2"_s.arg(u"Based on QXmpp", QXmppVersion());
    } else {
        d->clientName = u"%1 %2"_s.arg(qApp->applicationName(), qApp->applicationVersion());
    }
}

QXmppDiscoveryManager::~QXmppDiscoveryManager() = default;

///
/// Requests information from the specified XMPP entity.
///
/// \param jid  The target entity's JID.
/// \param node The target node (optional).
///
QString QXmppDiscoveryManager::requestInfo(const QString &jid, const QString &node)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::InfoQuery);
    request.setTo(jid);
    if (!node.isEmpty()) {
        request.setQueryNode(node);
    }
    if (client()->sendPacket(request)) {
        return request.id();
    } else {
        return QString();
    }
}

///
/// Requests items from the specified XMPP entity.
///
/// \param jid  The target entity's JID.
/// \param node The target node (optional).
///
QString QXmppDiscoveryManager::requestItems(const QString &jid, const QString &node)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::ItemsQuery);
    request.setTo(jid);
    if (!node.isEmpty()) {
        request.setQueryNode(node);
    }
    if (client()->sendPacket(request)) {
        return request.id();
    } else {
        return QString();
    }
}

///
/// Requests information from the specified XMPP entity.
///
/// \param jid  The target entity's JID.
/// \param node The target node (optional).
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppDiscoveryManager::InfoResult> QXmppDiscoveryManager::requestDiscoInfo(const QString &jid, const QString &node)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::InfoQuery);
    request.setTo(jid);
    if (!node.isEmpty()) {
        request.setQueryNode(node);
    }

    return chainIq<InfoResult>(client()->sendIq(std::move(request)), this);
}

///
/// Requests items from the specified XMPP entity.
///
/// \param jid  The target entity's JID.
/// \param node The target node (optional).
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppDiscoveryManager::ItemsResult> QXmppDiscoveryManager::requestDiscoItems(const QString &jid, const QString &node)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::ItemsQuery);
    request.setTo(jid);
    if (!node.isEmpty()) {
        request.setQueryNode(node);
    }

    return chainIq(client()->sendIq(std::move(request)), this, [](QXmppDiscoveryIq &&iq) -> ItemsResult {
        return iq.items();
    });
}

///
/// Returns the client's full capabilities.
///
QXmppDiscoveryIq QXmppDiscoveryManager::capabilities()
{
    QXmppDiscoveryIq iq;
    iq.setType(QXmppIq::Result);
    iq.setQueryType(QXmppDiscoveryIq::InfoQuery);

    // features
    QStringList features;
    // add base features of the client
    features << QXmppClientPrivate::discoveryFeatures();

    // add features of all registered extensions
    const auto extensions = client()->extensions();
    for (auto *extension : extensions) {
        if (extension) {
            features << extension->discoveryFeatures();
        }
    }

    iq.setFeatures(features);

    // identities
    QList<QXmppDiscoveryIq::Identity> identities;

    QXmppDiscoveryIq::Identity identity;
    identity.setCategory(clientCategory());
    identity.setType(clientType());
    identity.setName(clientName());
    identities << identity;

    for (auto *extension : client()->extensions()) {
        if (extension) {
            identities << extension->discoveryIdentities();
        }
    }

    iq.setIdentities(identities);

    // extended information
    if (!d->clientInfoForm.isNull()) {
        iq.setForm(d->clientInfoForm);
    }

    return iq;
}

///
/// Sets the capabilities node of the local XMPP client.
///
/// \param node
///
void QXmppDiscoveryManager::setClientCapabilitiesNode(const QString &node)
{
    d->clientCapabilitiesNode = node;
}

///
/// Sets the category of the local XMPP client.
///
/// You can find a list of valid categories at:
/// http://xmpp.org/registrar/disco-categories.html
///
/// \param category
///
void QXmppDiscoveryManager::setClientCategory(const QString &category)
{
    d->clientCategory = category;
}

///
/// Sets the type of the local XMPP client.
///
/// You can find a list of valid types at:
/// http://xmpp.org/registrar/disco-categories.html
///
void QXmppDiscoveryManager::setClientType(const QString &type)
{
    d->clientType = type;
}

/// Sets the name of the local XMPP client.
void QXmppDiscoveryManager::setClientName(const QString &name)
{
    d->clientName = name;
}

///
/// Returns the capabilities node of the local XMPP client.
///
/// By default this is "https://github.com/qxmpp-project/qxmpp".
///
QString QXmppDiscoveryManager::clientCapabilitiesNode() const
{
    return d->clientCapabilitiesNode;
}

///
/// Returns the category of the local XMPP client.
///
/// By default this is "client".
///
QString QXmppDiscoveryManager::clientCategory() const
{
    return d->clientCategory;
}

///
/// Returns the type of the local XMPP client.
///
/// With Qt builds for Android, Blackberry, iOS or Windows Phone this is set to
/// "phone", otherwise it defaults to "pc".
///
QString QXmppDiscoveryManager::clientType() const
{
    return d->clientType;
}

///
/// Returns the name of the local XMPP client.
///
/// By default this is "Based on QXmpp x.y.z".
///
QString QXmppDiscoveryManager::clientName() const
{
    return d->clientName;
}

///
/// Returns the client's extended information form, as defined
/// by \xep{0128, Service Discovery Extensions}.
///
QXmppDataForm QXmppDiscoveryManager::clientInfoForm() const
{
    return d->clientInfoForm;
}

///
/// Sets the client's extended information form, as defined
/// by \xep{0128, Service Discovery Extensions}.
///
void QXmppDiscoveryManager::setClientInfoForm(const QXmppDataForm &form)
{
    d->clientInfoForm = form;
}

/// \cond
QStringList QXmppDiscoveryManager::discoveryFeatures() const
{
    return { ns_disco_info.toString() };
}

bool QXmppDiscoveryManager::handleStanza(const QDomElement &element)
{
    if (QXmpp::handleIqRequests<QXmppDiscoveryIq>(element, client(), this)) {
        return true;
    }

    if (element.tagName() == u"iq" && QXmppDiscoveryIq::isDiscoveryIq(element)) {
        QXmppDiscoveryIq receivedIq;
        receivedIq.parse(element);

        switch (receivedIq.type()) {
        case QXmppIq::Get:
            break;
        case QXmppIq::Result:
        case QXmppIq::Error:
            // handle all replies
            if (receivedIq.queryType() == QXmppDiscoveryIq::InfoQuery) {
                Q_EMIT infoReceived(receivedIq);
            } else if (receivedIq.queryType() == QXmppDiscoveryIq::ItemsQuery) {
                Q_EMIT itemsReceived(receivedIq);
            }
            return true;

        case QXmppIq::Set:
            // let other manager handle "set" IQs
            return false;
        }
    }
    return false;
}

std::variant<QXmppDiscoveryIq, QXmppStanza::Error> QXmppDiscoveryManager::handleIq(QXmppDiscoveryIq &&iq)
{
    using Error = QXmppStanza::Error;

    if (!iq.queryNode().isEmpty() && !iq.queryNode().startsWith(d->clientCapabilitiesNode)) {
        return Error(Error::Cancel, Error::ItemNotFound, u"Unknown node."_s);
    }

    switch (iq.queryType()) {
    case QXmppDiscoveryIq::InfoQuery: {
        // respond to info queries for the client itself
        QXmppDiscoveryIq features = capabilities();
        features.setQueryNode(iq.queryNode());
        return features;
    }
    case QXmppDiscoveryIq::ItemsQuery: {
        QXmppDiscoveryIq reply;
        reply.setQueryType(QXmppDiscoveryIq::ItemsQuery);
        return reply;
    }
    }
    Q_UNREACHABLE();
}
/// \endcond
