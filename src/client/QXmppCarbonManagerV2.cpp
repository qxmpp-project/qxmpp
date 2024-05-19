// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppCarbonManagerV2.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppMessage.h"
#include "QXmppOutgoingClient.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp;
using namespace QXmpp::Private;

class CarbonEnableIq : public QXmppIq
{
public:
    CarbonEnableIq()
    {
        setType(QXmppIq::Set);
    }

    // parsing not implemented
    void parseElementFromChild(const QDomElement &) override
    {
    }
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override
    {
        writer->writeStartElement(QSL65("enable"));
        writer->writeDefaultNamespace(toString65(ns_carbons));
        writer->writeEndElement();
    }
};

auto parseIq(std::variant<QDomElement, QXmppError> &&sendResult) -> std::optional<QXmppError>
{
    if (auto el = std::get_if<QDomElement>(&sendResult)) {
        auto iqType = el->attribute(u"type"_s);
        if (iqType == u"result") {
            return {};
        }
        QXmppIq iq;
        iq.parse(*el);
        if (auto error = iq.errorOptional()) {
            return QXmppError { error->text(), std::move(*error) };
        }
        // Only happens with IQs with type=error, but no <error/> element
        return QXmppError { u"Unknown error received."_s, QXmppStanza::Error() };
    } else if (auto err = std::get_if<QXmppError>(&sendResult)) {
        return *err;
    }
    return {};
}

///
/// \class QXmppCarbonManagerV2
///
/// \brief The QXmppCarbonManagerV2 class handles message carbons as described in \xep{0280,
/// Message Carbons}.
///
/// The manager automatically enables carbons when a connection is established. Either by using
/// \xep{0386, Bind 2} if available or by sending a normal IQ request on connection.
/// Carbon copied messages from other devices of the same account and carbon copied messages from
/// other accounts are injected into the QXmppClient. This way you can handle them like any other
/// incoming message by implementing QXmppMessageHandler or using QXmppClient::messageReceived().
///
/// Checks are done to ensure that the entity sending the carbon copy is allowed to send the
/// forwarded message.
///
/// You don't need to do anything other than adding the extension to the client to use it.
/// \code
/// QXmppClient client;
/// client.addNewExtension<QXmppCarbonManagerV2>();
/// \endcode
///
/// To distinguish carbon messages, you can use QXmppMessage::isCarbonMessage().
///
/// \note Enabling via Bind 2 has been added in QXmpp 1.8.
///
/// \ingroup Managers
///
/// \since QXmpp 1.5
///

QXmppCarbonManagerV2::QXmppCarbonManagerV2() = default;
QXmppCarbonManagerV2::~QXmppCarbonManagerV2() = default;

bool QXmppCarbonManagerV2::handleStanza(const QDomElement &element, const std::optional<QXmppE2eeMetadata> &)
{
    if (element.tagName() != u"message") {
        return false;
    }

    auto carbon = firstChildElement(element, {}, ns_carbons);
    if (carbon.isNull() || (carbon.tagName() != u"sent" && carbon.tagName() != u"received")) {
        return false;
    }

    // carbon copies must always come from our bare JID
    auto from = element.attribute(u"from"_s);
    if (from != client()->configuration().jidBare()) {
        info(u"Received carbon copy from attacker or buggy client '" + from + u"' trying to use CVE-2017-5603.");
        return false;
    }

    auto forwarded = firstChildElement(carbon, u"forwarded", ns_forwarding);
    auto messageElement = firstChildElement(forwarded, u"message", ns_client);
    if (messageElement.isNull()) {
        return false;
    }

    QXmppMessage message;
    message.parse(messageElement);
    message.setCarbonForwarded(true);

    injectMessage(std::move(message));
    return true;
}

void QXmppCarbonManagerV2::onRegistered(QXmppClient *client)
{
    client->stream()->carbonManager().setEnableViaBind2(true);
    connect(client, &QXmppClient::connected, this, &QXmppCarbonManagerV2::enableCarbons);
}

void QXmppCarbonManagerV2::onUnregistered(QXmppClient *client)
{
    client->stream()->carbonManager().setEnableViaBind2(false);
    disconnect(client, &QXmppClient::connected, this, &QXmppCarbonManagerV2::enableCarbons);
}

void QXmppCarbonManagerV2::enableCarbons()
{
    // skip if stream could be resumed or carbons have been enabled via bind2 already
    if (client()->streamManagementState() == QXmppClient::ResumedStream ||
        client()->stream()->carbonManager().enabled()) {
        return;
    }

    client()->sendIq(CarbonEnableIq()).then(this, [this](QXmppClient::IqResult domResult) {
        if (auto err = parseIq(std::move(domResult))) {
            warning(u"Could not enable message carbons: " + err->description);
        } else {
            info(u"Message Carbons enabled."_s);
        }
    });
}
