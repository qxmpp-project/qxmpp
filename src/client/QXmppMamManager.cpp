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

#include <QDomElement>

#include "QXmppMamManager.h"
#include "QXmppMamIq.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppDataForm.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

/// \cond
QStringList QXmppMamManager::discoveryFeatures() const
{
    // XEP-0313: Message Archive Management
    return QStringList() << ns_mam;
}

bool QXmppMamManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == "message") {
        QDomElement resultElement = element.firstChildElement("result");
        if (!resultElement.isNull() && resultElement.namespaceURI() == ns_mam) {
            QDomElement forwardedElement = resultElement.firstChildElement("forwarded");
            QString queryId = resultElement.attribute("queryid");
            if (!forwardedElement.isNull() && forwardedElement.namespaceURI() == ns_forwarding) {
                QDomElement messageElement = forwardedElement.firstChildElement("message");
                QDomElement delayElement = forwardedElement.firstChildElement("delay");
                if (!messageElement.isNull()) {
                    QXmppMessage message;
                    message.parse(messageElement);
                    if (!delayElement.isNull() && delayElement.namespaceURI() == ns_delayed_delivery) {
                        const QString stamp = delayElement.attribute("stamp");
                        message.setStamp(QXmppUtils::datetimeFromString(stamp));
                    }
                    emit archivedMessageReceived(queryId, message);
                }
            }
            return true;
        }
    } else if (QXmppMamResultIq::isMamResultIq(element)) {
        QXmppMamResultIq result;
        result.parse(element);
        emit resultsRecieved(result.id(), result.resultSetReply(), result.complete());
        return true;
    }

    return false;
}
/// \endcond

/// Retrieves archived messages. For each received message, the
/// archiveMessageReceived() signal is emitted. Once all messages are received,
/// the resultsRecieved() signal is emitted. It returns a result set that can
/// be used to page through the results.
/// The number of results may be limited by the server.
///
/// \param to Optional entity that should be queried. Leave this empty to query
///           the local archive.
/// \param node Optional node that should be queried. This is used when querying
///             a pubsub node.
/// \param jid Optional JID to filter the results.
/// \param start Optional start time to filter the results.
/// \param end Optional end time to filter the results.
/// \param resultSetQuery Optional Result Set Management query. This can be used
///                       to limit the number of results and to page through the
///                       archive.
/// \return query id of the request. This can be used to associate the
///         corresponding resultsRecieved signal.
///
QString QXmppMamManager::retrieveArchivedMessages(const QString &to,
                                                  const QString &node,
                                                  const QString &jid,
                                                  const QDateTime &start,
                                                  const QDateTime &end,
                                                  const QXmppResultSetQuery &resultSetQuery)
{
    QList<QXmppDataForm::Field> fields;

    QXmppDataForm::Field hiddenField(QXmppDataForm::Field::HiddenField);
    hiddenField.setKey("FORM_TYPE");
    hiddenField.setValue(ns_mam);
    fields << hiddenField;

    if (!jid.isEmpty()) {
        QXmppDataForm::Field jidField;
        jidField.setKey("with");
        jidField.setValue(jid);
        fields << jidField;
    }

    if (start.isValid()) {
        QXmppDataForm::Field startField;
        startField.setKey("start");
        startField.setValue(QXmppUtils::datetimeToString(start));
        fields << startField;
    }

    if (end.isValid()) {
        QXmppDataForm::Field endField;
        endField.setKey("end");
        endField.setValue(QXmppUtils::datetimeToString(end));
        fields << endField;
    }

    QXmppDataForm form;
    form.setType(QXmppDataForm::Submit);
    form.setFields(fields);

    QXmppMamQueryIq queryIq;
    QString queryId = queryIq.id(); /* reuse the IQ id as query id */
    queryIq.setTo(to);
    queryIq.setNode(node);
    queryIq.setQueryId(queryId);
    queryIq.setForm(form);
    queryIq.setResultSetQuery(resultSetQuery);
    client()->sendPacket(queryIq);
    return queryId;
}
