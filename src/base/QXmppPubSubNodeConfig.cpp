// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubNodeConfig.h"

#include "StringLiterals.h"

static const auto NODE_CONFIG_FORM_TYPE = u"http://jabber.org/protocol/pubsub#node_config"_s;
static const auto PUBLISH_OPTIONS_FORM_TYPE = u"http://jabber.org/protocol/pubsub#publish-options"_s;

static const auto ACCESS_MODEL = u"pubsub#access_model"_s;
static const auto BODY_XSLT = u"pubsub#body_xslt"_s;
static const auto CHILD_ASSOCIATION_POLICY = u"pubsub#children_association_policy"_s;
static const auto CHILD_ASSOCIATION_ALLOWLIST = u"pubsub#children_association_whitelist"_s;
static const auto CHILD_NODES = u"pubsub#children"_s;
static const auto CHILD_NODES_MAX = u"pubsub#children_max"_s;
static const auto COLLECTIONS = u"pubsub#collection"_s;
static const auto CONTACT_JIDS = u"pubsub#contact"_s;
static const auto DATA_FORM_XSLT = u"pubsub#dataform_xslt"_s;
static const auto NOTIFICATIONS_ENABLED = u"pubsub#deliver_notifications"_s;
static const auto INCLUDE_PAYLOADS = u"pubsub#deliver_payloads"_s;
static const auto DESCRIPTION = u"pubsub#description"_s;
static const auto ITEM_EXPIRY = u"pubsub#item_expire"_s;
static const auto NOTIFICATION_ITEM_PUBLISHER = u"pubsub#itemreply"_s;
static const auto LANGUAGE = u"pubsub#language"_s;
static const auto MAX_ITEMS = u"pubsub#max_items"_s;
static const auto MAX_PAYLOAD_SIZE = u"pubsub#max_payload_size"_s;
static const auto NODE_TYPE = u"pubsub#node_type"_s;
static const auto NOTIFICATION_TYPE = u"pubsub#notification_type"_s;
static const auto CONFIG_NOTIFICATIONS_ENABLED = u"pubsub#notify_config"_s;
static const auto NODE_DELETE_NOTIFICATIONS_ENABLED = u"pubsub#notify_delete"_s;
static const auto RETRACT_NOTIFICATIONS_ENABLED = u"pubsub#notify_retract"_s;
static const auto SUB_NOTIFICATIONS_ENABLED = u"pubsub#notify_sub"_s;
static const auto PERSIST_ITEMS = u"pubsub#persist_items"_s;
static const auto PRESENCE_BASED_NOTIFICATIONS = u"pubsub#presence_based_delivery"_s;
static const auto PUBLISH_MODEL = u"pubsub#publish_model"_s;
static const auto PURGE_WHEN_OFFLINE = u"pubsub#purge_offline"_s;
static const auto ALLOWED_ROSTER_GROUPS = u"pubsub#roster_groups_allowed"_s;
static const auto SEND_LAST_ITEM = u"pubsub#send_last_published_item"_s;
static const auto TEMPORARY_SUBSCRIPTIONS = u"pubsub#tempsub"_s;
static const auto ALLOW_SUBSCRIPTIONS = u"pubsub#subscribe"_s;
static const auto TITLE = u"pubsub#title"_s;
static const auto PAYLOAD_TYPE = u"pubsub#type"_s;

// helper for std::visit
template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

class QXmppPubSubNodeConfigPrivate : public QSharedData
{
public:
    std::optional<QXmppPubSubNodeConfig::AccessModel> accessModel;
    QString bodyXslt;
    std::optional<QXmppPubSubNodeConfig::ChildAssociationPolicy> childAssociationPolicy;
    QStringList childAssociationAllowlist;
    QStringList childNodes;
    std::optional<quint32> childNodesMax;
    QStringList collections;
    QStringList contactJids;
    QString dataFormXslt;
    std::optional<bool> notificationsEnabled;
    std::optional<bool> includePayloads;
    QString description;
    std::optional<quint32> itemExpiry;
    std::optional<QXmppPubSubNodeConfig::ItemPublisher> notificationItemPublisher;
    QString language;
    QXmppPubSubNodeConfig::ItemLimit maxItems;
    std::optional<quint32> maxPayloadSize;
    std::optional<QXmppPubSubNodeConfig::NodeType> nodeType;
    std::optional<QXmppPubSubNodeConfig::NotificationType> notificationType;
    std::optional<bool> configNotificationsEnabled;
    std::optional<bool> deleteNotificationsEnabled;
    std::optional<bool> retractNotificationsEnabled;
    std::optional<bool> subNotificationsEnabled;
    std::optional<bool> persistItems;
    std::optional<bool> presenceBasedNotifications;
    std::optional<QXmppPubSubNodeConfig::PublishModel> publishModel;
    std::optional<bool> purgeWhenOffline;
    QStringList allowedRosterGroups;
    std::optional<QXmppPubSubNodeConfig::SendLastItemType> sendLastItem;
    std::optional<bool> temporarySubscriptions;
    std::optional<bool> allowSubscriptions;
    QString title;
    QString payloadType;
};

