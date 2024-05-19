// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppError.h"

#include "QXmppStanza.h"

#include "StringLiterals.h"

#include <QFileDevice>
#include <QNetworkReply>

///
/// \struct QXmppError
///
/// Generic error class holding a description and a more specific error object. The specific error
/// usually is something like a QXmppStanza::Error or an enum.
///
/// \since QXmpp 1.5
///

///
/// \var QXmppError::description
///
/// Human readable description of the error.
///

///
/// \var QXmppError::error
///
/// More specific details on the error. It may be of any type. Functions returning QXmppError
/// should tell you which types are used.
///

///
/// Constructs a QXmppError from an QIODevice
///
/// It tries to cast the IO device to different known IO devices to get a useful more specific
/// error, i.e. it returns a QXmppError with QFileDevice::FileError for QFileDevices.
///
QXmppError QXmppError::fromIoDevice(const QIODevice &device)
{
    if (const auto *file = dynamic_cast<const QFileDevice *>(&device)) {
        return QXmppError::fromFileDevice(*file);
    }
    if (const auto *networkReply = dynamic_cast<const QNetworkReply *>(&device)) {
        return QXmppError::fromNetworkReply(*networkReply);
    }
    return QXmppError { device.errorString(), std::any() };
}

///
/// \brief Constructs a QXmppError from a QNetworkReply
///
/// It creates a QXmppError with the error string and network error from the reply.
///
QXmppError QXmppError::fromNetworkReply(const QNetworkReply &reply)
{
    return { reply.errorString(), reply.error() };
}

///
/// \brief Constructs a QXmppError from a QFileDevice
///
/// It creates a QXmppError with the error string and a QFileDevice::FileError.
///
QXmppError QXmppError::fromFileDevice(const QFileDevice &file)
{
    return QXmppError { file.errorString(), file.error() };
}

///
/// Returns whether the error is a QNetworkReply::NetworkError.
///
bool QXmppError::isFileError() const
{
    return holdsType<QFileDevice::FileError>();
}

///
/// Returns whether the error is a QNetworkReply::NetworkError.
///
bool QXmppError::isNetworkError() const
{
    return holdsType<QNetworkReply::NetworkError>();
}

///
/// Returns whether the error is a QXmppStanza::Error.
///
bool QXmppError::isStanzaError() const
{
    return holdsType<QXmppStanza::Error>();
}

///
/// \fn QXmppError::holdsType()
///
/// Returns true if the error is of type T.
///

///
/// \fn QXmppError::value()
///
/// Copies the value if it has type T, returns empty optional otherwise.
///

///
/// \fn QXmppError::takeValue()
///
/// Moves out the value if it has type T, leaves the stored error intact and returns an empty
/// optional otherwise.
///
