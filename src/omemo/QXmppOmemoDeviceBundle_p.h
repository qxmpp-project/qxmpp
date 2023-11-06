// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppGlobal.h"

#include <QHash>

class QDomElement;
class QXmlStreamWriter;

class QXMPP_AUTOTEST_EXPORT QXmppOmemoDeviceBundle
{
public:
    QByteArray publicIdentityKey() const;
    void setPublicIdentityKey(const QByteArray &key);

    QByteArray signedPublicPreKey() const;
    void setSignedPublicPreKey(const QByteArray &key);

    uint32_t signedPublicPreKeyId() const;
    void setSignedPublicPreKeyId(uint32_t id);

    QByteArray signedPublicPreKeySignature() const;
    void setSignedPublicPreKeySignature(const QByteArray &signature);

    QHash<uint32_t, QByteArray> publicPreKeys() const;
    void addPublicPreKey(uint32_t id, const QByteArray &key);
    void removePublicPreKey(uint32_t id);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoDeviceBundle(const QDomElement &element);

private:
    QByteArray m_publicIdentityKey;
    QByteArray m_signedPublicPreKey;
    uint32_t m_signedPublicPreKeyId = 0;
    QByteArray m_signedPublicPreKeySignature;
    QHash<uint32_t, QByteArray> m_publicPreKeys;
};

Q_DECLARE_TYPEINFO(QXmppOmemoDeviceBundle, Q_MOVABLE_TYPE);
