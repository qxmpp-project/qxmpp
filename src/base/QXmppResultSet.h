// SPDX-FileCopyrightText: 2012 Oliver Goffart <ogoffart@woboq.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppStanza.h"

#include <QDateTime>

/// \brief The QXmppResultSetQuery class represents a set element in a query
/// as defined by \xep{0059}: Result Set Management.

class QXMPP_EXPORT QXmppResultSetQuery
{
public:
    QXmppResultSetQuery();

    int max() const;
    void setMax(int max);

    int index() const;
    void setIndex(int index);

    QString before() const;
    void setBefore(const QString &before);

    QString after() const;
    void setAfter(const QString &after);

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
/// as defined by \xep{0059}: Result Set Management.

class QXMPP_EXPORT QXmppResultSetReply
{
public:
    QXmppResultSetReply();

    QString first() const;
    void setFirst(const QString &first);

    QString last() const;
    void setLast(const QString &last);

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
