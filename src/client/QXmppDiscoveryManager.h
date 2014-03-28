/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifndef QXMPPDISCOVERYMANAGER_H
#define QXMPPDISCOVERYMANAGER_H

#include "QXmppClientExtension.h"

class QXmppDataForm;
class QXmppDiscoveryIq;
class QXmppDiscoveryManagerPrivate;

/// \brief The QXmppDiscoveryManager class makes it possible to discover information
/// about other entities as defined by XEP-0030: Service Discovery.
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppDiscoveryManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppDiscoveryManager();
    ~QXmppDiscoveryManager();

    QXmppDiscoveryIq capabilities();

    QString requestInfo(const QString& jid, const QString& node = QString());
    QString requestItems(const QString& jid, const QString& node = QString());

    QString clientCapabilitiesNode() const;
    void setClientCapabilitiesNode(const QString&);

    // http://xmpp.org/registrar/disco-categories.html#client
    QString clientCategory() const;
    void setClientCategory(const QString&);

    void setClientName(const QString&);
    QString clientName() const;

    QString clientType() const;
    void setClientType(const QString&);

    QXmppDataForm clientInfoForm() const;
    void setClientInfoForm(const QXmppDataForm &form);

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    /// This signal is emitted when an information response is received.
    void infoReceived(const QXmppDiscoveryIq&);

    /// This signal is emitted when an items response is received.
    void itemsReceived(const QXmppDiscoveryIq&);

private:
    QXmppDiscoveryManagerPrivate *d;
};

#endif // QXMPPDISCOVERYMANAGER_H
