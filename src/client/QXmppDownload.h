// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPDOWNLOAD_H
#define QXMPPDOWNLOAD_H

#include "QXmppError.h"
#include "QXmppFileTransfer.h"

#include <variant>

class QXMPP_EXPORT QXmppDownload : public QXmppFileTransfer
{
    Q_OBJECT
public:
    enum HashVerificationResult {
        ///
        /// \brief File did not contain strong hashes (or no hashes at all) and no verification
        /// was done.
        ///
        /// This value is not used when a hash value did not match. In that case the whole file
        /// download returns an error.
        ///
        NoStrongHashes,
        /// \brief The file integrity could be proved using a strong hash algorithm.
        HashVerified,
    };

    struct Downloaded {
        HashVerificationResult hashVerificationResult;
    };

    using Result = std::variant<Downloaded, QXmpp::Cancelled, QXmppError>;
    Q_SIGNAL void finished(QXmppDownload::Result);
};

Q_DECLARE_METATYPE(QXmppDownload::Result);

#endif  // QXMPPDOWNLOAD_H