std::optional<QXmppPubSubNodeConfig::AccessModel> QXmppPubSubNodeConfig::accessModelFromString(const QString &string)
{
    if (string == u"open") {
        return Open;
    }
    if (string == u"presence") {
        return Presence;
    }
    if (string == u"roster") {
        return Roster;
    }
    if (string == u"authorize") {
        return Authorize;
    }
    if (string == u"whitelist") {
        return Allowlist;
    }
    return std::nullopt;
}

QString QXmppPubSubNodeConfig::accessModelToString(AccessModel model)
{
    switch (model) {
    case Open:
        return u"open"_s;
    case Presence:
        return u"presence"_s;
    case Roster:
        return u"roster"_s;
    case Authorize:
        return u"authorize"_s;
    case Allowlist:
        return u"whitelist"_s;
    }
    return {};
}

std::optional<QXmppPubSubNodeConfig::PublishModel> QXmppPubSubNodeConfig::publishModelFromString(const QString &string)
{
    if (string == u"publishers") {
        return Publishers;
    }
    if (string == u"subscribers") {
        return Subscribers;
    }
    if (string == u"open") {
        return Anyone;
    }
    return std::nullopt;
}

QString QXmppPubSubNodeConfig::publishModelToString(QXmppPubSubNodeConfig::PublishModel model)
{
    switch (model) {
    case Publishers:
        return u"publishers"_s;
    case Subscribers:
        return u"subscribers"_s;
    case Anyone:
        return u"open"_s;
    }
    return {};
}

std::optional<QXmppPubSubNodeConfig::ChildAssociationPolicy> QXmppPubSubNodeConfig::childAssociatationPolicyFromString(const QString &string)
{
    if (string == u"all") {
        return ChildAssociationPolicy::All;
    }
    if (string == u"owners") {
        return ChildAssociationPolicy::Owners;
    }
    if (string == u"whitelist") {
        return ChildAssociationPolicy::Whitelist;
    }
    return std::nullopt;
}

QString QXmppPubSubNodeConfig::childAssociationPolicyToString(QXmppPubSubNodeConfig::ChildAssociationPolicy policy)
{
    switch (policy) {
    case ChildAssociationPolicy::All:
        return u"all"_s;
    case ChildAssociationPolicy::Owners:
        return u"owners"_s;
    case ChildAssociationPolicy::Whitelist:
        return u"whitelist"_s;
    }
    return {};
}

std::optional<QXmppPubSubNodeConfig::ItemPublisher> QXmppPubSubNodeConfig::itemPublisherFromString(const QString &string)
{
    if (string == u"owner") {
        return NodeOwner;
    }
    if (string == u"publisher") {
        return Publisher;
    }
    return std::nullopt;
}

QString QXmppPubSubNodeConfig::itemPublisherToString(ItemPublisher publisher)
{
    switch (publisher) {
    case NodeOwner:
        return u"owner"_s;
    case Publisher:
        return u"publisher"_s;
    }
    return {};
}

std::optional<QXmppPubSubNodeConfig::NodeType> QXmppPubSubNodeConfig::nodeTypeFromString(const QString &string)
{
    if (string == u"leaf") {
        return Leaf;
    }
    if (string == u"collection") {
        return Collection;
    }
    return std::nullopt;
}

QString QXmppPubSubNodeConfig::nodeTypeToString(NodeType type)
{
    switch (type) {
    case Leaf:
        return u"leaf"_s;
    case Collection:
        return u"collection"_s;
    }
    return {};
}

