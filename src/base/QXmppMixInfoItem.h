// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMIXINFOITEM_H
#define QXMPPMIXINFOITEM_H

#include "QXmppDataForm.h"
#include "QXmppPubSubBaseItem.h"

class QXmppMixInfoItemPrivate;

class QXMPP_EXPORT QXmppMixInfoItem : public QXmppPubSubBaseItem
{
public:
    QXmppMixInfoItem();
    QXmppMixInfoItem(const QXmppMixInfoItem &);
    QXmppMixInfoItem(QXmppMixInfoItem &&);
    ~QXmppMixInfoItem();

    QXmppMixInfoItem &operator=(const QXmppMixInfoItem &);
    QXmppMixInfoItem &operator=(QXmppMixInfoItem &&);

    QXmppDataForm::Type formType() const;
    void setFormType(QXmppDataForm::Type formType);

    const QString &name() const;
    void setName(QString);

    const QString &description() const;
    void setDescription(QString);

    const QStringList &contactJids() const;
    void setContactJids(QStringList);

    static bool isItem(const QDomElement &itemElement);

protected:
    /// \cond
    void parsePayload(const QDomElement &payloadElement) override;
    void serializePayload(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppMixInfoItemPrivate> d;
};

Q_DECLARE_METATYPE(QXmppMixInfoItem)

#endif  // QXMPPMIXINFOITEM_H
