// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPUPLOAD_H
#define QXMPPUPLOAD_H

#include "QXmppBitsOfBinaryDataList.h"
#include "QXmppError.h"
#include "QXmppFileShare.h"
#include "QXmppFileTransfer.h"

#include <variant>

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

private:
    friend class QXmppFileSharingManager;
};

Q_DECLARE_METATYPE(QXmppUpload::Result);

#endif  // QXMPPUPLOAD_H
