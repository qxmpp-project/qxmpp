// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppCarbonManagerV2.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppMessage.h"

#include <QDomElement>
#include <QStringBuilder>

using namespace QXmpp;
using namespace QXmpp::Private;

class CarbonEnableIq : public QXmppIq
{
public:
    CarbonEnableIq()
        : QXmppIq()
    {
        setType(QXmppIq::Set);
    }

    // parsing not implemented
    void parseElementFromChild(const QDomElement &) override
    {
    }
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override
    {
        writer->writeStartElement(QStringLiteral("enable"));
        writer->writeDefaultNamespace(ns_carbons);
        writer->writeEndElement();
    }
};

auto firstXmlnsElement(const QDomElement &el, const char *xmlns)
{
    for (auto child = el.firstChildElement();
         !child.isNull();
         child = child.nextSiblingElement()) {
        if (child.namespaceURI() == xmlns) {
            return child;
        }
    }
    return QDomElement();
}

auto firstChildElement(const QDomElement &el, const char *tagName, const char *xmlns)
{
    for (auto child = el.firstChild();
         !child.isNull();
         child = child.nextSibling()) {
        if (child.isElement() && child.namespaceURI() == xmlns) {
            auto childEl = child.toElement();
            if (childEl.tagName() == tagName) {
                return childEl;
            }
        }
    }
    return QDomElement();
}

auto parseIq(std::variant<QDomElement, QXmppError> &&sendResult) -> std::optional<QXmppError>
{
    if (auto el = std::get_if<QDomElement>(&sendResult)) {
        auto iqType = el->attribute(QStringLiteral("type"));
        if (iqType == "result") {
            return {};
        }
        QXmppIq iq;
        iq.parse(*el);
        if (auto error = iq.errorOptional()) {
            return QXmppError { error->text(), std::move(*error) };
        }
        // Only happens with IQs with type=error, but no <error/> element
        return QXmppError { QStringLiteral("Unknown error received."), QXmppStanza::Error() };
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
/// The manager automatically enables carbons when a connection is established. If the connection
/// could be resumed, no new request is done. Carbon copied messages from other devices of the same
/// account and carbon copied messages from other accounts are injected into the QXmppClient. This
/// way you can handle them like any other incoming message by implementing QXmppMessageHandler or
/// using QXmppClient::messageReceived().
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
/// \ingroup Managers
///
/// \since QXmpp 1.5
///

QXmppCarbonManagerV2::QXmppCarbonManagerV2() = default;
QXmppCarbonManagerV2::~QXmppCarbonManagerV2() = default;

bool QXmppCarbonManagerV2::handleStanza(const QDomElement &element, const std::optional<QXmppE2eeMetadata> &)
{
    if (element.tagName() != "message") {
        return false;
    }

    auto carbon = firstXmlnsElement(element, ns_carbons);
    if (carbon.isNull() || (carbon.tagName() != "sent" && carbon.tagName() != "received")) {
        return false;
    }

    // carbon copies must always come from our bare JID
    auto from = element.attribute(QStringLiteral("from"));
    if (from != client()->configuration().jidBare()) {
        info("Received carbon copy from attacker or buggy client '" % from %
             "' trying to use CVE-2017-5603.");
        return false;
    }

    auto forwarded = firstChildElement(carbon, "forwarded", ns_forwarding);
    auto messageElement = firstChildElement(forwarded, "message", ns_client);
    if (messageElement.isNull()) {
        return false;
    }

    QXmppMessage message;
    message.parse(messageElement);
    message.setCarbonForwarded(true);

    injectMessage(std::move(message));
    return true;
}

void QXmppCarbonManagerV2::setClient(QXmppClient *newClient)
{
    if (client()) {
        disconnect(client(), &QXmppClient::connected, this, &QXmppCarbonManagerV2::enableCarbons);
    }

    QXmppClientExtension::setClient(newClient);
    connect(newClient, &QXmppClient::connected, this, &QXmppCarbonManagerV2::enableCarbons);
}

void QXmppCarbonManagerV2::enableCarbons()
{
    if (client()->streamManagementState() == QXmppClient::ResumedStream) {
        // skip re-enabling for resumed streams
        return;
    }

    client()->sendIq(CarbonEnableIq()).then(this, [this](QXmppClient::IqResult domResult) {
        if (auto err = parseIq(std::move(domResult))) {
            warning("Could not enable message carbons: " % err->description);
        } else {
            info("Message Carbons enabled.");
        }
    });
}
