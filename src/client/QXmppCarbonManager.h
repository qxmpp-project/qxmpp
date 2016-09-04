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


#ifndef QXMPPCARBONMANAGER_H
#define QXMPPCARBONMANAGER_H

#include "QXmppClientExtension.h"

class QXmppMessage;

/// \brief The QXmppCarbonManager class handles message carbons
/// as described in XEP-0280: Message Carbons.
///
/// This class emits signals whenever another resource of the
/// currently connected client account sent or received a message.
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppCarbonManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppCarbonManager();
    ~QXmppCarbonManager();

    bool carbonsEnabled() const;
    void setCarbonsEnabled(bool enabled);

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    /// \brief Emitted when a message was received from someone else
    /// and directed to another resource.
    /// If you connect this signal to the \s QXmppClient::messageReceived
    /// signal, they will appear as normal messages.
    void messageReceived(const QXmppMessage&);

    /// \brief Emitted when another resource sent a message to
    /// someone else
    void messageSent(const QXmppMessage&);

private:
    bool m_carbonsEnabled;
};

#endif // QXMPPCARBONMANAGER_H
