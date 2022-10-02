// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSTICKERPACKITEM_H
#define QXMPPSTICKERPACKITEM_H

#include "QXmppPubSubItem.h"

#include <QVector>

class QXmppFileMetadata;
class QXmppHttpFileSource;
class QXmppEncryptedFileSource;

class QXmppStickerItemPrivate;

class QXMPP_EXPORT QXmppStickerItem
{
public:
    QXmppStickerItem();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppStickerItem)

    const QXmppFileMetadata &metadata() const;
    void setMetadata(const QXmppFileMetadata &metadata);

    const QVector<QXmppHttpFileSource> &httpSource() const;
    void setHttpSources(const QVector<QXmppHttpFileSource> &httpSources);

    const QVector<QXmppEncryptedFileSource> &encryptedSources() const;
    void setEncryptedSources(const QVector<QXmppEncryptedFileSource> &encryptedSources);

    /// \cond
    bool parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppStickerItemPrivate> d;
};

class QXmppStickerPackItemPrivate;

class QXMPP_EXPORT QXmppStickerPackItem : public QXmppPubSubItem
{
public:
    QXmppStickerPackItem();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppStickerPackItem)

    const QString &name() const;
    void setName(const QString &name);

    const QString &summary() const;
    void setSummary(const QString &summary);

    const QVector<QXmppStickerItem> &items() const;
    void setItems(const QVector<QXmppStickerItem> &items);

    void parsePayload(const QDomElement &payloadElement) override;
    void serializePayload(QXmlStreamWriter *writer) const override;

private:
    QSharedDataPointer<QXmppStickerPackItemPrivate> d;
};

#endif  // QXMPPSTICKERPACKITEM_H
