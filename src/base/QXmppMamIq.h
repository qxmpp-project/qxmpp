/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppDataForm.h"
#include "QXmppIq.h"
#include "QXmppResultSet.h"

#include <QSharedDataPointer>

class QXmppMamQueryIqPrivate;
class QXmppMamResultIqPrivate;

///
/// \brief The QXmppMamQueryIq class represents the query IQ for
/// \xep{0313}: Message Archive Management.
///
/// \ingroup Stanzas
///
/// \since QXmpp 1.0
///
class QXmppMamQueryIq : public QXmppIq
{
public:
    QXmppMamQueryIq();
    QXmppMamQueryIq(const QXmppMamQueryIq &);
    ~QXmppMamQueryIq();

    QXmppMamQueryIq &operator=(const QXmppMamQueryIq &);

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
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;

private:
    QSharedDataPointer<QXmppMamQueryIqPrivate> d;
};

///
/// \brief The QXmppMamQueryIq class represents the result IQ for
/// \xep{0313}: Message Archive Management.
///
/// \ingroup Stanzas
///
/// \since QXmpp 1.0
///
class QXmppMamResultIq : public QXmppIq
{
public:
    QXmppMamResultIq();
    QXmppMamResultIq(const QXmppMamResultIq &);
    ~QXmppMamResultIq();

    QXmppMamResultIq &operator=(const QXmppMamResultIq &);

    QXmppResultSetReply resultSetReply() const;
    void setResultSetReply(const QXmppResultSetReply &resultSetReply);
    bool complete() const;
    void setComplete(bool complete);

    static bool isMamResultIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;

private:
    QSharedDataPointer<QXmppMamResultIqPrivate> d;
};

#endif
