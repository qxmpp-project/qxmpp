// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMOIQ_H
#define QXMPPOMEMOIQ_H

#include "QXmppIq.h"
#include "QXmppOmemoElement_p.h"

class QXMPP_AUTOTEST_EXPORT QXmppOmemoIq : public QXmppIq
{
public:
    QXmppOmemoElement omemoElement();
    void setOmemoElement(const QXmppOmemoElement &omemoElement);

    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

    static bool isOmemoIq(const QDomElement &element);

private:
    QXmppOmemoElement m_omemoElement;
};

#endif  // QXMPPOMEMOIQ_H
