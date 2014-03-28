/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Olivier Goffart <ogoffart@woboq.com>
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

#ifndef QXMPPRESULTSET_H
#define QXMPPRESULTSET_H

#include <QDateTime>

#include "QXmppStanza.h"

/// \brief The QXmppResultSetQuery class represents a set element in a query
/// as defined by XEP-0059: Result Set Management.

class QXMPP_EXPORT QXmppResultSetQuery
{
public:
    QXmppResultSetQuery();

    int max() const;
    void setMax(int max);

    int index() const;
    void setIndex(int index);

    QString before() const;
    void setBefore(const QString &before );

    QString after() const;
    void setAfter(const QString &after );

    bool isNull() const;

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    int m_index;
    int m_max;
    QString m_after;
    QString m_before;
};

/// \brief The QXmppResultSetReply class represents a set element in a reply
/// as defined by XEP-0059: Result Set Management.

class QXMPP_EXPORT QXmppResultSetReply
{
public:
    QXmppResultSetReply();

    QString first() const;
    void setFirst(const QString &first );

    QString last() const;
    void setLast(const QString &last );

    int count() const;
    void setCount(int count);

    int index() const;
    void setIndex(int index);

    bool isNull() const;

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    int m_count;
    int m_index;
    QString m_first;
    QString m_last;
};

#endif // QXMPPRESULTSET_H
