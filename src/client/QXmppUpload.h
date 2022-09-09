// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPUPLOAD_H
#define QXMPPUPLOAD_H

#include "QXmppBitsOfBinaryDataList.h"
#include "QXmppError.h"
#include "QXmppFileMetadata.h"
#include "QXmppFileShare.h"
#include "QXmppFileTransfer.h"

#include <variant>

namespace QXmpp::Private {
using UploadResult = std::variant<std::any /* source */, QXmpp::Cancelled, QXmppError>;
}

class QXMPP_EXPORT QXmppUpload : public QXmppFileTransfer
{
    Q_OBJECT
public:
    struct FileResult
    {
        QXmppFileShare fileShare;
        QXmppBitsOfBinaryDataList dataBlobs;
    };

    using Result = std::variant<FileResult, QXmpp::Cancelled, QXmppError>;
    Q_SIGNAL void finished(QXmppUpload::Result);

protected:
    /// \cond
    Q_SIGNAL void uploadFinished(QXmpp::Private::UploadResult);
    /// \endcond

private:
    QXmppFileMetadata metadata;

    friend class QXmppFileSharingManager;
};

Q_DECLARE_METATYPE(QXmppUpload::Result);
Q_DECLARE_METATYPE(QXmpp::Private::UploadResult);

#endif  // QXMPPUPLOAD_H