std::optional<QXmppPubSubNodeConfig::NotificationType> QXmppPubSubNodeConfig::notificationTypeFromString(const QString &string)
{
    if (string == u"normal") {
        return Normal;
    }
    if (string == u"headline") {
        return Headline;
    }
    return std::nullopt;
}

QString QXmppPubSubNodeConfig::notificationTypeToString(NotificationType type)
{
    switch (type) {
    case Normal:
        return u"normal"_s;
    case Headline:
        return u"headline"_s;
    }
    return {};
}

std::optional<QXmppPubSubNodeConfig::SendLastItemType> QXmppPubSubNodeConfig::sendLastItemTypeFromString(const QString &string)
{
    if (string == u"never") {
        return Never;
    }
    if (string == u"on_sub") {
        return OnSubscription;
    }
    if (string == u"on_sub_and_presence") {
        return OnSubscriptionAndPresence;
    }
    return std::nullopt;
}

QString QXmppPubSubNodeConfig::sendLastItemTypeToString(SendLastItemType type)
{
    switch (type) {
    case Never:
        return u"never"_s;
    case OnSubscription:
        return u"on_sub"_s;
    case OnSubscriptionAndPresence:
        return u"on_sub_and_presence"_s;
    }
    return {};
}

std::optional<QXmppPubSubNodeConfig> QXmppPubSubNodeConfig::fromDataForm(const QXmppDataForm &form)
{
    if (form.formType() != NODE_CONFIG_FORM_TYPE) {
        return std::nullopt;
    }

    QXmppPubSubNodeConfig nodeConfig;
    if (QXmppDataFormBase::fromDataForm(form, nodeConfig)) {
        return nodeConfig;
    }
    return std::nullopt;
}

QXmppPubSubNodeConfig::QXmppPubSubNodeConfig()
    : d(new QXmppPubSubNodeConfigPrivate)
{
}

QXmppPubSubNodeConfig::QXmppPubSubNodeConfig(const QXmppPubSubNodeConfig &) = default;
QXmppPubSubNodeConfig::QXmppPubSubNodeConfig(QXmppPubSubNodeConfig &&) = default;
QXmppPubSubNodeConfig::~QXmppPubSubNodeConfig() = default;
QXmppPubSubNodeConfig &QXmppPubSubNodeConfig::operator=(const QXmppPubSubNodeConfig &) = default;
QXmppPubSubNodeConfig &QXmppPubSubNodeConfig::operator=(QXmppPubSubNodeConfig &&) = default;

std::optional<QXmppPubSubNodeConfig::AccessModel> QXmppPubSubNodeConfig::accessModel() const
{
    return d->accessModel;
}

void QXmppPubSubNodeConfig::setAccessModel(std::optional<AccessModel> accessModel)
{
    d->accessModel = accessModel;
}

QString QXmppPubSubNodeConfig::bodyXslt() const
{
    return d->bodyXslt;
}

void QXmppPubSubNodeConfig::setBodyXslt(const QString &bodyXslt)
{
    d->bodyXslt = bodyXslt;
}

std::optional<QXmppPubSubNodeConfig::ChildAssociationPolicy> QXmppPubSubNodeConfig::childAssociationPolicy() const
{
    return d->childAssociationPolicy;
}

void QXmppPubSubNodeConfig::setChildAssociationPolicy(std::optional<ChildAssociationPolicy> childAssociationPolicy)
{
    d->childAssociationPolicy = childAssociationPolicy;
}

QStringList QXmppPubSubNodeConfig::childAssociationAllowlist() const
{
    return d->childAssociationAllowlist;
}

void QXmppPubSubNodeConfig::setChildAssociationAllowlist(const QStringList &childAssociationWhitelist)
{
    d->childAssociationAllowlist = childAssociationWhitelist;
}

QStringList QXmppPubSubNodeConfig::childNodes() const
{
    return d->childNodes;
}

void QXmppPubSubNodeConfig::setChildNodes(const QStringList &childNodes)
{
    d->childNodes = childNodes;
}

std::optional<quint32> QXmppPubSubNodeConfig::childNodesMax() const
{
    return d->childNodesMax;
}

void QXmppPubSubNodeConfig::setChildNodesMax(std::optional<quint32> childNodesMax)
{
    d->childNodesMax = childNodesMax;
}

QStringList QXmppPubSubNodeConfig::collections() const
{
    return d->collections;
}

