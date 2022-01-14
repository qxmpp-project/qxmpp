// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMODEVICEBUNDLE_H
#define QXMPPOMEMODEVICEBUNDLE_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QXmppOmemoDeviceBundlePrivate;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppOmemoDeviceBundle
{
public:
    QXmppOmemoDeviceBundle();
    QXmppOmemoDeviceBundle(const QXmppOmemoDeviceBundle &other);
    ~QXmppOmemoDeviceBundle();

    QXmppOmemoDeviceBundle &operator=(const QXmppOmemoDeviceBundle &other);

    QByteArray publicIdentityKey() const;
    void setPublicIdentityKey(const QByteArray &key);

    QByteArray signedPublicPreKey() const;
    void setSignedPublicPreKey(const QByteArray &key);

    uint32_t signedPublicPreKeyId() const;
    void setSignedPublicPreKeyId(uint32_t id);

    QByteArray signedPublicPreKeySignature() const;
    void setSignedPublicPreKeySignature(const QByteArray &signature);

    QMap<uint32_t, QByteArray> publicPreKeys() const;
    void setPublicPreKeys(const QMap<uint32_t, QByteArray> &keys);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoDeviceBundle(const QDomElement &element);

private:
    QSharedDataPointer<QXmppOmemoDeviceBundlePrivate> d;
};

Q_DECLARE_TYPEINFO(QXmppOmemoDeviceBundle, Q_MOVABLE_TYPE);

#endif  // QXMPPOMEMODEVICEBUNDLE_H
