/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Melvin Keskin
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

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
    ~QXmppTrustMessageKeyOwner();

    QXmppTrustMessageKeyOwner &operator=(const QXmppTrustMessageKeyOwner &other);

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
