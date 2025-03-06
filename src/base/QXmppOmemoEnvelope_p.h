// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMOENVELOPE_H
#define QXMPPOMEMOENVELOPE_H

#include "QXmppGlobal.h"

class QDomElement;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppOmemoEnvelope
{
public:
    uint32_t recipientDeviceId() const;
    void setRecipientDeviceId(uint32_t id);

    bool isUsedForKeyExchange() const;
    void setUsedForKeyExchange(bool isUsed);

    QByteArray data() const;
    void setData(const QByteArray &data);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoEnvelope(const QDomElement &element);

private:
    uint32_t m_recipientDeviceId = 0;
    bool m_isUsedForKeyExchange = false;
    QByteArray m_data;
};

Q_DECLARE_TYPEINFO(QXmppOmemoEnvelope, Q_MOVABLE_TYPE);

#endif  // QXMPPOMEMOENVELOPE_H
