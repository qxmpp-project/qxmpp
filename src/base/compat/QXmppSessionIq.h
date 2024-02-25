// SPDX-FileCopyrightText: 2011 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSESSIONIQ_H
#define QXMPPSESSIONIQ_H

#include "QXmppIq.h"

#if QXMPP_DEPRECATED_SINCE(1, 7)
class QXMPP_EXPORT QXmppSessionIq : public QXmppIq
{
public:
    [[deprecated]] static bool isSessionIq(const QDomElement &element);

private:
    [[deprecated]] void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
};
#endif

#endif  // QXMPPSESSION_H
