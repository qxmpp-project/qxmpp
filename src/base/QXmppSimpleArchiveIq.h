/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Author:
 *  James Turner (james.turner@kdab.com)
 *  Truphone Labs (labs@truphone.com)
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#ifndef QXMPPSIMPLEARCHIVEIQ_H
#define QXMPPSIMPLEARCHIVEIQ_H

#include "QXmppIq.h"
#include "QXmppResultSet.h"

#include <QDateTime>

class QXmlStreamWriter;
class QDomElement;

/// \brief Represents an archive message query as defined by 
/// XEP-0313: Message Archive Management
///
/// It is used to get messages via a result set
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppSimpleArchiveQueryIq : public QXmppIq
{
public:
    QXmppSimpleArchiveQueryIq();
    
    QXmppResultSetReply resultSetReply() const;
    void setResultSetReply(const QXmppResultSetReply &rsm);

    /// \cond
    static bool isSimpleArchiveQueryIq(const QDomElement &element);

    QString queryId() const;
    void setQueryId(const QString& queryId);

    QDateTime start() const;
    void setStart(const QDateTime &start);

    QDateTime end() const;
    void setEnd(const QDateTime &end );
    
    QString with() const;
    void setWith(const QString &with);

    QXmppResultSetQuery resultSetQuery() const;
    void setResultSetQuery(const QXmppResultSetQuery &rsm);
protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_with;
    QDateTime m_start;
    QDateTime m_end;
    QString m_queryId;
    QXmppResultSetQuery m_rsmQuery;
    QXmppResultSetReply m_rsmReply;
};

#endif // QXMPPSIMPLEARCHIVEIQ_H