void QXmppPubSubNodeConfig::setCollections(const QStringList &collections)
{
    d->collections = collections;
}

QStringList QXmppPubSubNodeConfig::contactJids() const
{
    return d->contactJids;
}

void QXmppPubSubNodeConfig::setContactJids(const QStringList &contactJids)
{
    d->contactJids = contactJids;
}

QString QXmppPubSubNodeConfig::dataFormXslt() const
{
    return d->dataFormXslt;
}

void QXmppPubSubNodeConfig::setDataFormXslt(const QString &dataFormXslt)
{
    d->dataFormXslt = dataFormXslt;
}

std::optional<bool> QXmppPubSubNodeConfig::notificationsEnabled() const
{
    return d->notificationsEnabled;
}

void QXmppPubSubNodeConfig::setNotificationsEnabled(std::optional<bool> notificationsEnabled)
{
    d->notificationsEnabled = notificationsEnabled;
}

std::optional<bool> QXmppPubSubNodeConfig::includePayloads() const
{
    return d->includePayloads;
}

void QXmppPubSubNodeConfig::setIncludePayloads(std::optional<bool> includePayloads)
{
    d->includePayloads = includePayloads;
}

QString QXmppPubSubNodeConfig::description() const
{
    return d->description;
}

void QXmppPubSubNodeConfig::setDescription(const QString &description)
{
    d->description = description;
}

std::optional<quint32> QXmppPubSubNodeConfig::itemExpiry() const
{
    return d->itemExpiry;
}

void QXmppPubSubNodeConfig::setItemExpiry(std::optional<quint32> itemExpiry)
{
    d->itemExpiry = itemExpiry;
}

std::optional<QXmppPubSubNodeConfig::ItemPublisher> QXmppPubSubNodeConfig::notificationItemPublisher() const
{
    return d->notificationItemPublisher;
}

void QXmppPubSubNodeConfig::setNotificationItemPublisher(std::optional<ItemPublisher> notificationItemPublisher)
{
    d->notificationItemPublisher = notificationItemPublisher;
}

QString QXmppPubSubNodeConfig::language() const
{
    return d->language;
}

void QXmppPubSubNodeConfig::setLanguage(const QString &language)
{
    d->language = language;
}

QXmppPubSubNodeConfig::ItemLimit QXmppPubSubNodeConfig::maxItems() const
{
    return d->maxItems;
}

void QXmppPubSubNodeConfig::setMaxItems(ItemLimit maxItems)
{
    d->maxItems = maxItems;
}

std::optional<quint32> QXmppPubSubNodeConfig::maxPayloadSize() const
{
    return d->maxPayloadSize;
}

void QXmppPubSubNodeConfig::setMaxPayloadSize(std::optional<quint32> maxPayloadSize)
{
    d->maxPayloadSize = maxPayloadSize;
}

std::optional<QXmppPubSubNodeConfig::NodeType> QXmppPubSubNodeConfig::nodeType() const
{
    return d->nodeType;
}

void QXmppPubSubNodeConfig::setNodeType(std::optional<NodeType> nodeType)
{
    d->nodeType = nodeType;
}

std::optional<QXmppPubSubNodeConfig::NotificationType> QXmppPubSubNodeConfig::notificationType() const
{
    return d->notificationType;
}

void QXmppPubSubNodeConfig::setNotificationType(std::optional<QXmppPubSubNodeConfig::NotificationType> notificationType)
{
    d->notificationType = notificationType;
}

std::optional<bool> QXmppPubSubNodeConfig::configNotificationsEnabled() const
{
    return d->configNotificationsEnabled;
}

void QXmppPubSubNodeConfig::setConfigNotificationsEnabled(std::optional<bool> configNotificationsEnabled)
{
    d->configNotificationsEnabled = configNotificationsEnabled;
}

std::optional<bool> QXmppPubSubNodeConfig::deleteNotificationsEnabled() const
{
    return d->deleteNotificationsEnabled;
}

void QXmppPubSubNodeConfig::setDeleteNotificationsEnabled(std::optional<bool> nodeDeleteNotificationsEnabled)
{
    d->deleteNotificationsEnabled = nodeDeleteNotificationsEnabled;
}

std::optional<bool> QXmppPubSubNodeConfig::retractNotificationsEnabled() const
{
    return d->retractNotificationsEnabled;
}

