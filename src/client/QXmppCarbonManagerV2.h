// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCARBONMANAGERV2_H
#define QXMPPCARBONMANAGERV2_H

#include "QXmppClientExtension.h"

class QXMPP_EXPORT QXmppCarbonManagerV2 : public QXmppClientExtension
{
    Q_OBJECT
public:
    QXmppCarbonManagerV2();
    ~QXmppCarbonManagerV2();

    bool handleStanza(const QDomElement &, const std::optional<QXmppE2eeMetadata> &) override;

protected:
    void onRegistered(QXmppClient *client) override;
    void onUnregistered(QXmppClient *client) override;

private:
    void enableCarbons();

    // placeholder (we may need a d-ptr in the future)
    void *d;
};

#endif  // QXMPPCARBONMANAGERV2_H
