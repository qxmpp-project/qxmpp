// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPFILESHARINGMANAGER_H
#define QXMPPFILESHARINGMANAGER_H

#include "QXmppClientExtension.h"
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
class QXmppFileMetadata;
class QXmppFileShare;
class QXmppFileSharingManagerPrivate;

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

    std::shared_ptr<QXmppUpload> sendFile(std::shared_ptr<QXmppFileSharingProvider> provider,
                                          const QString &filePath,
                                          const std::optional<QString> &description = {});

    std::shared_ptr<QXmppDownload> downloadFile(const QXmppFileShare &fileShare,
                                                std::unique_ptr<QIODevice> output);

private:
    void internalRegisterProvider(std::type_index, std::shared_ptr<QXmppFileSharingProvider> provider);

    std::unique_ptr<QXmppFileSharingManagerPrivate> d;
};

#endif  // QXMPPFILESHARINGMANAGER_H