void QXmppPubSubNodeConfig::setRetractNotificationsEnabled(std::optional<bool> retractNotificationsEnabled)
{
    d->retractNotificationsEnabled = retractNotificationsEnabled;
}

std::optional<bool> QXmppPubSubNodeConfig::subNotificationsEnabled() const
{
    return d->subNotificationsEnabled;
}

void QXmppPubSubNodeConfig::setSubNotificationsEnabled(std::optional<bool> subNotificationsEnabled)
{
    d->subNotificationsEnabled = subNotificationsEnabled;
}

std::optional<bool> QXmppPubSubNodeConfig::persistItems() const
{
    return d->persistItems;
}

void QXmppPubSubNodeConfig::setPersistItems(std::optional<bool> persistItems)
{
    d->persistItems = persistItems;
}

std::optional<bool> QXmppPubSubNodeConfig::presenceBasedNotifications() const
{
    return d->presenceBasedNotifications;
}

void QXmppPubSubNodeConfig::setPresenceBasedNotifications(std::optional<bool> presenceBasedNotifications)
{
    d->presenceBasedNotifications = presenceBasedNotifications;
}

std::optional<QXmppPubSubNodeConfig::PublishModel> QXmppPubSubNodeConfig::publishModel() const
{
    return d->publishModel;
}

void QXmppPubSubNodeConfig::setPublishModel(std::optional<PublishModel> publishModel)
{
    d->publishModel = publishModel;
}

std::optional<bool> QXmppPubSubNodeConfig::purgeWhenOffline() const
{
    return d->purgeWhenOffline;
}

void QXmppPubSubNodeConfig::setPurgeWhenOffline(std::optional<bool> purgeWhenOffline)
{
    d->purgeWhenOffline = purgeWhenOffline;
}

QStringList QXmppPubSubNodeConfig::allowedRosterGroups() const
{
    return d->allowedRosterGroups;
}

void QXmppPubSubNodeConfig::setAllowedRosterGroups(const QStringList &allowedRosterGroups)
{
    d->allowedRosterGroups = allowedRosterGroups;
}

std::optional<QXmppPubSubNodeConfig::SendLastItemType> QXmppPubSubNodeConfig::sendLastItem() const
{
    return d->sendLastItem;
}

void QXmppPubSubNodeConfig::setSendLastItem(std::optional<QXmppPubSubNodeConfig::SendLastItemType> sendLastItem)
{
    d->sendLastItem = sendLastItem;
}

std::optional<bool> QXmppPubSubNodeConfig::temporarySubscriptions() const
{
    return d->temporarySubscriptions;
}

void QXmppPubSubNodeConfig::setTemporarySubscriptions(std::optional<bool> temporarySubscriptions)
{
    d->temporarySubscriptions = temporarySubscriptions;
}

std::optional<bool> QXmppPubSubNodeConfig::allowSubscriptions() const
{
    return d->allowSubscriptions;
}

void QXmppPubSubNodeConfig::setAllowSubscriptions(std::optional<bool> allowSubscriptions)
{
    d->allowSubscriptions = allowSubscriptions;
}

QString QXmppPubSubNodeConfig::title() const
{
    return d->title;
}

void QXmppPubSubNodeConfig::setTitle(const QString &title)
{
    d->title = title;
}

QString QXmppPubSubNodeConfig::payloadType() const
{
    return d->payloadType;
}

void QXmppPubSubNodeConfig::setPayloadType(const QString &payloadType)
{
    d->payloadType = payloadType;
}

QString QXmppPubSubNodeConfig::formType() const
{
    return NODE_CONFIG_FORM_TYPE;
}

