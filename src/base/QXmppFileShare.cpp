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

/// \cond
class QXmppFileSharePrivate : public QSharedData
{
public:
    QXmppFileMetadata metadata;
    QString id;
    QVector<QXmppHttpFileSource> httpSources;
    QVector<QXmppEncryptedFileSource> encryptedSources;
    QXmppFileShare::Disposition disposition = Disposition::Inline;
};
/// \endcond

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

/// Returns the HTTP sources for this file.
const QVector<QXmppHttpFileSource> &QXmppFileShare::httpSources() const
{
    return d->httpSources;
}

/// Sets the HTTP sources for this file.
void QXmppFileShare::setHttpSources(const QVector<QXmppHttpFileSource> &newHttpSources)
{
    d->httpSources = newHttpSources;
}

/// Returns the encrypted sources for this file.
const QVector<QXmppEncryptedFileSource> &QXmppFileShare::encryptedSources() const
{
    return d->encryptedSources;
}

/// Sets the encrypted sources for this file.
void QXmppFileShare::setEncryptedSourecs(const QVector<QXmppEncryptedFileSource> &newEncryptedSources)
{
    d->encryptedSources = newEncryptedSources;
}

/// \cond
void QXmppFileShare::visitSources(std::function<bool(const std::any &)> &&visitor) const
{
    for (const auto &httpSource : d->httpSources) {
        if (visitor(httpSource)) {
            return;
        }
    }
    for (const auto &encryptedSource : d->encryptedSources) {
        if (visitor(encryptedSource)) {
            return;
        }
    }
}

void QXmppFileShare::addSource(const std::any &source)
{
    if (source.type() == typeid(QXmppHttpFileSource)) {
        d->httpSources.push_back(std::any_cast<QXmppHttpFileSource>(source));
    } else if (source.type() == typeid(QXmppEncryptedFileSource)) {
        d->encryptedSources.push_back(std::any_cast<QXmppEncryptedFileSource>(source));
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

        // sources:
        // expect that there's only one sources element with the correct namespace
        auto sources = firstChildElement(el, u"sources");
        for (const auto &sourceEl : iterChildElements(sources, u"url-data")) {
            QXmppHttpFileSource source;
            if (source.parse(sourceEl)) {
                d->httpSources.push_back(std::move(source));
            }
        }
        for (const auto &sourceEl : iterChildElements(sources, u"encrypted")) {
            QXmppEncryptedFileSource source;
            if (source.parse(sourceEl)) {
                d->encryptedSources.push_back(std::move(source));
            }
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
    writer->writeStartElement(QSL65("sources"));
    for (const auto &source : d->httpSources) {
        source.toXml(writer);
    }
    for (const auto &source : d->encryptedSources) {
        source.toXml(writer);
    }
    writer->writeEndElement();
    writer->writeEndElement();
}
/// \endcond
