/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Author:
 *  Linus Jahn <lnj@kaidan.im>
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

#include "QXmppMixItem.h"
#include "QXmppDataForm.h"
#include "QXmppConstants_p.h"
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

QXmppMixInfoItem::QXmppMixInfoItem()
    : d(new QXmppMixInfoItemPrivate)
{
}

QXmppMixInfoItem::QXmppMixInfoItem(const QXmppMixInfoItem &) = default;

QXmppMixInfoItem &QXmppMixInfoItem::operator=(const QXmppMixInfoItem &) = default;

QXmppMixInfoItem::~QXmppMixInfoItem() = default;

/// Returns the user-specified name of the MIX channel. This is not the name
/// part of the channel's JID.

QString QXmppMixInfoItem::name() const
{
    return d->name;
}

/// Sets the name of the channel.

void QXmppMixInfoItem::setName(const QString& name)
{
    d->name = name;
}

/// Returns the description of the channel. This string might be very long.

QString QXmppMixInfoItem::description() const
{
    return d->description;
}

/// Sets the longer channel description.

void QXmppMixInfoItem::setDescription(const QString& description)
{
    d->description = description;
}

/// Returns a list of JIDs that are responsible for this channel.

QStringList QXmppMixInfoItem::contactJids() const
{
    return d->contactJids;
}

/// Sets a list of public JIDs that are responsible for this channel.

void QXmppMixInfoItem::setContactJids(const QStringList& contactJids)
{
    d->contactJids = contactJids;
}

/// Returns true, if the given dom element is a MIX channel info item.

bool QXmppMixInfoItem::isMixChannelInfo(const QDomElement& element)
{
    QXmppDataForm form;
    form.parse(element);
    for (const auto &field : form.fields()) {
        if (field.key() == "FORM_TYPE")
            return field.value() == ns_mix;
    }
    return false;
}

void QXmppMixInfoItem::parse(const QXmppElement& element)
{
    QXmppDataForm form;
    form.parse(element.sourceDomElement());

    for (auto& field : form.fields()) {
        if (field.key() == "Name")
            d->name = field.value().toString();
        else if (field.key() == "Description")
            d->description = field.value().toString();
        else if (field.key() == "Contact")
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
    formType.setKey("FORM_TYPE");
    formType.setValue(ns_mix);
    fields << formType;

    QXmppDataForm::Field nameField;
    nameField.setKey("Name");
    nameField.setValue(d->name);
    fields << nameField;

    QXmppDataForm::Field descriptionField;
    descriptionField.setKey("Description");
    descriptionField.setValue(d->description);
    fields << descriptionField;

    QXmppDataForm::Field contactsField;
    contactsField.setKey("Contact");
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

class QXmppMixParticipantItemPrivate : public QSharedData
{
public:
    QString nick;
    QString jid;
};

QXmppMixParticipantItem::QXmppMixParticipantItem()
    : d(new QXmppMixParticipantItemPrivate)
{
}

QXmppMixParticipantItem::QXmppMixParticipantItem(const QXmppMixParticipantItem &) = default;

QXmppMixParticipantItem &QXmppMixParticipantItem::operator=(const QXmppMixParticipantItem &) = default;

QXmppMixParticipantItem::~QXmppMixParticipantItem() = default;

/// Returns the participant's nickname.

QString QXmppMixParticipantItem::nick() const
{
    return d->nick;
}

/// Sets the participants nickname.

void QXmppMixParticipantItem::setNick(const QString& nick)
{
    d->nick = nick;
}

/// Returns the participant's JID.

QString QXmppMixParticipantItem::jid() const
{
    return d->jid;
}

/// Sets the participant's JID.

void QXmppMixParticipantItem::setJid(const QString& jid)
{
    d->jid = jid;
}

void QXmppMixParticipantItem::parse(const QXmppElement& itemContent)
{
    d->nick = itemContent.firstChildElement("nick").value();
    d->jid = itemContent.firstChildElement("jid").value();
}

QXmppElement QXmppMixParticipantItem::toElement() const
{
    QXmppElement element;
    element.setTagName("participant");
    element.setAttribute("xmlns", ns_mix);

    QXmppElement jid;
    jid.setTagName("jid");
    jid.setValue(d->jid);
    element.appendChild(jid);

    QXmppElement nick;
    nick.setTagName("nick");
    nick.setValue(d->nick);
    element.appendChild(nick);

    return element;
}

/// Returns true, if this dom element is a MIX participant item.

bool QXmppMixParticipantItem::isMixParticipantItem(const QDomElement &element)
{
    return element.tagName() == "participant" && element.namespaceURI() == ns_mix;
}
