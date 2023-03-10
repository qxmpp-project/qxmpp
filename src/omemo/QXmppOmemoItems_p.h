// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMOITEMS_H
#define QXMPPOMEMOITEMS_H

#include "QXmppOmemoDeviceBundle_p.h"
#include "QXmppOmemoDeviceList_p.h"
#include "QXmppPubSubBaseItem.h"

class QXmppOmemoDeviceBundleItem : public QXmppPubSubBaseItem
{
public:
    QXmppOmemoDeviceBundle deviceBundle() const;
    void setDeviceBundle(const QXmppOmemoDeviceBundle &deviceBundle);

    static bool isItem(const QDomElement &itemElement);

protected:
    void parsePayload(const QDomElement &payloadElement) override;
    void serializePayload(QXmlStreamWriter *writer) const override;

private:
    QXmppOmemoDeviceBundle m_deviceBundle;
};

class QXmppOmemoDeviceListItem : public QXmppPubSubBaseItem
{
public:
    QXmppOmemoDeviceList deviceList() const;
    void setDeviceList(const QXmppOmemoDeviceList &deviceList);

    static bool isItem(const QDomElement &itemElement);

protected:
    void parsePayload(const QDomElement &payloadElement) override;
    void serializePayload(QXmlStreamWriter *writer) const override;

private:
    QXmppOmemoDeviceList m_deviceList;
};

#endif  // QXMPPOMEMOITEMS_H
