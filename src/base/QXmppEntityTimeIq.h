/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    int m_tzo;
    QDateTime m_utc;
};

#endif  //QXMPPENTITYTIMEIQ_H
