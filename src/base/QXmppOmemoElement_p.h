// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMOELEMENT_H
#define QXMPPOMEMOELEMENT_H

#include "QXmppGlobal.h"
#include "QXmppOmemoEnvelope_p.h"

#include <optional>

#include <QMultiMap>

class QDomElement;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppOmemoElement
{
public:
    uint32_t senderDeviceId() const;
    void setSenderDeviceId(uint32_t id);

    QByteArray payload() const;
    void setPayload(const QByteArray &payload);

    std::optional<QXmppOmemoEnvelope> searchEnvelope(const QString &recipientJid, uint32_t recipientDeviceId) const;
    void addEnvelope(const QString &recipientJid, const QXmppOmemoEnvelope &envelope);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoElement(const QDomElement &element);

private:
    uint32_t m_senderDeviceId = 0;
    QByteArray m_payload;
    QMultiMap<QString, QXmppOmemoEnvelope> m_envelopes;
};

Q_DECLARE_TYPEINFO(QXmppOmemoElement, Q_MOVABLE_TYPE);

#endif  // QXMPPOMEMOELEMENT_H
