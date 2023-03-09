// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppDataFormBase.h"
#include "QXmppMixInfoItem.h"
#include "QXmppMixParticipantItem.h"

static const auto NAME = QStringLiteral("Name");
static const auto DESCRIPTION = QStringLiteral("Description");
static const auto CONTACT_JIDS = QStringLiteral("Contact");

class QXmppMixInfoItemPrivate : public QSharedData, public QXmppDataFormBase
{
public:
    QString name;
    QString description;
    QStringList contactJids;

    ~QXmppMixInfoItemPrivate() override = default;

    void reset()
    {
        name.clear();
        description.clear();
        contactJids.clear();
    }

    QString formType() const override
    {
        return ns_mix;
    }

    void parseForm(const QXmppDataForm &form) override
    {
        const auto fields = form.fields();
        for (const auto &field : fields) {
            const auto key = field.key();
            const auto value = field.value();

            if (key == NAME) {
                name = value.toString();
            } else if (key == DESCRIPTION) {
                description = value.toString();
            } else if (key == CONTACT_JIDS) {
                contactJids = value.toStringList();
            }
        }
    }
    void serializeForm(QXmppDataForm &form) const override
    {
        using Type = QXmppDataForm::Field::Type;
        serializeNullable(form, Type::TextSingleField, NAME, name);
        serializeNullable(form, Type::TextSingleField, DESCRIPTION, description);
        serializeEmptyable(form, Type::JidMultiField, CONTACT_JIDS, contactJids);
    }
};

///
/// \class QXmppMixInfoItem
///
/// \brief The QXmppMixInfoItem class represents a PubSub item of a MIX
/// channel containing channel information as defined by \xep{0369, Mediated
/// Information eXchange (MIX)}.
///
/// \since QXmpp 1.5
///
/// \ingroup Stanzas
///

QXmppMixInfoItem::QXmppMixInfoItem()
    : d(new QXmppMixInfoItemPrivate)
{
}

/// Default copy-constructor
QXmppMixInfoItem::QXmppMixInfoItem(const QXmppMixInfoItem &) = default;
/// Default move-constructor
QXmppMixInfoItem::QXmppMixInfoItem(QXmppMixInfoItem &&) = default;
/// Default assignment operator
QXmppMixInfoItem &QXmppMixInfoItem::operator=(const QXmppMixInfoItem &) = default;
/// Default move-assignment operator
QXmppMixInfoItem &QXmppMixInfoItem::operator=(QXmppMixInfoItem &&) = default;
QXmppMixInfoItem::~QXmppMixInfoItem() = default;

///
/// Returns the user-specified name of the MIX channel. This is not the name
/// part of the channel's JID.
///
const QString &QXmppMixInfoItem::name() const
{
    return d->name;
}

///
/// Sets the name of the channel.
///
void QXmppMixInfoItem::setName(QString name)
{
    d->name = std::move(name);
}

///
/// Returns the description of the channel. This string might be very long.
///
const QString &QXmppMixInfoItem::description() const
{
    return d->description;
}

///
/// Sets the longer channel description.
///
void QXmppMixInfoItem::setDescription(QString description)
{
    d->description = std::move(description);
}

///
/// Returns a list of JIDs that are responsible for this channel.
///
const QStringList &QXmppMixInfoItem::contactJids() const
{
    return d->contactJids;
}

///
/// Sets a list of public JIDs that are responsible for this channel.
///
void QXmppMixInfoItem::setContactJids(QStringList contactJids)
{
    d->contactJids = std::move(contactJids);
}

///
/// Returns true, if the given dom element is a MIX channel info item.
///
bool QXmppMixInfoItem::isItem(const QDomElement &element)
{
    return QXmppPubSubBaseItem::isItem(element, [](const QDomElement &payload) {
        // check FORM_TYPE without parsing a full QXmppDataForm
        if (payload.tagName() != u'x' || payload.namespaceURI() != ns_data) {
            return false;
        }
        for (auto fieldEl = payload.firstChildElement();
             !fieldEl.isNull();
             fieldEl = fieldEl.nextSiblingElement()) {
            if (fieldEl.attribute(QStringLiteral("var")) == QStringLiteral(u"FORM_TYPE")) {
                return fieldEl.firstChildElement(QStringLiteral("value")).text() == ns_mix;
            }
        }
        return false;
    });
}

/// \cond
void QXmppMixInfoItem::parsePayload(const QDomElement &payload)
{
    d->reset();

    QXmppDataForm form;
    form.parse(payload);

    d->parseForm(form);
}

void QXmppMixInfoItem::serializePayload(QXmlStreamWriter *writer) const
{
    auto form = d->toDataForm();
    form.setType(QXmppDataForm::Result);
    form.toXml(writer);
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
/// \since QXmpp 1.5
///
/// \ingroup Stanzas
///

QXmppMixParticipantItem::QXmppMixParticipantItem()
    : d(new QXmppMixParticipantItemPrivate)
{
}

/// Default copy-constructor
QXmppMixParticipantItem::QXmppMixParticipantItem(const QXmppMixParticipantItem &) = default;
/// Default move-constructor
QXmppMixParticipantItem::QXmppMixParticipantItem(QXmppMixParticipantItem &&) = default;
/// Default assignment operator
QXmppMixParticipantItem &QXmppMixParticipantItem::operator=(const QXmppMixParticipantItem &) = default;
/// Default move-assignment operator
QXmppMixParticipantItem &QXmppMixParticipantItem::operator=(QXmppMixParticipantItem &&) = default;
QXmppMixParticipantItem::~QXmppMixParticipantItem() = default;

///
/// Returns the participant's nickname.
///
const QString &QXmppMixParticipantItem::nick() const
{
    return d->nick;
}

///
/// Sets the participants nickname.
///
void QXmppMixParticipantItem::setNick(QString nick)
{
    d->nick = std::move(nick);
}

///
/// Returns the participant's JID.
///
const QString &QXmppMixParticipantItem::jid() const
{
    return d->jid;
}

///
/// Sets the participant's JID.
///
void QXmppMixParticipantItem::setJid(QString jid)
{
    d->jid = std::move(jid);
}

/// \cond
void QXmppMixParticipantItem::parsePayload(const QDomElement &payload)
{
    d->nick = payload.firstChildElement(QStringLiteral("nick")).text();
    d->jid = payload.firstChildElement(QStringLiteral("jid")).text();
}

void QXmppMixParticipantItem::serializePayload(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("participant"));
    writer->writeDefaultNamespace(ns_mix);
    if (!d->jid.isEmpty()) {
        writer->writeTextElement("jid", d->jid);
    }
    if (!d->nick.isEmpty()) {
        writer->writeTextElement("nick", d->nick);
    }
    writer->writeEndElement();
}
/// \endcond

///
/// Returns true, if this dom element is a MIX participant item.
///
bool QXmppMixParticipantItem::isItem(const QDomElement &element)
{
    return QXmppPubSubBaseItem::isItem(element, [](const QDomElement &payload) {
        return payload.tagName() == QStringLiteral("participant") &&
            payload.namespaceURI() == ns_mix;
    });
}
