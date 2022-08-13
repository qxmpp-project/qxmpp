// SPDX-FileCopyrightText: 2016 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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
/// \warning This manager does not decrypt e2ee messages. You can use QXmppCarbonManagerV2.
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
