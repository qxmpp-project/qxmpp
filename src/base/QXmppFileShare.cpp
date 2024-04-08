// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppFileShare.h"

#include "QXmppConstants_p.h"
#include "QXmppEncryptedFileSource.h"
#include "QXmppFileMetadata.h"
#include "QXmppHttpFileSource.h"
#include "QXmppUtils_p.h"

#include <optional>

#include <QDomElement>
#include <QUrl>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;
using Disposition = QXmppFileShare::Disposition;

namespace QXmpp::Private {

struct FileSources {
    static FileSources fromSourcesDom(const QDomElement &el);
    void innerToXml(QXmlStreamWriter *writer) const;

    QVector<QXmppHttpFileSource> httpSources;
    QVector<QXmppEncryptedFileSource> encryptedSources;
};

FileSources FileSources::fromSourcesDom(const QDomElement &el)
{
    FileSources sources;
    for (const auto &sourceEl : iterChildElements(el, u"url-data", ns_url_data)) {
        QXmppHttpFileSource source;
        if (source.parse(sourceEl)) {
            sources.httpSources.push_back(std::move(source));
        }
    }
    for (const auto &sourceEl : iterChildElements(el, u"encrypted", ns_esfs)) {
        QXmppEncryptedFileSource source;
        if (source.parse(sourceEl)) {
            sources.encryptedSources.push_back(std::move(source));
        }
    }
    return sources;
}

void FileSources::innerToXml(QXmlStreamWriter *writer) const
{
    for (const auto &source : httpSources) {
        source.toXml(writer);
    }
    for (const auto &source : encryptedSources) {
        source.toXml(writer);
    }
}

}  // namespace QXmpp::Private

static std::optional<Disposition> dispositionFromString(const QString &str)
{
    if (str == u"inline") {
        return Disposition::Inline;
    }
    if (str == u"attachment") {
        return Disposition::Attachment;
    }
    return {};
}

static QString dispositionToString(Disposition value)
{
    switch (value) {
    case Disposition::Inline:
        return QStringLiteral("inline");
    case Disposition::Attachment:
        return QStringLiteral("attachment");
    }
    Q_UNREACHABLE();
}

class QXmppFileSourcesAttachmentPrivate : public QSharedData
{
public:
    QString id;
    FileSources sources;
};

///
/// \class QXmppFileSourcesAttachment
///
/// Attachment of file sources to a previous file sharing element from \xep{0447, Stateless file
/// sharing}.
///
/// \since QXmpp 1.7
///

/// Default constructor
QXmppFileSourcesAttachment::QXmppFileSourcesAttachment()
    : d(new QXmppFileSourcesAttachmentPrivate)
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppFileSourcesAttachment)

///
/// Returns the ID of the referenced file-sharing element.
///
const QString &QXmppFileSourcesAttachment::id() const
{
    return d->id;
}

///
/// Sets the ID of the referenced file-sharing element.
///
void QXmppFileSourcesAttachment::setId(const QString &id)
{
    d->id = id;
}

///
/// Returns the HTTP sources for this file.
///
const QVector<QXmppHttpFileSource> &QXmppFileSourcesAttachment::httpSources() const
{
    return d->sources.httpSources;
}

///
/// Sets the HTTP sources for this file.
///
void QXmppFileSourcesAttachment::setHttpSources(const QVector<QXmppHttpFileSource> &newHttpSources)
{
    d->sources.httpSources = newHttpSources;
}

///
/// Returns the encrypted sources for this file.
///
const QVector<QXmppEncryptedFileSource> &QXmppFileSourcesAttachment::encryptedSources() const
{
    return d->sources.encryptedSources;
}

///
/// Sets the encrypted sources for this file.
///
void QXmppFileSourcesAttachment::setEncryptedSources(const QVector<QXmppEncryptedFileSource> &newEncryptedSources)
{
    d->sources.encryptedSources = newEncryptedSources;
}

std::optional<QXmppFileSourcesAttachment> QXmppFileSourcesAttachment::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"sources" || el.namespaceURI() != ns_sfs) {
        return {};
    }
    QXmppFileSourcesAttachment result;
    result.d->id = el.attribute(QStringLiteral("id"));
    result.d->sources = FileSources::fromSourcesDom(el);
    return result;
}

void QXmppFileSourcesAttachment::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("sources"));
    writer->writeDefaultNamespace(toString65(ns_sfs));
    writer->writeAttribute(QSL65("id"), d->id);
    d->sources.innerToXml(writer);
    writer->writeEndElement();
}

class QXmppFileSharePrivate : public QSharedData
{
public:
    QXmppFileMetadata metadata;
    QString id;
    FileSources sources;
    QXmppFileShare::Disposition disposition = Disposition::Inline;
};

