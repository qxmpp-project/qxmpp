// SPDX-FileCopyrightText: 2016 Niels Ole Salscheider <niels_ole@salscheider-online.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMAMIQ_H
#define QXMPPMAMIQ_H

#include "QXmppDataForm.h"
#include "QXmppIq.h"
#include "QXmppResultSet.h"

#include <QSharedDataPointer>

class QXmppMamQueryIqPrivate;
class QXmppMamResultIqPrivate;

class QXMPP_EXPORT QXmppMamQueryIq : public QXmppIq
{
public:
    QXmppMamQueryIq();
    QXmppMamQueryIq(const QXmppMamQueryIq &);
    QXmppMamQueryIq(QXmppMamQueryIq &&);
    ~QXmppMamQueryIq();

    QXmppMamQueryIq &operator=(const QXmppMamQueryIq &);
    QXmppMamQueryIq &operator=(QXmppMamQueryIq &&);

    QXmppDataForm form() const;
    void setForm(const QXmppDataForm &form);
    QXmppResultSetQuery resultSetQuery() const;
    void setResultSetQuery(const QXmppResultSetQuery &resultSetQuery);
    QString node() const;
    void setNode(const QString &node);
    QString queryId() const;
    void setQueryId(const QString &id);

    /// \cond
    static bool isMamQueryIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppMamQueryIqPrivate> d;
};

class QXMPP_EXPORT QXmppMamResultIq : public QXmppIq
{
public:
    QXmppMamResultIq();
    QXmppMamResultIq(const QXmppMamResultIq &);
    QXmppMamResultIq(QXmppMamResultIq &&);
    ~QXmppMamResultIq();

    QXmppMamResultIq &operator=(const QXmppMamResultIq &);
    QXmppMamResultIq &operator=(QXmppMamResultIq &&);

    QXmppResultSetReply resultSetReply() const;
    void setResultSetReply(const QXmppResultSetReply &resultSetReply);
    bool complete() const;
    void setComplete(bool complete);

    /// \cond
    static bool isMamResultIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppMamResultIqPrivate> d;
};

#endif
