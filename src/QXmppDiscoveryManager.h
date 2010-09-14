/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

class QXmppOutgoingClient;
class QXmppDiscoveryIq;

/// \ingroup Managers

class QXmppDiscoveryManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    void requestInformation(const QString& jid, const QString& node = "");

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(QXmppStream *stream, const QDomElement &element);
    /// \endcond

signals:
    void informationReceived(const QXmppDiscoveryIq&);

private:
    QXmppDiscoveryIq capabilities();
};

#endif // QXMPPDISCOVERYMANAGER_H
