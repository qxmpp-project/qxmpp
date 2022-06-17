// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPTRUSTMESSAGEKEYOWNER_H
#define QXMPPTRUSTMESSAGEKEYOWNER_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QXmlStreamWriter;
class QXmppTrustMessageKeyOwnerPrivate;

class QXMPP_EXPORT QXmppTrustMessageKeyOwner
{
public:
    QXmppTrustMessageKeyOwner();
    QXmppTrustMessageKeyOwner(const QXmppTrustMessageKeyOwner &other);
    QXmppTrustMessageKeyOwner(QXmppTrustMessageKeyOwner &&);
    ~QXmppTrustMessageKeyOwner();

    QXmppTrustMessageKeyOwner &operator=(const QXmppTrustMessageKeyOwner &other);
    QXmppTrustMessageKeyOwner &operator=(QXmppTrustMessageKeyOwner &&);

    QString jid() const;
    void setJid(const QString &jid);

    QList<QByteArray> trustedKeys() const;
    void setTrustedKeys(const QList<QByteArray> &keyIds);

    QList<QByteArray> distrustedKeys() const;
    void setDistrustedKeys(const QList<QByteArray> &keyIds);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isTrustMessageKeyOwner(const QDomElement &element);

private:
    QSharedDataPointer<QXmppTrustMessageKeyOwnerPrivate> d;
};

Q_DECLARE_TYPEINFO(QXmppTrustMessageKeyOwner, Q_MOVABLE_TYPE);

#endif  // QXMPPTRUSTMESSAGEKEYOWNER_H
