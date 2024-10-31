// SPDX-FileCopyrightText: 2016 Niels Ole Salscheider <niels_ole@salscheider-online.de>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMamManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppDataForm.h"
#include "QXmppE2eeExtension.h"
#include "QXmppMamIq.h"
#include "QXmppMessage.h"
#include "QXmppPromise.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "Algorithms.h"
#include "StringLiterals.h"

#include <unordered_map>

#include <QDomElement>

using namespace QXmpp;
using namespace QXmpp::Private;

template<typename T>
auto sum(const T &c)
{
    return std::accumulate(c.begin(), c.end(), 0);
}

struct MamMessage {
    QDomElement element;
    std::optional<QDateTime> delay;
};

enum EncryptedType { Unencrypted,
                     Encrypted };

QXmppMessage parseMamMessage(const MamMessage &mamMessage, EncryptedType encrypted)
{
    QXmppMessage m;
    m.parse(mamMessage.element, encrypted == Encrypted ? ScePublic : SceAll);
    if (mamMessage.delay) {
        m.setStamp(*mamMessage.delay);
    }
    return m;
}

std::optional<std::tuple<MamMessage, QString>> parseMamMessageResult(const QDomElement &messageEl)
{
    auto resultElement = firstChildElement(messageEl, u"result", ns_mam);
    if (resultElement.isNull()) {
        return {};
    }

    auto forwardedElement = firstChildElement(resultElement, u"forwarded", ns_forwarding);
    if (forwardedElement.isNull()) {
        return {};
    }

    auto queryId = resultElement.attribute(u"queryid"_s);

    auto messageElement = firstChildElement(forwardedElement, u"message", ns_client);
    if (messageElement.isNull()) {
        return {};
    }

    auto parseDelay = [](const auto &forwardedEl) -> std::optional<QDateTime> {
        auto delayEl = firstChildElement(forwardedEl, u"delay", ns_delayed_delivery);
        if (!delayEl.isNull()) {
            return QXmppUtils::datetimeFromString(delayEl.attribute(u"stamp"_s));
        }
        return {};
    };

    return { { MamMessage { messageElement, parseDelay(forwardedElement) }, queryId } };
}

struct RetrieveRequestState {
    QXmppPromise<QXmppMamManager::RetrieveResult> promise;
    QXmppMamResultIq iq;
    QVector<MamMessage> messages;
    QVector<QXmppMessage> processedMessages;
    uint runningDecryptionJobs = 0;

    void finish()
    {
        Q_ASSERT(messages.count() == processedMessages.count());
        promise.finish(
            QXmppMamManager::RetrievedMessages {
                std::move(iq),
                std::move(processedMessages) });
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
    return { ns_mam.toString() };
}

bool QXmppMamManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == u"message") {
        if (auto result = parseMamMessageResult(element)) {
            auto &[message, queryId] = *result;

            auto itr = d->ongoingRequests.find(queryId.toStdString());
            if (itr != d->ongoingRequests.end()) {
                // future-based API
                itr->second.messages.append(std::move(message));
            } else {
                // signal-based API
                Q_EMIT archivedMessageReceived(queryId, parseMamMessage(message, Unencrypted));
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
    hiddenField.setKey(u"FORM_TYPE"_s);
    hiddenField.setValue(ns_mam.toString());
    fields << hiddenField;

    if (!jid.isEmpty()) {
        QXmppDataForm::Field jidField;
        jidField.setKey(u"with"_s);
        jidField.setValue(jid);
        fields << jidField;
    }

    if (start.isValid()) {
        QXmppDataForm::Field startField;
        startField.setKey(u"start"_s);
        startField.setValue(QXmppUtils::datetimeToString(start));
        fields << startField;
    }

    if (end.isValid()) {
        QXmppDataForm::Field endField;
        endField.setKey(u"end"_s);
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
/// \return result of the query
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppMamManager::RetrieveResult> QXmppMamManager::retrieveMessages(const QString &to, const QString &node, const QString &jid, const QDateTime &start, const QDateTime &end, const QXmppResultSetQuery &resultSetQuery)
{
    auto queryIq = buildRequest(to, node, jid, start, end, resultSetQuery);
    auto queryId = queryIq.queryId();

    auto [itr, inserted] = d->ongoingRequests.insert({ queryIq.queryId().toStdString(), RetrieveRequestState() });
    Q_ASSERT(inserted);

    // create task here; promise could finish immediately after client()->sendIq()
    auto task = itr->second.promise.task();

    // retrieve messages
    client()->sendIq(std::move(queryIq)).then(this, [this, queryId](QXmppClient::IqResult result) {
        auto itr = d->ongoingRequests.find(queryId.toStdString());
        if (itr == d->ongoingRequests.end()) {
            return;
        }
        auto &state = itr->second;

        // handle IQ sending errors
        if (std::holds_alternative<QXmppError>(result)) {
            state.promise.finish(std::get<QXmppError>(result));
            d->ongoingRequests.erase(itr);
            return;
        }

        // parse IQ
        auto &iq = state.iq;
        iq.parse(std::get<QDomElement>(result));

        // decrypt encrypted messages
        if (auto *e2eeExt = client()->encryptionExtension()) {
            // initialize processed messages (we need random access because
            // decryptMessage() may finish in random order)
            state.processedMessages.resize(state.messages.size());
            state.runningDecryptionJobs = state.messages.size();

            const auto size = state.messages.size();
            for (qsizetype i = 0; i < size; i++) {
                const auto &message = state.messages.at(i);

                // decrypt message if needed
                if (e2eeExt->isEncrypted(message.element)) {
                    e2eeExt->decryptMessage(parseMamMessage(state.messages.at(i), Encrypted)).then(this, [this, i, queryId](auto result) {
                        // find state (again)
                        auto itr = d->ongoingRequests.find(queryId.toStdString());
                        Q_ASSERT(itr != d->ongoingRequests.end());

                        auto &state = itr->second;

                        // store decrypted message, fallback to encrypted message
                        if (std::holds_alternative<QXmppMessage>(result)) {
                            state.processedMessages[i] = std::get<QXmppMessage>(std::move(result));
                        } else {
                            warning(u"Error decrypting message."_s);
                            state.processedMessages[i] = parseMamMessage(state.messages[i], Unencrypted);
                        }

                        // finish promise on last job
                        state.runningDecryptionJobs--;
                        if (state.runningDecryptionJobs == 0) {
                            state.finish();
                            d->ongoingRequests.erase(itr);
                        }
                    });
                } else {
                    state.processedMessages[i] = parseMamMessage(state.messages.at(i), Unencrypted);

                    // finish promise on last job (may be needed if no messages are encrypted or
                    // decryption finishes instantly)
                    state.runningDecryptionJobs--;
                    if (state.runningDecryptionJobs == 0) {
                        state.finish();
                        d->ongoingRequests.erase(itr);
                    }
                }
            }
        } else {
            // for the case without decryption
            state.processedMessages = transform<QVector<QXmppMessage>>(state.messages, [](const auto &m) {
                return parseMamMessage(m, Unencrypted);
            });

            state.finish();
            d->ongoingRequests.erase(itr);
        }
    });

    return task;
}
