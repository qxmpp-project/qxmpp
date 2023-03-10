// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPDISCOVERYMANAGER_H
#define QXMPPDISCOVERYMANAGER_H

#include "QXmppClientExtension.h"

#include <variant>

template<typename T>
class QXmppTask;
class QXmppDataForm;
class QXmppDiscoveryIq;
class QXmppDiscoveryManagerPrivate;
struct QXmppError;

/// \brief The QXmppDiscoveryManager class makes it possible to discover information
/// about other entities as defined by \xep{0030}: Service Discovery.
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppDiscoveryManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppDiscoveryManager();
    ~QXmppDiscoveryManager() override;

    QXmppDiscoveryIq capabilities();

    QString requestInfo(const QString &jid, const QString &node = QString());
    QString requestItems(const QString &jid, const QString &node = QString());

    using InfoResult = std::variant<QXmppDiscoveryIq, QXmppError>;
    using ItemsResult = std::variant<QList<QXmppDiscoveryIq::Item>, QXmppError>;
    QXmppTask<InfoResult> requestDiscoInfo(const QString &jid, const QString &node = {});
    QXmppTask<ItemsResult> requestDiscoItems(const QString &jid, const QString &node = {});

    QString clientCapabilitiesNode() const;
    void setClientCapabilitiesNode(const QString &);

    // http://xmpp.org/registrar/disco-categories.html#client
    QString clientCategory() const;
    void setClientCategory(const QString &);

    void setClientName(const QString &);
    QString clientName() const;

    QString clientType() const;
    void setClientType(const QString &);

    QXmppDataForm clientInfoForm() const;
    void setClientInfoForm(const QXmppDataForm &form);

    /// \cond
    QStringList discoveryFeatures() const override;
    bool handleStanza(const QDomElement &element) override;
    std::variant<QXmppDiscoveryIq, QXmppStanza::Error> handleIq(QXmppDiscoveryIq &&iq);
    /// \endcond

Q_SIGNALS:
    /// This signal is emitted when an information response is received.
    void infoReceived(const QXmppDiscoveryIq &);

    /// This signal is emitted when an items response is received.
    void itemsReceived(const QXmppDiscoveryIq &);

private:
    const std::unique_ptr<QXmppDiscoveryManagerPrivate> d;
};

#endif  // QXMPPDISCOVERYMANAGER_H
