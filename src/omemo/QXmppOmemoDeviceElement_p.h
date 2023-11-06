// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppGlobal.h"

class QDomElement;
class QXmlStreamWriter;

class QXMPP_AUTOTEST_EXPORT QXmppOmemoDeviceElement
{
public:
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
    uint32_t m_id = 0;
    QString m_label;
};

Q_DECLARE_TYPEINFO(QXmppOmemoDeviceElement, Q_MOVABLE_TYPE);
