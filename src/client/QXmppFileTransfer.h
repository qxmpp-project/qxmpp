// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPFILETRANSFER_H
#define QXMPPFILETRANSFER_H

#include "QXmppGlobal.h"

#include <QObject>

class QXMPP_EXPORT QXmppFileTransfer : public QObject
{
    Q_OBJECT
    /// Progress of the file transfer between 0.0 and 1.0.
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged)

public:
    /// Returns the current progress between 0.0 and 1.0.
    virtual float progress() const = 0;
    virtual void cancel() = 0;
    virtual bool isFinished() const = 0;
    virtual quint64 bytesTransferred() const = 0;
    virtual quint64 bytesTotal() const = 0;

    // TODO consider adding speed getter

    Q_SIGNAL void progressChanged();
};

#endif  // QXMPPFILETRANSFER_H
