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

#ifndef QXMPPVERSIONMANAGER_H
#define QXMPPVERSIONMANAGER_H

#include "QXmppClientExtension.h"

class QXmppOutgoingClient;
class QXmppVersionIq;

/// \brief The QXmppVersionManager class makes it possible to request for
/// the software version of an entity as defined by XEP-0092: Software Version.
///
/// \ingroup Managers

class QXmppVersionManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    void requestVersion(const QString& jid);

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(QXmppStream *stream, const QDomElement &element);
    /// \endcond

signals:
    void versionReceived(const QXmppVersionIq&);
};

#endif // QXMPPVERSIONMANAGER_H
