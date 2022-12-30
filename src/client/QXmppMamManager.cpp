// SPDX-FileCopyrightText: 2016 Niels Ole Salscheider <niels_ole@salscheider-online.de>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMamManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppDataForm.h"
#include "QXmppE2eeExtension.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppMamIq.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

#include <unordered_map>

#include <QDomElement>
#include <QFuture>
#include <QFutureInterface>

using namespace QXmpp::Private;

struct RetrieveRequestState
{
    QFutureInterface<QXmppMamManager::RetrieveResult> interface;
    QXmppMamResultIq iq;
    QVector<QXmppMessage> messages;

    void reportFinished()
    {
        interface.reportResult(
            QXmppMamManager::RetrievedMessages {
                std::move(iq),
                std::move(messages) });
        interface.reportFinished();
    }
};

class QXmppMamManagerPrivate
{
public:
    // std::string because older Qt 5 versions don't add std::hash support for QString
    std::unordered_map<std::string, RetrieveRequestState> ongoingRequests;
};

///
/// \struct QXmppMamManager::RetrievedMessages
///
/// \brief Contains all retrieved messages and the result IQ that can be used for pagination.
///
/// \since QXmpp 1.5
///

///
/// \var QXmppMamManager::RetrievedMessages::result
///
/// The returned result IQ from the MAM server.
///

///
/// \var QXmppMamManager::RetrievedMessages::messages
///
/// A vector of retrieved QXmppMessages.
///

///
/// \typedef QXmppMamManager::RetrieveResult
///
/// Contains RetrievedMessages or an QXmppError.
///
/// \since QXmpp 1.5
///

QXmppMamManager::QXmppMamManager()
    : d(std::make_unique<QXmppMamManagerPrivate>())
{
}

QXmppMamManager::~QXmppMamManager() = default;

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

            if (forwardedElement.isNull() || forwardedElement.namespaceURI() != ns_forwarding) {
                return false;
            }

            auto messageElement = forwardedElement.firstChildElement("message");
            auto delayElement = forwardedElement.firstChildElement("delay");

            if (messageElement.isNull()) {
                return false;
            }

            QXmppMessage message;
            message.parse(messageElement);
            if (!delayElement.isNull() && delayElement.namespaceURI() == ns_delayed_delivery) {
                const QString stamp = delayElement.attribute("stamp");
                message.setStamp(QXmppUtils::datetimeFromString(stamp));
            }

            auto itr = d->ongoingRequests.find(queryId.toStdString());
            if (itr != d->ongoingRequests.end()) {
                // future-based API
                itr->second.messages.append(std::move(message));
            } else {
                // signal-based API
                Q_EMIT archivedMessageReceived(queryId, message);
            }
            return true;
        }
    } else if (QXmppMamResultIq::isMamResultIq(element)) {
        QXmppMamResultIq result;
        result.parse(element);
        Q_EMIT resultsRecieved(result.id(), result.resultSetReply(), result.complete());
        return true;
    }

    return false;
}
/// \endcond

static QXmppMamQueryIq buildRequest(const QString &to,
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
    return queryIq;
}

///
/// Retrieves archived messages. For each received message, the
/// archiveMessageReceived() signal is emitted. Once all messages are received,
/// the resultsRecieved() signal is emitted. It returns a result set that can
/// be used to page through the results.
/// The number of results may be limited by the server.
///
/// \warning This API does not work with end-to-end encrypted messages. You can
/// use the new QFuture-based API (retrieveMessages()) for that.
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
    auto queryIq = buildRequest(to, node, jid, start, end, resultSetQuery);
    client()->sendPacket(queryIq);
    return queryIq.id();
}

///
/// Retrieves archived messages and reports all messages at once via a QFuture.
///
/// This function tries to decrypt encrypted messages.
///
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
/// \since QXmpp 1.5
///
QFuture<QXmppMamManager::RetrieveResult> QXmppMamManager::retrieveMessages(const QString &to, const QString &node, const QString &jid, const QDateTime &start, const QDateTime &end, const QXmppResultSetQuery &resultSetQuery)
{
    auto queryIq = buildRequest(to, node, jid, start, end, resultSetQuery);

    auto [itr, _] = d->ongoingRequests.insert({ queryIq.queryId().toStdString(), RetrieveRequestState() });

    // retrieve messages
    await(client()->sendIq(std::move(queryIq)), this, [this, queryId = queryIq.queryId()](QXmppClient::IqResult result) {
        auto itr = d->ongoingRequests.find(queryId.toStdString());
        if (itr == d->ongoingRequests.end()) {
            return;
        }

        if (std::holds_alternative<QDomElement>(result)) {
            auto &iq = itr->second.iq;
            iq.parse(std::get<QDomElement>(result));

            if (iq.type() == QXmppIq::Error) {
                itr->second.interface.reportResult(QXmppError { iq.error().text(), iq.error() });
                itr->second.interface.reportFinished();
                d->ongoingRequests.erase(itr);
                return;
            }

            // decrypt encrypted messages
            if (auto *e2eeExt = client()->encryptionExtension()) {
                auto &messages = itr->second.messages;
                auto running = std::make_shared<uint>(0);

                for (auto i = 0; i < messages.size(); i++) {
                    if (!e2eeExt->isEncrypted(messages.at(i))) {
                        continue;
                    }

                    auto message = messages.at(i);
                    (*running)++;
                    await(e2eeExt->decryptMessage(std::move(message)), this, [this, i, running, queryId](auto result) {
                        (*running)--;
                        auto itr = d->ongoingRequests.find(queryId.toStdString());
                        if (itr == d->ongoingRequests.end()) {
                            return;
                        }

                        if (std::holds_alternative<QXmppMessage>(result)) {
                            itr->second.messages[i] = std::get<QXmppMessage>(std::move(result));
                        } else {
                            warning(QStringLiteral("Error decrypting message."));
                        }
                        if (*running == 0) {
                            itr->second.reportFinished();
                            d->ongoingRequests.erase(itr);
                        }
                    });
                }
            } else {
                itr->second.reportFinished();
                d->ongoingRequests.erase(itr);
            }
        } else {
            itr->second.interface.reportResult(std::get<QXmppError>(result));
            itr->second.interface.reportFinished();
            d->ongoingRequests.erase(itr);
        }
    });

    return itr->second.interface.future();
}
