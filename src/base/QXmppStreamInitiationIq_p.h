// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppDataForm.h"
#include "QXmppIq.h"
#include "QXmppTransferManager.h"

#include <QDateTime>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.  It exists for the convenience
// of the QXmppTransferManager class.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

/// \cond
class QXMPP_AUTOTEST_EXPORT QXmppStreamInitiationIq : public QXmppIq
{
public:
    enum Profile {
        None = 0,
        FileTransfer
    };

    QXmppDataForm featureForm() const;
    void setFeatureForm(const QXmppDataForm &form);

    QXmppTransferFileInfo fileInfo() const;
    void setFileInfo(const QXmppTransferFileInfo &info);

    QString mimeType() const;
    void setMimeType(const QString &mimeType);

    QXmppStreamInitiationIq::Profile profile() const;
    void setProfile(QXmppStreamInitiationIq::Profile profile);

    QString siId() const;
    void setSiId(const QString &id);

    static bool isStreamInitiationIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;

private:
    QXmppDataForm m_featureForm;
    QXmppTransferFileInfo m_fileInfo;
    QString m_mimeType;
    Profile m_profile;
    QString m_siId;
};
/// \endcond
