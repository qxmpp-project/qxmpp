/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

class QXmppDiscoveryIq;

/// \brief The QXmppDiscoveryManager class makes it possible to discover information
/// about other entities as defined by XEP-0030: Service Discovery.
///
/// \ingroup Managers

class QXmppDiscoveryManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppDiscoveryManager();

    QString requestInfo(const QString& jid, const QString& node = "");
    QString requestItems(const QString& jid, const QString& node = "");

    QString clientCapabilitiesNode() const;
    void setClientCapabilitiesNode(const QString&);

    // http://xmpp.org/registrar/disco-categories.html#client
    QString clientCategory() const;
    void setClientCategory(const QString&);

    void setClientName(const QString&);
    QString clientName() const;

    QString clientType() const;
    void setClientType(const QString&);

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement &element);
    QXmppDiscoveryIq capabilities();
    /// \endcond

signals:
    /// This signal is emitted when an information response is received.
    void infoReceived(const QXmppDiscoveryIq&);

    /// This signal is emitted when an items response is received.
    void itemsReceived(const QXmppDiscoveryIq&);

private:
    QString m_clientCapabilitiesNode;
    QString m_clientCategory;
    QString m_clientType;
    QString m_clientName;
};

#endif // QXMPPDISCOVERYMANAGER_H
