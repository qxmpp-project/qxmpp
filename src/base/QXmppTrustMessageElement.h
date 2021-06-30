/*
 * Copyright (C) 2008-2021 The QXmpp developers
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

#ifndef QXMPPTRUSTMESSAGEELEMENT_H
#define QXMPPTRUSTMESSAGEELEMENT_H

#include "QXmppGlobal.h"

#include <QDomElement>
#include <QList>
#include <QSharedDataPointer>
#include <QXmlStreamWriter>

class QXmppTrustMessageElementPrivate;
class QXmppTrustMessageKeyOwner;

class QXMPP_EXPORT QXmppTrustMessageElement
{
public:
    QXmppTrustMessageElement();
    QXmppTrustMessageElement(const QXmppTrustMessageElement &other);
    ~QXmppTrustMessageElement();

    QXmppTrustMessageElement &operator=(const QXmppTrustMessageElement &other);

    QString usage() const;
    void setUsage(const QString &usage);

    QString encryption() const;
    void setEncryption(const QString &encryption);

    QList<QXmppTrustMessageKeyOwner> keyOwners() const;
    void setKeyOwners(const QList<QXmppTrustMessageKeyOwner> &keyOwners);
    void addKeyOwner(const QXmppTrustMessageKeyOwner &keyOwner);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isTrustMessageElement(const QDomElement &element);

private:
    QSharedDataPointer<QXmppTrustMessageElementPrivate> d;
};

Q_DECLARE_TYPEINFO(QXmppTrustMessageElement, Q_MOVABLE_TYPE);

#endif  // QXMPPTRUSTMESSAGEELEMENT_H
