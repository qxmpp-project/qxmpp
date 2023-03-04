// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMODEVICELIST_H
#define QXMPPOMEMODEVICELIST_H

#include "QXmppGlobal.h"
#include "QXmppOmemoDeviceElement_p.h"

#include "QList"

class QDomElement;
class QXmlStreamWriter;

class QXMPP_AUTOTEST_EXPORT QXmppOmemoDeviceList : public QList<QXmppOmemoDeviceElement>
{
public:
    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoDeviceList(const QDomElement &element);
};

Q_DECLARE_TYPEINFO(QXmppOmemoDeviceList, Q_MOVABLE_TYPE);

#endif  // QXMPPOMEMODEVICELIST_H
