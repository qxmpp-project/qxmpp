// SPDX-FileCopyrightText: 2016 Niels Ole Salscheider <niels_ole@salscheider-online.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMamIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

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
/// Default move constructor
QXmppMamQueryIq::QXmppMamQueryIq(QXmppMamQueryIq &&) = default;
QXmppMamQueryIq::~QXmppMamQueryIq() = default;
/// Default assignemnt operator
QXmppMamQueryIq &QXmppMamQueryIq::operator=(const QXmppMamQueryIq &) = default;
/// Default move-assignemnt operator
QXmppMamQueryIq &QXmppMamQueryIq::operator=(QXmppMamQueryIq &&) = default;

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
    return isIqType(element, u"query", ns_mam);
}

void QXmppMamQueryIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(u"query"_s);
    d->node = queryElement.attribute(u"node"_s);
    d->queryId = queryElement.attribute(u"queryId"_s);
    QDomElement resultSetElement = queryElement.firstChildElement(u"set"_s);
    if (!resultSetElement.isNull()) {
        d->resultSetQuery.parse(resultSetElement);
    }
    QDomElement formElement = queryElement.firstChildElement(u"x"_s);
    if (!formElement.isNull()) {
        d->form.parse(formElement);
    }
}

void QXmppMamQueryIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("query"));
    writer->writeDefaultNamespace(toString65(ns_mam));
    if (!d->node.isEmpty()) {
        writer->writeAttribute(QSL65("node"), d->node);
    }
    if (!d->queryId.isEmpty()) {
        writer->writeAttribute(QSL65("queryid"), d->queryId);
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

/// Default move constructor
QXmppMamResultIq::QXmppMamResultIq(QXmppMamResultIq &&) = default;
/// Default copy constructor
QXmppMamResultIq::QXmppMamResultIq(const QXmppMamResultIq &) = default;
QXmppMamResultIq::~QXmppMamResultIq() = default;
/// Default assignemnt operator
QXmppMamResultIq &QXmppMamResultIq::operator=(const QXmppMamResultIq &) = default;
/// Default move-assignemnt operator
QXmppMamResultIq &QXmppMamResultIq::operator=(QXmppMamResultIq &&) = default;

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
    if (element.tagName() == u"iq") {
        QDomElement finElement = element.firstChildElement(u"fin"_s);
        if (!finElement.isNull() && finElement.namespaceURI() == ns_mam) {
            return true;
        }
    }
    return false;
}

void QXmppMamResultIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement finElement = element.firstChildElement(u"fin"_s);
    d->complete = finElement.attribute(u"complete"_s) == u"true";
    QDomElement resultSetElement = finElement.firstChildElement(u"set"_s);
    if (!resultSetElement.isNull()) {
        d->resultSetReply.parse(resultSetElement);
    }
}

void QXmppMamResultIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("fin"));
    writer->writeDefaultNamespace(toString65(ns_mam));
    if (d->complete) {
        writer->writeAttribute(QSL65("complete"), u"true"_s);
    }
    d->resultSetReply.toXml(writer);
    writer->writeEndElement();
}
/// \endcond
