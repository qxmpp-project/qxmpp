/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

///
/// \brief The QXmppCarbonManager class handles message carbons
/// as described in \xep{0280}: Message Carbons.
///
/// This class emits signals whenever another resource of the
/// currently connected client account sent or received a message.
///
/// \ingroup Managers
///
/// \since QXmpp 1.0
///
class QXMPP_EXPORT QXmppCarbonManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppCarbonManager();
    ~QXmppCarbonManager() override;

    bool carbonsEnabled() const;
    void setCarbonsEnabled(bool enabled);

    /// \cond
    QStringList discoveryFeatures() const override;
    bool handleStanza(const QDomElement &element) override;
    /// \endcond

Q_SIGNALS:
    void messageReceived(const QXmppMessage &);
    void messageSent(const QXmppMessage &);

private:
    bool m_carbonsEnabled;
};

#endif  // QXMPPCARBONMANAGER_H
