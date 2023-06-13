// SPDX-FileCopyrightText: 2023 Matthieu Volat <mazhe@alkumuna.eu>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMUCHISTORY_H
#define QXMPPMUCHISTORY_H

#include "QXmppStanza.h"

#include <QDateTime>

/// \brief The QXmppMucHistory class represents a room history request
///
/// It is used to manage how much history should be requested and received
/// when joining a room.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppMucHistory
{
public:
    QXmppMucHistory();
    bool isNull() const;

    int maxchars() const;
    void setMaxchars(int maxchars);

    int maxstanzas() const;
    void setMaxstanzas(int maxstanzas);

    int seconds() const;
    void setSeconds(int seconds);

    QDateTime since() const;
    void setSince(QDateTime &since);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond
private:
    int m_maxchars;
    int m_maxstanzas;
    int m_seconds;
    QDateTime m_since;
};

#endif
