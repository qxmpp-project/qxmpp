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

#include "QXmppClientExtension.h"

class QXmppClientExtensionPrivate
{
public:
    QXmppClient *client;
};

/// Constructs a QXmppClient extension.
///

QXmppClientExtension::QXmppClientExtension()
    : d(new QXmppClientExtensionPrivate)
{
    d->client = 0;
}

/// Destroys a QXmppClient extension.
///

QXmppClientExtension::~QXmppClientExtension()
{
    delete d;
}

/// Returns the client which loaded this extension.
///

QXmppClient *QXmppClientExtension::client()
{
    return d->client;
}

/// Sets the client which loaded this extension.
///
/// \param client

void QXmppClientExtension::setClient(QXmppClient *client)
{
    d->client = client;
}

/// Logs a debugging message.
///
/// \param message

void QXmppClientExtension::debug(const QString &message)
{
    emit logMessage(QXmppLogger::DebugMessage, message);
}

/// Logs an informational message.
///
/// \param message

void QXmppClientExtension::info(const QString &message)
{
    emit logMessage(QXmppLogger::InformationMessage, message);
}

/// Logs a warning message.
///
/// \param message

void QXmppClientExtension::warning(const QString &message)
{
    emit logMessage(QXmppLogger::WarningMessage, message);
}

