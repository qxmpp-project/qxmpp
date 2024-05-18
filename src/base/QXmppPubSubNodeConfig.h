// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUBSUBNODECONFIG_H
#define QXMPPPUBSUBNODECONFIG_H

#include "QXmppDataForm.h"
#include "QXmppDataFormBase.h"

#include <variant>

class QXmppPubSubNodeConfigPrivate;

class QXMPP_EXPORT QXmppPubSubNodeConfig : public QXmppExtensibleDataFormBase
{
public:
    struct Unset { };
    struct Max { };
    using ItemLimit = std::variant<Unset, uint64_t, Max>;

    enum AccessModel : uint8_t {
        Open,
        Presence,
        Roster,
        Authorize,
        Allowlist
    };
    static std::optional<AccessModel> accessModelFromString(const QString &);
    static QString accessModelToString(AccessModel);

    enum PublishModel : uint8_t {
        Publishers,
        Subscribers,
        Anyone
    };
    static std::optional<PublishModel> publishModelFromString(const QString &);
    static QString publishModelToString(PublishModel);

    enum class ChildAssociationPolicy : uint8_t {
        All,
        Owners,
        Whitelist
    };
    static std::optional<ChildAssociationPolicy> childAssociatationPolicyFromString(const QString &);
    static QString childAssociationPolicyToString(ChildAssociationPolicy);

    enum ItemPublisher : uint8_t {
        NodeOwner,
        Publisher
    };
    static std::optional<ItemPublisher> itemPublisherFromString(const QString &);
    static QString itemPublisherToString(ItemPublisher);

    enum NodeType : uint8_t {
        Leaf,
        Collection
    };
    static std::optional<NodeType> nodeTypeFromString(const QString &);
    static QString nodeTypeToString(NodeType);

    enum NotificationType : uint8_t {
        Normal,
        Headline
    };
    static std::optional<NotificationType> notificationTypeFromString(const QString &);
    static QString notificationTypeToString(NotificationType);

    enum SendLastItemType : uint8_t {
        Never,
        OnSubscription,
        OnSubscriptionAndPresence
    };
    static std::optional<SendLastItemType> sendLastItemTypeFromString(const QString &);
    static QString sendLastItemTypeToString(SendLastItemType);

    static std::optional<QXmppPubSubNodeConfig> fromDataForm(const QXmppDataForm &form);

    QXmppPubSubNodeConfig();
    QXmppPubSubNodeConfig(const QXmppPubSubNodeConfig &);
    QXmppPubSubNodeConfig(QXmppPubSubNodeConfig &&);
    ~QXmppPubSubNodeConfig() override;

    QXmppPubSubNodeConfig &operator=(const QXmppPubSubNodeConfig &);
    QXmppPubSubNodeConfig &operator=(QXmppPubSubNodeConfig &&);

    std::optional<AccessModel> accessModel() const;
    void setAccessModel(std::optional<AccessModel> accessModel);

    QString bodyXslt() const;
    void setBodyXslt(const QString &bodyXslt);

    std::optional<ChildAssociationPolicy> childAssociationPolicy() const;
    void setChildAssociationPolicy(std::optional<ChildAssociationPolicy> childAssociationPolicy);

    QStringList childAssociationAllowlist() const;
    void setChildAssociationAllowlist(const QStringList &childAssociationWhitelist);

    QStringList childNodes() const;
    void setChildNodes(const QStringList &childNodes);

    std::optional<quint32> childNodesMax() const;
    void setChildNodesMax(std::optional<quint32> childNodesMax);

    QStringList collections() const;
    void setCollections(const QStringList &collections);

    QStringList contactJids() const;
    void setContactJids(const QStringList &contactJids);

    QString dataFormXslt() const;
    void setDataFormXslt(const QString &dataFormXslt);

    std::optional<bool> notificationsEnabled() const;
    void setNotificationsEnabled(std::optional<bool> notificationsEnabled);

    std::optional<bool> includePayloads() const;
    void setIncludePayloads(std::optional<bool> includePayloads);

    QString description() const;
    void setDescription(const QString &description);

    std::optional<quint32> itemExpiry() const;
    void setItemExpiry(std::optional<quint32> itemExpiry);

    std::optional<ItemPublisher> notificationItemPublisher() const;
    void setNotificationItemPublisher(std::optional<ItemPublisher> notificationItemPublisher);

    QString language() const;
    void setLanguage(const QString &language);

    ItemLimit maxItems() const;
    void setMaxItems(ItemLimit maxItems);
    inline void resetMaxItems() { setMaxItems(Unset()); }

    std::optional<quint32> maxPayloadSize() const;
    void setMaxPayloadSize(std::optional<quint32> maxPayloadSize);

    std::optional<NodeType> nodeType() const;
    void setNodeType(std::optional<NodeType> nodeType);

    std::optional<QXmppPubSubNodeConfig::NotificationType> notificationType() const;
    void setNotificationType(std::optional<QXmppPubSubNodeConfig::NotificationType> notificationType);

    std::optional<bool> configNotificationsEnabled() const;
    void setConfigNotificationsEnabled(std::optional<bool> configNotificationsEnabled);

    std::optional<bool> deleteNotificationsEnabled() const;
    void setDeleteNotificationsEnabled(std::optional<bool> nodeDeleteNotificationsEnabled);

    std::optional<bool> retractNotificationsEnabled() const;
    void setRetractNotificationsEnabled(std::optional<bool> retractNotificationsEnabled);

    std::optional<bool> subNotificationsEnabled() const;
    void setSubNotificationsEnabled(std::optional<bool> subNotificationsEnabled);

    std::optional<bool> persistItems() const;
    void setPersistItems(std::optional<bool> persistItems);

    std::optional<bool> presenceBasedNotifications() const;
    void setPresenceBasedNotifications(std::optional<bool> presenceBasedNotifications);

    std::optional<PublishModel> publishModel() const;
    void setPublishModel(std::optional<PublishModel> publishModel);

    std::optional<bool> purgeWhenOffline() const;
    void setPurgeWhenOffline(std::optional<bool> purgeWhenOffline);

    QStringList allowedRosterGroups() const;
    void setAllowedRosterGroups(const QStringList &allowedRosterGroups);

    std::optional<SendLastItemType> sendLastItem() const;
    void setSendLastItem(std::optional<SendLastItemType> sendLastItem);

    std::optional<bool> temporarySubscriptions() const;
    void setTemporarySubscriptions(std::optional<bool> temporarySubscriptions);

    std::optional<bool> allowSubscriptions() const;
    void setAllowSubscriptions(std::optional<bool> allowSubscriptions);

    QString title() const;
    void setTitle(const QString &title);

    QString payloadType() const;
    void setPayloadType(const QString &payloadType);

protected:
    QString formType() const override;
    bool parseField(const QXmppDataForm::Field &) override;
    void serializeForm(QXmppDataForm &) const override;

private:
    QSharedDataPointer<QXmppPubSubNodeConfigPrivate> d;
};

class QXMPP_EXPORT QXmppPubSubPublishOptions : public QXmppPubSubNodeConfig
{
public:
    static std::optional<QXmppPubSubPublishOptions> fromDataForm(const QXmppDataForm &form);

protected:
    QString formType() const override;
};

Q_DECLARE_METATYPE(QXmppPubSubNodeConfig);
Q_DECLARE_METATYPE(QXmppPubSubPublishOptions);

#endif  // QXMPPPUBSUBNODECONFIG_H
