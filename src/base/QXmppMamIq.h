/*
 * Copyright (C) 2016-2017 The QXmpp developers
 *
 * Author:
 *  Niels Ole Salscheider
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

#ifndef QXMPPMAMIQ_H
#define QXMPPMAMIQ_H

#include "QXmppIq.h"
#include "QXmppDataForm.h"
#include "QXmppResultSet.h"

/// \brief The QXmppMamQueryIq class represents the query IQ for
/// XEP-0313: Message Archive Management.
class QXmppMamQueryIq : public QXmppIq
{
public:
    QXmppMamQueryIq();

    QXmppDataForm form() const;
    void setForm(const QXmppDataForm &form);
    QXmppResultSetQuery resultSetQuery() const;
    void setResultSetQuery(const QXmppResultSetQuery &resultSetQuery);
    QString node() const;
    void setNode(const QString &node);
    QString queryId() const;
    void setQueryId(const QString &id);

    static bool isMamQueryIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QXmppDataForm m_form;
    QXmppResultSetQuery m_resultSetQuery;
    QString m_node;
    QString m_queryId;
};

/// \brief The QXmppMamQueryIq class represents the result IQ for
/// XEP-0313: Message Archive Management.
class QXmppMamResultIq : public QXmppIq
{
public:
    QXmppMamResultIq();

    QXmppResultSetReply resultSetReply() const;
    void setResultSetReply(const QXmppResultSetReply &resultSetReply);
    bool complete() const;
    void setComplete(bool complete);

    static bool isMamResultIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QXmppResultSetReply m_resultSetReply;
    bool m_complete;
};

#endif
