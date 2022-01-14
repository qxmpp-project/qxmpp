// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMOELEMENT_H
#define QXMPPOMEMOELEMENT_H

#include "QXmppGlobal.h"

#include <optional>

#include <QSharedDataPointer>

class QDomElement;
class QXmppOmemoElementPrivate;
class QXmppOmemoEnvelope;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppOmemoElement
{
public:
    QXmppOmemoElement();
    QXmppOmemoElement(const QXmppOmemoElement &other);
    ~QXmppOmemoElement();

    QXmppOmemoElement &operator=(const QXmppOmemoElement &other);

    uint32_t senderDeviceId() const;
    void setSenderDeviceId(uint32_t id);

    QByteArray payload() const;
    void setPayload(const QByteArray &payload);

    std::optional<QXmppOmemoEnvelope> searchEnvelope(const QString &recipientJid, uint32_t recipientDeviceId) const;
    void addEnvelope(const QString &recipientJid, QXmppOmemoEnvelope &envelope);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoElement(const QDomElement &element);

private:
    QSharedDataPointer<QXmppOmemoElementPrivate> d;
};

Q_DECLARE_TYPEINFO(QXmppOmemoElement, Q_MOVABLE_TYPE);

#endif  // QXMPPOMEMOELEMENT_H
