// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPFILESHARINGMANAGER_H
#define QXMPPFILESHARINGMANAGER_H

#include "QXmppBitsOfBinaryDataList.h"
#include "QXmppClientExtension.h"
#include "QXmppFileShare.h"
#include "QXmppFileSharingProvider.h"
#include "QXmppGlobal.h"

#include <functional>
#include <memory>
#include <typeindex>
#include <variant>

#include <QFuture>
#include <QMimeType>
#include <QSize>

class QIODevice;
class QXmppFileDownloadPrivate;
class QXmppFileMetadata;
class QXmppFileShare;
class QXmppFileSharingManagerPrivate;
class QXmppFileUploadPrivate;

class QXMPP_EXPORT QXmppFileUpload : public QObject
{
    Q_OBJECT
    /// Progress of the file upload between 0.0 and 1.0.
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged)
public:
    struct FileResult
    {
        QXmppFileShare fileShare;
        QXmppBitsOfBinaryDataList dataBlobs;
    };

    using Result = std::variant<FileResult, QXmpp::Cancelled, QXmppError>;

    ~QXmppFileUpload();

    float progress() const;
    Q_SIGNAL void progressChanged();

    void cancel();
    bool isFinished() const;
    quint64 bytesTransferred() const;
    quint64 bytesTotal() const;

    Q_SIGNAL void finished(QXmppFileUpload::Result);

private:
    QXmppFileUpload();

    void reportFinished(Result);

    std::unique_ptr<QXmppFileUploadPrivate> d;
    friend class QXmppFileSharingManager;
};

Q_DECLARE_METATYPE(QXmppFileUpload::Result);

class QXMPP_EXPORT QXmppFileDownload : public QObject
{
    Q_OBJECT
    /// Progress of the file download between 0.0 and 1.0.
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged)
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

    struct Downloaded
    {
        HashVerificationResult hashVerificationResult;
    };

    using Result = std::variant<Downloaded, QXmpp::Cancelled, QXmppError>;

    ~QXmppFileDownload();

    float progress() const;
    Q_SIGNAL void progressChanged();

    void cancel();
    bool isFinished() const;
    quint64 bytesTransferred() const;
    quint64 bytesTotal() const;

    Q_SIGNAL void finished(QXmppFileDownload::Result);

private:
    QXmppFileDownload();

    void reportProgress(quint64 bytesReceived, quint64 bytesTotal);
    void reportFinished(Result);

    std::unique_ptr<QXmppFileDownloadPrivate> d;
    friend class QXmppFileSharingManager;
};

Q_DECLARE_METATYPE(QXmppFileDownload::Result);

class QXMPP_EXPORT QXmppFileSharingManager : public QXmppClientExtension
{
public:
    struct MetadataThumbnail
    {
        uint32_t width;
        uint32_t height;
        QByteArray data;
        QMimeType mimeType;
    };

    struct MetadataGeneratorResult
    {
        std::optional<QSize> dimensions;
        std::optional<uint32_t> length;
        QVector<MetadataThumbnail> thumbnails;
        std::unique_ptr<QIODevice> dataDevice;
    };

    using MetadataGenerator = std::function<QFuture<std::shared_ptr<MetadataGeneratorResult>>(std::unique_ptr<QIODevice>)>;

    QXmppFileSharingManager();
    ~QXmppFileSharingManager();

    void setMetadataGenerator(MetadataGenerator &&generator);

    ///
    /// \brief Register a provider for automatic downloads
    /// \param manager A shared_ptr to a QXmppFileSharingProvider subclass
    /// The provider must define SourceType to the type of the accepted file source.
    ///
    template<typename ProviderType>
    void registerProvider(std::shared_ptr<ProviderType> manager)
    {
        std::type_index index(typeid(typename ProviderType::SourceType));
        internalRegisterProvider(index, manager);
    }

    std::shared_ptr<QXmppFileUpload> sendFile(std::shared_ptr<QXmppFileSharingProvider> provider,
                                              const QString &filePath,
                                              const std::optional<QString> &description = {});

    std::shared_ptr<QXmppFileDownload> downloadFile(const QXmppFileShare &fileShare,
                                                    std::unique_ptr<QIODevice> output);

private:
    friend class QXmppEncryptedFileSharingProvider;

    void internalRegisterProvider(std::type_index, std::shared_ptr<QXmppFileSharingProvider> provider);
    std::shared_ptr<QXmppFileSharingProvider> providerForSource(const std::any &source) const;

    std::unique_ptr<QXmppFileSharingManagerPrivate> d;
};

#endif  // QXMPPFILESHARINGMANAGER_H