bool QXmppPubSubNodeConfig::parseField(const QXmppDataForm::Field &field)
{
    // ignore hidden fields
    using Type = QXmppDataForm::Field::Type;
    if (field.type() == Type::HiddenField) {
        return false;
    }

    const auto key = field.key();
    const auto value = field.value();

    if (key == ACCESS_MODEL) {
        d->accessModel = accessModelFromString(field.value().toString());
    } else if (key == BODY_XSLT) {
        d->bodyXslt = value.toString();
    } else if (key == CHILD_ASSOCIATION_POLICY) {
        d->childAssociationPolicy = childAssociatationPolicyFromString(value.toString());
    } else if (key == CHILD_ASSOCIATION_ALLOWLIST) {
        d->childAssociationAllowlist = value.toStringList();
    } else if (key == CHILD_NODES) {
        d->childNodes = value.toStringList();
    } else if (key == CHILD_NODES_MAX) {
        d->childNodesMax = parseUInt(value);
    } else if (key == COLLECTIONS) {
        d->collections = value.toStringList();
    } else if (key == CONTACT_JIDS) {
        d->contactJids = value.toStringList();
    } else if (key == DATA_FORM_XSLT) {
        d->dataFormXslt = value.toString();
    } else if (key == NOTIFICATIONS_ENABLED) {
        d->notificationsEnabled = parseBool(value);
    } else if (key == INCLUDE_PAYLOADS) {
        d->includePayloads = parseBool(value);
    } else if (key == DESCRIPTION) {
        d->description = value.toString();
    } else if (key == ITEM_EXPIRY) {
        d->itemExpiry = parseUInt(value);
    } else if (key == NOTIFICATION_ITEM_PUBLISHER) {
        d->notificationItemPublisher = itemPublisherFromString(value.toString());
    } else if (key == LANGUAGE) {
        d->language = value.toString();
    } else if (key == MAX_ITEMS) {
        bool ok = false;
        if (const auto maxItems = value.toULongLong(&ok); ok) {
            d->maxItems = maxItems;
        } else if (value.type() == QVariant::String && value.toString() == u"max") {
            d->maxItems = Max();
        } else {
            d->maxItems = Unset();
        }
    } else if (key == MAX_PAYLOAD_SIZE) {
        d->maxPayloadSize = parseUInt(value);
    } else if (key == NODE_TYPE) {
        d->nodeType = nodeTypeFromString(value.toString());
    } else if (key == NOTIFICATION_TYPE) {
        d->notificationType = notificationTypeFromString(value.toString());
    } else if (key == CONFIG_NOTIFICATIONS_ENABLED) {
        d->configNotificationsEnabled = parseBool(value);
    } else if (key == NODE_DELETE_NOTIFICATIONS_ENABLED) {
        d->deleteNotificationsEnabled = parseBool(value);
    } else if (key == RETRACT_NOTIFICATIONS_ENABLED) {
        d->retractNotificationsEnabled = parseBool(value);
    } else if (key == SUB_NOTIFICATIONS_ENABLED) {
        d->subNotificationsEnabled = parseBool(value);
    } else if (key == PERSIST_ITEMS) {
        d->persistItems = parseBool(value);
    } else if (key == PRESENCE_BASED_NOTIFICATIONS) {
        d->presenceBasedNotifications = parseBool(value);
    } else if (key == PUBLISH_MODEL) {
        d->publishModel = publishModelFromString(value.toString());
    } else if (key == PURGE_WHEN_OFFLINE) {
        d->purgeWhenOffline = parseBool(value);
    } else if (key == ALLOWED_ROSTER_GROUPS) {
        d->allowedRosterGroups = value.toStringList();
    } else if (key == SEND_LAST_ITEM) {
        d->sendLastItem = sendLastItemTypeFromString(value.toString());
    } else if (key == TEMPORARY_SUBSCRIPTIONS) {
        d->temporarySubscriptions = parseBool(value);
    } else if (key == ALLOW_SUBSCRIPTIONS) {
        d->allowSubscriptions = parseBool(value);
    } else if (key == TITLE) {
        d->title = value.toString();
    } else if (key == PAYLOAD_TYPE) {
        d->payloadType = value.toString();
    } else {
        return false;
    }
    return true;
}

