// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QXmlStreamWriter;
class QXmppTrustMessageElementPrivate;
class QXmppTrustMessageKeyOwner;

class QXMPP_EXPORT QXmppTrustMessageElement
{
public:
    QXmppTrustMessageElement();
    QXmppTrustMessageElement(const QXmppTrustMessageElement &other);
    QXmppTrustMessageElement(QXmppTrustMessageElement &&);
    ~QXmppTrustMessageElement();

    QXmppTrustMessageElement &operator=(const QXmppTrustMessageElement &other);
    QXmppTrustMessageElement &operator=(QXmppTrustMessageElement &&);

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
