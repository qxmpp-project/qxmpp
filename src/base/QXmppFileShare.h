// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPFILESHARE_H
#define QXMPPFILESHARE_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QUrl;
class QXmlStreamWriter;
class QXmppFileSharePrivate;
class QXmppFileMetadata;
class QXmppHttpFileSource;
class QXmppEncryptedFileSource;

class QXMPP_EXPORT QXmppFileShare
{
public:
    enum Disposition {
        Inline,
        Attachment,
    };

    QXmppFileShare();
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppFileShare)

    Disposition disposition() const;
    void setDisposition(Disposition);

    const QXmppFileMetadata &metadata() const;
    void setMetadata(const QXmppFileMetadata &);

    const QVector<QXmppHttpFileSource> &httpSources() const;
    void setHttpSources(const QVector<QXmppHttpFileSource> &newHttpSources);

    const QVector<QXmppEncryptedFileSource> &encryptedSources() const;
    void setEncryptedSourecs(const QVector<QXmppEncryptedFileSource> &newEncryptedSources);

    /// \cond
    bool parse(const QDomElement &el);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppFileSharePrivate> d;
};

#endif  // QXMPPFILESHARE_H