void QXmppPubSubNodeConfig::serializeForm(QXmppDataForm &form) const
{
    using Type = QXmppDataForm::Field::Type;

    serializeOptional(form,
                      Type::ListSingleField,
                      ACCESS_MODEL,
                      d->accessModel, accessModelToString);
    serializeNullable(form,
                      Type::TextSingleField,
                      BODY_XSLT,
                      d->bodyXslt);
    serializeOptional(form,
                      Type::ListSingleField,
                      CHILD_ASSOCIATION_POLICY,
                      d->childAssociationPolicy, childAssociationPolicyToString);
    serializeEmptyable(form,
                       Type::TextMultiField,
                       CHILD_ASSOCIATION_ALLOWLIST,
                       d->childAssociationAllowlist);
    serializeEmptyable(form,
                       Type::TextMultiField,
                       CHILD_NODES,
                       d->childNodes);
    serializeOptionalNumber(form, Type::TextSingleField,
                            CHILD_NODES_MAX,
                            d->childNodesMax);
    serializeEmptyable(form,
                       Type::TextMultiField,
                       COLLECTIONS,
                       d->collections);
    serializeEmptyable(form,
                       Type::JidMultiField,
                       CONTACT_JIDS,
                       d->contactJids);
    serializeNullable(form,
                      Type::TextSingleField,
                      DATA_FORM_XSLT,
                      d->dataFormXslt);
    serializeOptional(form,
                      Type::BooleanField,
                      NOTIFICATIONS_ENABLED,
                      d->notificationsEnabled);
    serializeOptional(form,
                      Type::BooleanField,
                      INCLUDE_PAYLOADS,
                      d->includePayloads);
    serializeNullable(form,
                      Type::TextSingleField,
                      DESCRIPTION,
                      d->description);
    serializeOptionalNumber(form,
                            Type::TextSingleField,
                            ITEM_EXPIRY,
                            d->itemExpiry);
    serializeOptional(form,
                      Type::ListSingleField,
                      NOTIFICATION_ITEM_PUBLISHER,
                      d->notificationItemPublisher, itemPublisherToString);
    serializeNullable(form,
                      Type::TextSingleField,
                      LANGUAGE,
                      d->language);
    std::visit(overloaded {
                   [](Unset) {},
                   [&](uint64_t value) {
                       serializeValue(form, Type::TextSingleField, MAX_ITEMS, QString::number(value));
                   },
                   [&](Max) {
                       serializeValue(form, Type::TextSingleField, MAX_ITEMS, u"max"_s);
                   } },
               d->maxItems);
    serializeOptionalNumber(form,
                            Type::TextSingleField,
                            MAX_PAYLOAD_SIZE,
                            d->maxPayloadSize);
    serializeOptional(form,
                      Type::ListSingleField,
                      NODE_TYPE,
                      d->nodeType, nodeTypeToString);
    serializeOptional(form,
                      Type::ListSingleField,
                      NOTIFICATION_TYPE,
                      d->notificationType, notificationTypeToString);
    serializeOptional(form,
                      Type::BooleanField,
                      CONFIG_NOTIFICATIONS_ENABLED,
                      d->configNotificationsEnabled);
    serializeOptional(form,
                      Type::BooleanField,
                      NODE_DELETE_NOTIFICATIONS_ENABLED,
                      d->deleteNotificationsEnabled);
    serializeOptional(form,
                      Type::BooleanField,
                      RETRACT_NOTIFICATIONS_ENABLED,
                      d->retractNotificationsEnabled);
    serializeOptional(form,
                      Type::BooleanField,
                      SUB_NOTIFICATIONS_ENABLED,
                      d->subNotificationsEnabled);
    serializeOptional(form,
                      Type::BooleanField,
                      PERSIST_ITEMS,
                      d->persistItems);
    serializeOptional(form,
                      Type::BooleanField,
                      PRESENCE_BASED_NOTIFICATIONS,
                      d->presenceBasedNotifications);
    serializeOptional(form,
                      Type::ListSingleField,
                      PUBLISH_MODEL,
                      d->publishModel, publishModelToString);
    serializeOptional(form,
                      Type::BooleanField,
                      PURGE_WHEN_OFFLINE,
                      d->purgeWhenOffline);
    serializeEmptyable(form,
                       Type::ListMultiField,
                       ALLOWED_ROSTER_GROUPS,
                       d->allowedRosterGroups);
    serializeOptional(form,
                      Type::ListSingleField,
                      SEND_LAST_ITEM,
                      d->sendLastItem, sendLastItemTypeToString);
    serializeOptional(form,
                      Type::BooleanField,
                      TEMPORARY_SUBSCRIPTIONS,
                      d->temporarySubscriptions);
    serializeOptional(form,
                      Type::BooleanField,
                      ALLOW_SUBSCRIPTIONS,
                      d->allowSubscriptions);
    serializeNullable(form,
                      Type::TextSingleField,
                      TITLE,
                      d->title);
    serializeNullable(form,
                      Type::TextSingleField,
                      PAYLOAD_TYPE,
                      d->payloadType);
}

QString QXmppPubSubPublishOptions::formType() const
{
    return PUBLISH_OPTIONS_FORM_TYPE;
}
