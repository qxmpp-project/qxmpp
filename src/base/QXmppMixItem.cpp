// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMixItem.h"

#include "QXmppConstants_p.h"
#include "QXmppDataForm.h"
#include "QXmppUtils.h"

#include <QBuffer>
#include <QDomElement>
#include <QSharedData>

class QXmppMixInfoItemPrivate : public QSharedData
{
public:
    QString name;
    QString description;
    QStringList contactJids;
};

///
/// \class QXmppMixInfoItem
///
/// \brief The QXmppMixInfoItem class represents a PubSub item of a MIX
/// channel containing channel information as defined by \xep{0369, Mediated
/// Information eXchange (MIX)}.
///
/// \since QXmpp 1.1
///
/// \ingroup Stanzas
///

QXmppMixInfoItem::QXmppMixInfoItem()
    : d(new QXmppMixInfoItemPrivate)
{
}

/// Default copy-constructor
QXmppMixInfoItem::QXmppMixInfoItem(const QXmppMixInfoItem&) = default;

/// Default assignment operator
QXmppMixInfoItem& QXmppMixInfoItem::operator=(const QXmppMixInfoItem&) = default;

QXmppMixInfoItem::~QXmppMixInfoItem() = default;

///
/// Returns the user-specified name of the MIX channel. This is not the name
/// part of the channel's JID.
///
QString QXmppMixInfoItem::name() const
{
    return d->name;
}

///
/// Sets the name of the channel.
///
void QXmppMixInfoItem::setName(const QString& name)
{
    d->name = name;
}

///
/// Returns the description of the channel. This string might be very long.
///
QString QXmppMixInfoItem::description() const
{
    return d->description;
}

///
/// Sets the longer channel description.
///
void QXmppMixInfoItem::setDescription(const QString& description)
{
    d->description = description;
}

///
/// Returns a list of JIDs that are responsible for this channel.
///
QStringList QXmppMixInfoItem::contactJids() const
{
    return d->contactJids;
}

///
/// Sets a list of public JIDs that are responsible for this channel.
///
void QXmppMixInfoItem::setContactJids(const QStringList& contactJids)
{
    d->contactJids = contactJids;
}

///
/// Returns true, if the given dom element is a MIX channel info item.
///
bool QXmppMixInfoItem::isMixChannelInfo(const QDomElement& element)
{
    QXmppDataForm form;
    form.parse(element);
    for (const auto& field : form.fields()) {
        if (field.key() == QStringLiteral("FORM_TYPE"))
            return field.value() == ns_mix;
    }
    return false;
}

/// \cond
void QXmppMixInfoItem::parse(const QXmppElement& element)
{
    QXmppDataForm form;
    form.parse(element.sourceDomElement());

    for (auto& field : form.fields()) {
        if (field.key() == QStringLiteral("Name"))
            d->name = field.value().toString();
        else if (field.key() == QStringLiteral("Description"))
            d->description = field.value().toString();
        else if (field.key() == QStringLiteral("Contact"))
            d->contactJids = field.value().toStringList();
    }
}

QXmppElement QXmppMixInfoItem::toElement() const
{
    QXmppDataForm form;
    form.setType(QXmppDataForm::Result);
    QList<QXmppDataForm::Field> fields;

    QXmppDataForm::Field formType;
    formType.setType(QXmppDataForm::Field::HiddenField);
    formType.setKey(QStringLiteral("FORM_TYPE"));
    formType.setValue(ns_mix);
    fields << formType;

    QXmppDataForm::Field nameField;
    nameField.setKey(QStringLiteral("Name"));
    nameField.setValue(d->name);
    fields << nameField;

    QXmppDataForm::Field descriptionField;
    descriptionField.setKey(QStringLiteral("Description"));
    descriptionField.setValue(d->description);
    fields << descriptionField;

    QXmppDataForm::Field contactsField;
    contactsField.setKey(QStringLiteral("Contact"));
    contactsField.setValue(d->contactJids);
    contactsField.setType(QXmppDataForm::Field::JidMultiField);
    fields << contactsField;

    form.setFields(fields);

    // FIXME: this is too complicated; maybe don't use QXmppElement in QXmppPubSubItem?
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    form.toXml(&writer);

    QDomDocument doc;
    doc.setContent(buffer.data());
    return QXmppElement(doc.documentElement());
}
/// \endcond

class QXmppMixParticipantItemPrivate : public QSharedData
{
public:
    QString nick;
    QString jid;
};

///
/// \class QXmppMixParticipantItem
///
/// The QXmppMixParticipantItem class represents a PubSub item of a MIX channel
/// participant as defined by \xep{0369, Mediated Information eXchange (MIX)}.
///
/// \since QXmpp 1.1
///
/// \ingroup Stanzas
///

QXmppMixParticipantItem::QXmppMixParticipantItem()
    : d(new QXmppMixParticipantItemPrivate)
{
}

/// Default copy-constructor
QXmppMixParticipantItem::QXmppMixParticipantItem(const QXmppMixParticipantItem&) = default;

/// Default assignment operator
QXmppMixParticipantItem& QXmppMixParticipantItem::operator=(const QXmppMixParticipantItem&) = default;

QXmppMixParticipantItem::~QXmppMixParticipantItem() = default;

///
/// Returns the participant's nickname.
///
QString QXmppMixParticipantItem::nick() const
{
    return d->nick;
}

///
/// Sets the participants nickname.
///
void QXmppMixParticipantItem::setNick(const QString& nick)
{
    d->nick = nick;
}

///
/// Returns the participant's JID.
///
QString QXmppMixParticipantItem::jid() const
{
    return d->jid;
}

///
/// Sets the participant's JID.
///
void QXmppMixParticipantItem::setJid(const QString& jid)
{
    d->jid = jid;
}

/// \cond
void QXmppMixParticipantItem::parse(const QXmppElement& itemContent)
{
    d->nick = itemContent.firstChildElement(QStringLiteral("nick")).value();
    d->jid = itemContent.firstChildElement(QStringLiteral("jid")).value();
}

QXmppElement QXmppMixParticipantItem::toElement() const
{
    QXmppElement element;
    element.setTagName(QStringLiteral("participant"));
    element.setAttribute(QStringLiteral("xmlns"), ns_mix);

    QXmppElement jid;
    jid.setTagName(QStringLiteral("jid"));
    jid.setValue(d->jid);
    element.appendChild(jid);

    QXmppElement nick;
    nick.setTagName(QStringLiteral("nick"));
    nick.setValue(d->nick);
    element.appendChild(nick);

    return element;
}
/// \endcond

///
/// Returns true, if this dom element is a MIX participant item.
///
bool QXmppMixParticipantItem::isMixParticipantItem(const QDomElement& element)
{
    return element.tagName() == QStringLiteral("participant") && element.namespaceURI() == ns_mix;
}
