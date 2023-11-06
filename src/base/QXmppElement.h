// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppGlobal.h"

#include <QMap>
#include <QStringList>
#include <QXmlStreamWriter>

class QDomElement;
class QXmppElement;
class QXmppElementPrivate;

using QXmppElementList = QList<QXmppElement>;

class QXMPP_EXPORT QXmppElement
{
public:
    QXmppElement();
    QXmppElement(const QXmppElement &other);
    QXmppElement(const QDomElement &element);
    ~QXmppElement();

    QDomElement sourceDomElement() const;

    QStringList attributeNames() const;

    QString attribute(const QString &name) const;
    void setAttribute(const QString &name, const QString &value);

    void appendChild(const QXmppElement &child);
    QXmppElement firstChildElement(const QString &name = QString()) const;
    QXmppElement nextSiblingElement(const QString &name = QString()) const;
    void removeChild(const QXmppElement &child);

    QString tagName() const;
    void setTagName(const QString &type);

    QString value() const;
    void setValue(const QString &text);

    bool isNull() const;
    void toXml(QXmlStreamWriter *writer) const;

    QXmppElement &operator=(const QXmppElement &other);

private:
    QXmppElement(QXmppElementPrivate *other);
    // ### QXmpp2: Use an std::shared_ptr if possible?
    QXmppElementPrivate *d;
};
