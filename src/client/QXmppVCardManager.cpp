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
    : d(new QXmppVCardManagerPrivate)
{
    d->isClientVCardReceived = false;
}

QXmppVCardManager::~QXmppVCardManager()
{
    delete d;
}

/// This function requests the server for vCard of the specified jid.
/// Once received the signal vCardReceived() is emitted.
///
/// \param jid Jid of the specific entry in the roster
///
QString QXmppVCardManager::requestVCard(const QString &jid)
{
    QXmppVCardIq request(jid);
    if (client()->sendPacket(request))
        return request.id();
    else
        return QString();
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

// helper for std::visit
template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template<typename IqType, typename Handler>
bool handleIqType(Handler handler, const QDomElement &element, const QString &tagName, const QString &xmlNamespace)
{
    if (!IqType::checkIqType(tagName, xmlNamespace)) {
        return false;
    }

    IqType iq;
    iq.parse(element);
    if constexpr (std::is_invocable<Handler, IqType &&>()) {
        handler(std::move(iq));
    } else {
        handler->handleIq(std::move(iq));
    }
    return true;
}

template<typename... IqTypes, typename Handler>
bool visitIq(Handler handler, const QDomElement &element)
{
    auto tagName = element.firstChildElement().tagName();
    auto xmlns = element.firstChildElement().namespaceURI();

    return (handleIqType<IqTypes>(handler, element, tagName, xmlns) || ...);
}

#include "QXmppVersionIq.h"

bool QXmppVCardManager::handleStanza(const QDomElement &element)
{
    return visitIq<QXmppVCardIq, QXmppVersionIq>(this, element);
    return visitIq<QXmppVCardIq>(
                [](QXmppVCardIq) {
                    return;
                }, element);
    return visitIq<QXmppVCardIq, QXmppVersionIq>(
                [](std::variant<QXmppVCardIq, QXmppVersionIq>) {
                    return;
                }, element);
    return visitIq<QXmppVCardIq, QXmppVersionIq>(
                overloaded {
                    [](QXmppVCardIq) {
                        return;
                    },
                    [](QXmppVersionIq) {
                        return;
                    }
                }, element);


    if (element.tagName() == "iq" && QXmppVCardIq::isVCard(element)) {
        QXmppVCardIq vCardIq;
        vCardIq.parse(element);

        if (vCardIq.from().isEmpty() || vCardIq.from() == client()->configuration().jidBare()) {
            d->clientVCard = vCardIq;
            d->isClientVCardReceived = true;
            emit clientVCardReceived();
        }

        emit vCardReceived(vCardIq);

        return true;
    }

    return false;
}

bool QXmppVCardManager::handleIq(QXmppVCardIq)
{
    return false;
}

bool QXmppVCardManager::handleIq(QXmppVersionIq)
{
    return false;
}
/// \endcond
