// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
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
    using Result = std::variant<QXmpp::Success, QXmpp::Cancelled, QXmppError>;
    Q_SIGNAL void finished(QXmppDownload::Result);
};

Q_DECLARE_METATYPE(QXmppDownload::Result);

#endif  // QXMPPDOWNLOAD_H
