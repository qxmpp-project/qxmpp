// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPENTITYTIMEIQ_H
#define QXMPPENTITYTIMEIQ_H

#include "QXmppIq.h"

#include <QDateTime>

///
/// \brief QXmppEntityTimeIq represents an entity time request/response as
/// defined in \xep{0202}: Entity Time.
///
/// \ingroup Stanzas
///
class QXMPP_EXPORT QXmppEntityTimeIq : public QXmppIq
{
public:
    int tzo() const;
    void setTzo(int tzo);

    QDateTime utc() const;
    void setUtc(const QDateTime &utc);

    static bool isEntityTimeIq(const QDomElement &element);
    /// \cond
    static bool checkIqType(const QString &tagName, const QString &xmlns);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    int m_tzo;
    QDateTime m_utc;
};

#endif  // QXMPPENTITYTIMEIQ_H
