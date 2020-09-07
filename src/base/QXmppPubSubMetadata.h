/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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

#ifndef QXMPPPUBSUBMETADATA_H
#define QXMPPPUBSUBMETADATA_H

#include "QXmppDataFormBase.h"
#include "QXmppPubSubNodeConfig.h"

#include <variant>

class QXmppPubSubMetadataPrivate;

class QXMPP_EXPORT QXmppPubSubMetadata : public QXmppExtensibleDataFormBase
{
public:
    struct Unset {
    };
    struct Max {
    };
    using ItemLimit = std::variant<Unset, quint64, Max>;

    QXmppPubSubMetadata();
    QXmppPubSubMetadata(const QXmppPubSubMetadata &);
    ~QXmppPubSubMetadata();

    QXmppPubSubMetadata &operator=(const QXmppPubSubMetadata &);

    QStringList contactJids() const;
    void setContactJids(const QStringList &contactJids);

    QDateTime creationDate() const;
    void setCreationDate(const QDateTime &creationDate);

    QString creatorJid() const;
    void setCreatorJid(const QString &creatorJid);

    QString description() const;
    void setDescription(const QString &description);

    QString language() const;
    void setLanguage(const QString &language);

    std::optional<QXmppPubSubNodeConfig::AccessModel> accessModel() const;
    void setAccessModel(std::optional<QXmppPubSubNodeConfig::AccessModel> accessModel);

    std::optional<QXmppPubSubNodeConfig::PublishModel> publishModel() const;
    void setPublishModel(std::optional<QXmppPubSubNodeConfig::PublishModel> publishModel);

    std::optional<quint64> numberOfSubscribers() const;
    void setNumberOfSubscribers(const std::optional<quint64> &numberOfSubscribers);

    QStringList ownerJids() const;
    void setOwnerJids(const QStringList &ownerJids);

    QStringList publisherJids() const;
    void setPublisherJids(const QStringList &publisherJids);

    QString title() const;
    void setTitle(const QString &title);

    QString type() const;
    void setType(const QString &type);

    ItemLimit maxItems() const;
    void setMaxItems(ItemLimit maxItems);

protected:
    QString formType() const override;
    bool parseField(const QXmppDataForm::Field &) override;
    void serializeForm(QXmppDataForm &) const override;

private:
    QSharedDataPointer<QXmppPubSubMetadataPrivate> d;
};

#endif  // QXMPPPUBSUBMETADATA_H
