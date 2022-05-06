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
    bool handleStanza(const QDomElement &element, const std::optional<QXmppE2eeMetadata> &e2eeMetadata) override;
    /// \endcond

#if QXMPP_DEPRECATED_SINCE(1, 5)
    QT_DEPRECATED_X("Use a client extension with handleMessage(QXmppMessage)")
    Q_SIGNAL void messageReceived(const QXmppMessage &);
    QT_DEPRECATED_X("Use a client extension with handleMessage(QXmppMessage)")
    Q_SIGNAL void messageSent(const QXmppMessage &);
#endif

private:
    bool m_carbonsEnabled;
};

#endif  // QXMPPCARBONMANAGER_H
