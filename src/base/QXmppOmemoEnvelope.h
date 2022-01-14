// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMOENVELOPE_H
#define QXMPPOMEMOENVELOPE_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QXmppOmemoEnvelopePrivate;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppOmemoEnvelope
{
public:
    QXmppOmemoEnvelope();
    QXmppOmemoEnvelope(const QXmppOmemoEnvelope &other);
    ~QXmppOmemoEnvelope();

    QXmppOmemoEnvelope &operator=(const QXmppOmemoEnvelope &other);

    uint32_t recipientDeviceId() const;
    void setRecipientDeviceId(uint32_t id);

    bool isUsedForKeyExchange() const;
    void setIsUsedForKeyExchange(bool isUsed);

    QByteArray data() const;
    void setData(const QByteArray &data);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoEnvelope(const QDomElement &element);

private:
    QSharedDataPointer<QXmppOmemoEnvelopePrivate> d;
};

Q_DECLARE_TYPEINFO(QXmppOmemoEnvelope, Q_MOVABLE_TYPE);

#endif  // QXMPPOMEMOENVELOPE_H
