/*
 * Copyright (C) 2008-2021 The QXmpp developers
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

#include "QXmppMamIq.h"

#include "QXmppConstants_p.h"

#include <QDomElement>

class QXmppMamQueryIqPrivate : public QSharedData
{
public:
    QXmppDataForm form;
    QXmppResultSetQuery resultSetQuery;
    QString node;
    QString queryId;
};

///
/// \class QXmppMamQueryIq
///
/// The QXmppMamQueryIq class represents the query IQ for \xep{0313, Message
/// Archive Management}.
///
/// \ingroup Stanzas
///
/// \since QXmpp 1.0
///

QXmppMamQueryIq::QXmppMamQueryIq()
    : QXmppIq(QXmppIq::Set),
      d(new QXmppMamQueryIqPrivate)
{
}

/// Default copy constructor
QXmppMamQueryIq::QXmppMamQueryIq(const QXmppMamQueryIq &) = default;

QXmppMamQueryIq::~QXmppMamQueryIq() = default;

/// Default assignemnt operator
QXmppMamQueryIq &QXmppMamQueryIq::operator=(const QXmppMamQueryIq &) = default;

///
/// Returns the form that specifies the query.
///
QXmppDataForm QXmppMamQueryIq::form() const
{
    return d->form;
}

///
/// Sets the data form that specifies the query.
///
/// \param form The data form.
///
void QXmppMamQueryIq::setForm(const QXmppDataForm &form)
{
    d->form = form;
}

///
/// Returns the result set query for result set management.
///
QXmppResultSetQuery QXmppMamQueryIq::resultSetQuery() const
{
    return d->resultSetQuery;
}

///
/// Sets the result set query for result set management.
///
/// \param resultSetQuery The result set query.
///
void QXmppMamQueryIq::setResultSetQuery(const QXmppResultSetQuery &resultSetQuery)
{
    d->resultSetQuery = resultSetQuery;
}

///
/// Returns the node to query.
///
QString QXmppMamQueryIq::node() const
{
    return d->node;
}

///
/// Sets the node to query.
///
/// \param node The node to query.
///
void QXmppMamQueryIq::setNode(const QString &node)
{
    d->node = node;
}

///
/// Returns the queryid that will be included in the results.
///
QString QXmppMamQueryIq::queryId() const
{
    return d->queryId;
}

///
/// Sets the queryid that will be included in the results.
///
/// \param id The query id.
///
void QXmppMamQueryIq::setQueryId(const QString &id)
{
    d->queryId = id;
}

/// \cond
bool QXmppMamQueryIq::isMamQueryIq(const QDomElement &element)
{
    if (element.tagName() == QStringLiteral("iq")) {
        QDomElement queryElement = element.firstChildElement(QStringLiteral("query"));
        if (!queryElement.isNull() && queryElement.namespaceURI() == ns_mam) {
            return true;
        }
    }
    return false;
}

void QXmppMamQueryIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(QStringLiteral("query"));
    d->node = queryElement.attribute(QStringLiteral("node"));
    d->queryId = queryElement.attribute(QStringLiteral("queryId"));
    QDomElement resultSetElement = queryElement.firstChildElement(QStringLiteral("set"));
    if (!resultSetElement.isNull()) {
        d->resultSetQuery.parse(resultSetElement);
    }
    QDomElement formElement = queryElement.firstChildElement(QStringLiteral("x"));
    if (!formElement.isNull()) {
        d->form.parse(formElement);
    }
}

void QXmppMamQueryIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("query"));
    writer->writeDefaultNamespace(ns_mam);
    if (!d->node.isEmpty()) {
        writer->writeAttribute(QStringLiteral("node"), d->node);
    }
    if (!d->queryId.isEmpty()) {
        writer->writeAttribute(QStringLiteral("queryid"), d->queryId);
    }
    d->form.toXml(writer);
    d->resultSetQuery.toXml(writer);
    writer->writeEndElement();
}
/// \endcond

class QXmppMamResultIqPrivate : public QSharedData
{
public:
    QXmppResultSetReply resultSetReply;
    bool complete;
};

///
/// \class QXmppMamQueryIq
///
/// The QXmppMamQueryIq class represents the result IQ for \xep{0313, Message
/// Archive Management}.
///
/// \ingroup Stanzas
///
/// \since QXmpp 1.0
///

QXmppMamResultIq::QXmppMamResultIq()
    : d(new QXmppMamResultIqPrivate)
{
    d->complete = false;
}

/// Default copy constructor
QXmppMamResultIq::QXmppMamResultIq(const QXmppMamResultIq &) = default;

QXmppMamResultIq::~QXmppMamResultIq() = default;

/// Default assignemnt operator
QXmppMamResultIq &QXmppMamResultIq::operator=(const QXmppMamResultIq &) = default;

///
/// Returns the result set reply for result set management.
///
QXmppResultSetReply QXmppMamResultIq::resultSetReply() const
{
    return d->resultSetReply;
}

///
/// Sets the result set reply for result set management
///
void QXmppMamResultIq::setResultSetReply(const QXmppResultSetReply &resultSetReply)
{
    d->resultSetReply = resultSetReply;
}

///
/// Returns true if the results returned by the server are complete (not
/// limited by the server).
///
bool QXmppMamResultIq::complete() const
{
    return d->complete;
}

///
/// Sets if the results returned by the server are complete (not limited by the
/// server).
///
void QXmppMamResultIq::setComplete(bool complete)
{
    d->complete = complete;
}

/// \cond
bool QXmppMamResultIq::isMamResultIq(const QDomElement &element)
{
    if (element.tagName() == QStringLiteral("iq")) {
        QDomElement finElement = element.firstChildElement(QStringLiteral("fin"));
        if (!finElement.isNull() && finElement.namespaceURI() == ns_mam) {
            return true;
        }
    }
    return false;
}

void QXmppMamResultIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement finElement = element.firstChildElement(QStringLiteral("fin"));
    d->complete = finElement.attribute(QStringLiteral("complete")) == QStringLiteral("true");
    QDomElement resultSetElement = finElement.firstChildElement(QStringLiteral("set"));
    if (!resultSetElement.isNull()) {
        d->resultSetReply.parse(resultSetElement);
    }
}

void QXmppMamResultIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("fin"));
    writer->writeDefaultNamespace(ns_mam);
    if (d->complete) {
        writer->writeAttribute(QStringLiteral("complete"), QStringLiteral("true"));
    }
    d->resultSetReply.toXml(writer);
    writer->writeEndElement();
}
/// \endcond
