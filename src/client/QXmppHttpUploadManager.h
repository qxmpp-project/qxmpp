// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPHTTPUPLOADMANAGER_H
#define QXMPPHTTPUPLOADMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppError.h"

#include <variant>

#include <QUrl>

class QFileInfo;
class QNetworkAccessManager;
struct QXmppHttpUploadPrivate;
struct QXmppHttpUploadManagerPrivate;

class QXMPP_EXPORT QXmppHttpUpload : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(quint64 bytesSent READ bytesSent NOTIFY progressChanged)
    Q_PROPERTY(quint64 bytesTotal READ bytesTotal NOTIFY progressChanged)

public:
    struct Cancelled
    {
    };

    using Result = std::variant<QUrl, Cancelled, QXmppError>;

    ~QXmppHttpUpload();

    float progress() const;
    quint64 bytesSent() const;
    quint64 bytesTotal() const;

    void cancel();
    bool isFinished() const;
    std::optional<Result> result() const;

    Q_SIGNAL void progressChanged();
    Q_SIGNAL void finished(const QXmppHttpUpload::Result &result);

private:
    friend class QXmppHttpUploadManager;

    QXmppHttpUpload();

    std::unique_ptr<QXmppHttpUploadPrivate> d;
};

Q_DECLARE_METATYPE(QXmppHttpUpload::Result);

class QXMPP_EXPORT QXmppHttpUploadManager : public QXmppClientExtension
{
    Q_OBJECT
public:
    QXmppHttpUploadManager();
    explicit QXmppHttpUploadManager(QNetworkAccessManager *netManager);
    ~QXmppHttpUploadManager();

    std::shared_ptr<QXmppHttpUpload> uploadFile(QIODevice *data, const QString &filename, const QMimeType &mimeType, qint64 fileSize = -1, const QString &uploadServiceJid = {});
    std::shared_ptr<QXmppHttpUpload> uploadFile(const QFileInfo &fileInfo, const QString &filename = {}, const QString &uploadServiceJid = {});

private:
    std::unique_ptr<QXmppHttpUploadManagerPrivate> d;
};

#endif  // QXMPPHTTPUPLOADMANAGER_H
