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

class QXmppClientExtension : public QObject
{
    Q_OBJECT

public:
    QXmppClientExtension();
    virtual ~QXmppClientExtension();

    virtual QStringList discoveryFeatures() const;
    virtual bool handleStanza(const QDomElement &stanza) = 0;

signals:
    /// This signal is emitted to send logging messages.
    void logMessage(QXmppLogger::MessageType type, const QString &msg);

protected:
    QXmppClient *client();

    // Logging helpers
    void debug(const QString&);
    void info(const QString&);
    void warning(const QString&);

private:
    void setClient(QXmppClient *client);
    QXmppClientExtensionPrivate * const d;

    friend class QXmppClient;
};

#endif
