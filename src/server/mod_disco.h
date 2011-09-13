/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#ifndef QXMPP_SERVER_DISCO_H
#define QXMPP_SERVER_DISCO_H

#include <QStringList>

#include "QXmppServerExtension.h"

/// \brief QXmppServer extension for XEP-0030: Service Discovery.
///

class QXmppServerDiscovery : public QXmppServerExtension
{
    Q_OBJECT
    Q_CLASSINFO("ExtensionName", "disco");
    Q_PROPERTY(QStringList items READ items WRITE setItems);

public:
    QStringList items() const;
    void setItems(const QStringList &items);

    /// \cond
    QStringList discoveryFeatures() const;
    QStringList discoveryItems() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

private:
    QStringList m_discoveryItems;
};

#endif