///
/// \class QXmppFileShare
///
/// File sharing element from \xep{0447, Stateless file sharing}. Contains
/// metadata and source URLs.
///
/// \note jinglepub references are currently missing
///
/// \since QXmpp 1.5
///

///
/// \enum QXmppFileShare::Disposition
///
/// \brief Decides whether to display the file contents (e.g. an image) inline in the chat or as
/// a file.
///

/// Default constructor
QXmppFileShare::QXmppFileShare()
    : d(new QXmppFileSharePrivate)
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppFileShare)

/// Returns the disposition setting for this file.
QXmppFileShare::Disposition QXmppFileShare::disposition() const
{
    return d->disposition;
}

/// Sets the disposition setting for this file.
void QXmppFileShare::setDisposition(Disposition disp)
{
    d->disposition = disp;
}

///
/// Returns the ID of this file element.
///
/// This is useful for attaching sources to one of multiple files in a message.
///
/// \since QXmpp 1.7
///
const QString &QXmppFileShare::id() const
{
    return d->id;
}

///
/// Sets the ID of this file element.
///
/// This is useful for attaching sources to one of multiple files in a message.
///
/// \since QXmpp 1.7
///
void QXmppFileShare::setId(const QString &id)
{
    d->id = id;
}

/// Returns the metadata of the shared file.
const QXmppFileMetadata &QXmppFileShare::metadata() const
{
    return d->metadata;
}

/// Sets the metadata of the shared file.
void QXmppFileShare::setMetadata(const QXmppFileMetadata &metadata)
{
    d->metadata = metadata;
}

///
/// Returns the HTTP sources for this file.
///
const QVector<QXmppHttpFileSource> &QXmppFileShare::httpSources() const
{
    return d->sources.httpSources;
}

///
/// Sets the HTTP sources for this file.
///
void QXmppFileShare::setHttpSources(const QVector<QXmppHttpFileSource> &newHttpSources)
{
    d->sources.httpSources = newHttpSources;
}

///
/// Returns the encrypted sources for this file.
///
const QVector<QXmppEncryptedFileSource> &QXmppFileShare::encryptedSources() const
{
    return d->sources.encryptedSources;
}

///
/// Sets the encrypted sources for this file.
///
void QXmppFileShare::setEncryptedSourecs(const QVector<QXmppEncryptedFileSource> &newEncryptedSources)
{
    d->sources.encryptedSources = newEncryptedSources;
}

/// \cond
void QXmppFileShare::visitSources(std::function<bool(const std::any &)> &&visitor) const
{
    for (const auto &httpSource : d->sources.httpSources) {
        if (visitor(httpSource)) {
            return;
        }
    }
    for (const auto &encryptedSource : d->sources.encryptedSources) {
        if (visitor(encryptedSource)) {
            return;
        }
    }
}

void QXmppFileShare::addSource(const std::any &source)
{
    if (source.type() == typeid(QXmppHttpFileSource)) {
        d->sources.httpSources.push_back(std::any_cast<QXmppHttpFileSource>(source));
    } else if (source.type() == typeid(QXmppEncryptedFileSource)) {
        d->sources.encryptedSources.push_back(std::any_cast<QXmppEncryptedFileSource>(source));
    }
}

bool QXmppFileShare::parse(const QDomElement &el)
{
    if (el.tagName() == u"file-sharing" && el.namespaceURI() == ns_sfs) {
        // disposition
        d->disposition = dispositionFromString(el.attribute(QStringLiteral("disposition")))
                             .value_or(Disposition::Inline);
        d->id = el.attribute(QStringLiteral("id"));

        // file metadata
        auto fileEl = firstChildElement(el, u"file");
        d->metadata = QXmppFileMetadata();
        if (!d->metadata.parse(fileEl)) {
            return false;
        }

        // sources
        if (auto sourcesEl = firstChildElement(el, u"sources", ns_sfs); !sourcesEl.isNull()) {
            d->sources = FileSources::fromSourcesDom(sourcesEl);
        }
        return true;
    }
    return false;
}

void QXmppFileShare::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("file-sharing"));
    writer->writeDefaultNamespace(toString65(ns_sfs));
    writer->writeAttribute(QSL65("disposition"), dispositionToString(d->disposition));
    writeOptionalXmlAttribute(writer, u"id", d->id);
    d->metadata.toXml(writer);

    // sources
    writer->writeStartElement(QSL65("sources"));
    d->sources.innerToXml(writer);
    writer->writeEndElement();
    writer->writeEndElement();
}
/// \endcond
