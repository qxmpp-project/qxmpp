// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMODEVICEELEMENT_H
#define QXMPPOMEMODEVICEELEMENT_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QXmppOmemoDeviceElementPrivate;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppOmemoDeviceElement
{
public:
    QXmppOmemoDeviceElement();
    QXmppOmemoDeviceElement(const QXmppOmemoDeviceElement &other);
    ~QXmppOmemoDeviceElement();

    QXmppOmemoDeviceElement &operator=(const QXmppOmemoDeviceElement &other);
    bool operator==(const QXmppOmemoDeviceElement &other) const;

    uint32_t id() const;
    void setId(uint32_t id);

    QString label() const;
    void setLabel(const QString &label);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoDeviceElement(const QDomElement &element);

private:
    QSharedDataPointer<QXmppOmemoDeviceElementPrivate> d;
};

Q_DECLARE_TYPEINFO(QXmppOmemoDeviceElement, Q_MOVABLE_TYPE);

#endif  // QXMPPOMEMODEVICEELEMENT_H
