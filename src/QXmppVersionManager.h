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

#include <QObject>

class QXmppOutgoingClient;
class QXmppVersionIq;

/// \brief The QXmppVersionManager class makes it possible to request for
/// the software version of an entity as defined by XEP-0092: Software Version.
///
/// \ingroup Managers

class QXmppVersionManager : public QObject
{
    Q_OBJECT

public:
    QXmppVersionManager(QXmppOutgoingClient* stream, QObject *parent = 0);

signals:
    void versionReceived(const QXmppVersionIq&);

private slots:
    void versionIqReceived(const QXmppVersionIq&);

private:
    // reference to the xmpp stream (no ownership)
    QXmppOutgoingClient* m_stream;
};

#endif // QXMPPVERSIONMANAGER_H
