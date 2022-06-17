// SPDX-FileCopyrightText: 2020 Robert Märkisch <zatrox@kaidan.im>
// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPushEnableIq.h"

#include "QXmppConstants_p.h"
#include "QXmppDataForm.h"

#include <QDomElement>

class QXmppPushEnableIqPrivate : public QSharedData
{
public:
    QString node;
    QString jid;
    QXmppPushEnableIq::Mode mode;
    QXmppDataForm dataForm;
};

QXmppPushEnableIq::QXmppPushEnableIq()
    : d(new QXmppPushEnableIqPrivate())
{
}

/// Default copy-constructor
QXmppPushEnableIq::QXmppPushEnableIq(const QXmppPushEnableIq &) = default;
/// Default move-constructor
QXmppPushEnableIq::QXmppPushEnableIq(QXmppPushEnableIq &&) = default;
QXmppPushEnableIq::~QXmppPushEnableIq() = default;
/// Default assignment operator
QXmppPushEnableIq &QXmppPushEnableIq::operator=(const QXmppPushEnableIq &) = default;
/// Default move-assignment operator
QXmppPushEnableIq &QXmppPushEnableIq::operator=(QXmppPushEnableIq &&) = default;

///
/// \brief Returns the jid of the app server
///
QString QXmppPushEnableIq::jid() const
{
    return d->jid;
}

///
/// \brief Sets the jid of the app server
///
void QXmppPushEnableIq::setJid(const QString &jid)
{
    d->jid = jid;
}

///
/// \brief Returns the pubsub node on the app server used by the IQ
///
QString QXmppPushEnableIq::node() const
{
    return d->node;
}

///
/// \brief Set the pubsub note on the app server to be used by the IQ
///
void QXmppPushEnableIq::setNode(const QString &node)
{
    d->node = node;
}

///
/// \brief Returns the mode
///
QXmppPushEnableIq::Mode QXmppPushEnableIq::mode()
{
    return d->mode;
}

///
/// \brief Set whether the IQ should enable or disable push notifications
///
void QXmppPushEnableIq::setMode(QXmppPushEnableIq::Mode mode)
{
    d->mode = mode;
}

///
/// \brief Returns the data form containing the publish options which the user
/// server Should send to the app server.
///
/// It is only available for enable IQs.
///
QXmppDataForm QXmppPushEnableIq::dataForm() const
{
    return d->dataForm;
}

///
/// \brief Sets the data form containing the publish options which the user
/// server Should send to the app server.
///
///  It should only be set for enable IQs.
///
void QXmppPushEnableIq::setDataForm(const QXmppDataForm &form)
{
    d->dataForm = form;
}

///
/// \brief Checks whether a QDomElement is a push notification enable / disable
/// IQ.
///
bool QXmppPushEnableIq::isPushEnableIq(const QDomElement &element)
{
    auto childElement = element.firstChildElement();
    return childElement.namespaceURI() == ns_push &&
        (childElement.tagName() == QStringLiteral("enable") || childElement.tagName() == QStringLiteral("disable"));
}

/// \cond
void QXmppPushEnableIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement childElement = element.firstChildElement();
    while (!childElement.isNull()) {
        if (childElement.namespaceURI() == ns_push) {
            if (childElement.tagName() == QStringLiteral("enable")) {
                d->mode = Enable;

                auto dataFormElement = childElement.firstChildElement("x");
                if (!dataFormElement.isNull() && dataFormElement.namespaceURI() == ns_data) {
                    QXmppDataForm dataForm;
                    dataForm.parse(dataFormElement);
                    d->dataForm = dataForm;
                }
            } else {
                d->mode = Disable;
            }
            d->jid = childElement.attribute(QStringLiteral("jid"));
            d->node = childElement.attribute(QStringLiteral("node"));
            break;
        }

        childElement = childElement.nextSiblingElement();
    }
}

void QXmppPushEnableIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    switch (d->mode) {
    case Enable:
        writer->writeStartElement(QStringLiteral("enable"));
        break;
    case Disable:
        writer->writeStartElement(QStringLiteral("disable"));
        break;
    }

    writer->writeDefaultNamespace(ns_push);
    writer->writeAttribute(QStringLiteral("jid"), d->jid);
    writer->writeAttribute(QStringLiteral("node"), d->node);

    if (d->mode == Enable) {
        d->dataForm.toXml(writer);
    }
    writer->writeEndElement();
}
/// \endcond
