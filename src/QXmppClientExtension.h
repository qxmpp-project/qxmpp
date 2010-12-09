/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

#ifndef QXMPPCLIENTEXTENSION_H
#define QXMPPCLIENTEXTENSION_H

#include "QXmppLogger.h"

class QDomElement;
class QStringList;

class QXmppClient;
class QXmppClientExtensionPrivate;
class QXmppStream;

/// \brief The QXmppClientExtension class is the base class for QXmppClient
/// extensions.
///

class QXmppClientExtension : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppClientExtension();
    virtual ~QXmppClientExtension();

    virtual QStringList discoveryFeatures() const;

    /// \brief Handles the incoming XMPP \a stanza.
    ///
    /// Returns true if the stanza was handled and no further processing
    /// should occur, or false otherwise.
    virtual bool handleStanza(const QDomElement &stanza) = 0;

protected:
    QXmppClient *client();

private:
    void setClient(QXmppClient *client);
    QXmppClientExtensionPrivate * const d;

    friend class QXmppClient;
};

#endif
