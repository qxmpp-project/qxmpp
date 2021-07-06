/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *  Linus Jahn
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "QXmppDataForm.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <optional>

#include <QDebug>
#include <QDomElement>
#include <QMimeDatabase>
#include <QMimeType>
#include <QSize>
#include <QStringList>
#include <QUrl>

struct field_type {
    QXmppDataForm::Field::Type type;
    const char *str;
};

static field_type field_types[] = {
    { QXmppDataForm::Field::BooleanField, "boolean" },
    { QXmppDataForm::Field::FixedField, "fixed" },
    { QXmppDataForm::Field::HiddenField, "hidden" },
    { QXmppDataForm::Field::JidMultiField, "jid-multi" },
    { QXmppDataForm::Field::JidSingleField, "jid-single" },
    { QXmppDataForm::Field::ListMultiField, "list-multi" },
    { QXmppDataForm::Field::ListSingleField, "list-single" },
    { QXmppDataForm::Field::TextMultiField, "text-multi" },
    { QXmppDataForm::Field::TextPrivateField, "text-private" },
    { QXmppDataForm::Field::TextSingleField, "text-single" },
    { static_cast<QXmppDataForm::Field::Type>(-1), nullptr },
};

std::optional<QXmppDataForm::Field::Type> fieldTypeFromString(const QString &type)
{
    struct field_type *ptr;
    for (ptr = field_types; ptr->str; ptr++) {
        if (type == ptr->str) {
            return ptr->type;
        }
    }
    return {};
}

QString fieldTypeToString(QXmppDataForm::Field::Type type)
{
    struct field_type *ptr;
    for (ptr = field_types; ptr->str; ptr++) {
        if (type == ptr->type) {
            return ptr->str;
        }
    }
    return {};
}

std::optional<QXmppDataForm::Type> formTypeFromString(const QString &type)
{
    if (type == "form")
        return QXmppDataForm::Form;
    if (type == "submit")
        return QXmppDataForm::Submit;
    if (type == "cancel")
        return QXmppDataForm::Cancel;
    if (type == "result")
        return QXmppDataForm::Result;
    return {};
}

QString formTypeToString(QXmppDataForm::Type type)
{
    switch (type) {
    case QXmppDataForm::Form:
        return "form";
    case QXmppDataForm::Submit:
        return "submit";
    case QXmppDataForm::Cancel:
        return "cancel";
    case QXmppDataForm::Result:
        return "result";
    default:
        return {};
    }
}

class QXmppDataFormMediaSourcePrivate : public QSharedData
{
public:
    QUrl uri;
    QMimeType contentType;
};

///
/// \class QXmppDataForm::MediaSource
///
/// The QXmppDataForm::MediaSource class represents a link to one of possibly
/// multiple sources for a media element from \xep{0221, Data Forms Media
/// Element} consisting of a MIME type and a QUrl.
///
/// \since QXmpp 1.1
///

///
/// Default constructor
///
QXmppDataForm::MediaSource::MediaSource()
    : d(new QXmppDataFormMediaSourcePrivate)
{
}

///
/// Constructs a MediaSource and sets its uri and contentType.
///
/// \param uri
/// \param contentType
///
QXmppDataForm::MediaSource::MediaSource(const QUrl &uri, const QMimeType &contentType)
    : d(new QXmppDataFormMediaSourcePrivate)
{
    d->uri = uri;
    d->contentType = contentType;
}

/// Default copy-constructor
QXmppDataForm::MediaSource::MediaSource(const QXmppDataForm::MediaSource &) = default;

QXmppDataForm::MediaSource::~MediaSource() = default;

/// Default assignment operator
QXmppDataForm::MediaSource &QXmppDataForm::MediaSource::operator=(const QXmppDataForm::MediaSource &) = default;

///
/// Returns the media URI as QUrl. This can be i.e. a \c http:// URL or a
/// \c cid: Bits of Binary URI.
///
QUrl QXmppDataForm::MediaSource::uri() const
{
    return d->uri;
}

///
/// Sets the URI.
///
void QXmppDataForm::MediaSource::setUri(const QUrl &uri)
{
    d->uri = uri;
}

///
/// Returns the content type of the source
///
QMimeType QXmppDataForm::MediaSource::contentType() const
{
    return d->contentType;
}

///
/// Sets the content type of the media source.
///
void QXmppDataForm::MediaSource::setContentType(const QMimeType &contentType)
{
    d->contentType = contentType;
}

///
/// Returns true if two media sources are identical.
///
bool QXmppDataForm::MediaSource::operator==(const QXmppDataForm::MediaSource &other) const
{
    return d->uri == other.uri() && d->contentType == other.contentType();
}

class QXmppDataFormMediaPrivate : public QSharedData
{
public:
    QSize size;
    QList<QPair<QString, QString>> uris;
};

///
/// \class QXmppDataForm::Media
///
/// The QXmppDataForm::Media class represents a media field as defined by
/// \xep{0221, Data Forms Media Element}.
///
/// \deprecated This class is deprecated since QXmpp 1.1.
///

///
/// Constructs an empty QXmppDataForm::Media.
///
/// \deprecated This class is deprecated since QXmpp 1.1.
///
QXmppDataForm::Media::Media()
    : d(new QXmppDataFormMediaPrivate)
{
}

///
/// Constructs a copy of \a other.
///
/// \deprecated This class is deprecated since QXmpp 1.1.
///
QXmppDataForm::Media::Media(const QXmppDataForm::Media &other) = default;

///
/// Destroys the media.
///
/// \deprecated This class is deprecated since QXmpp 1.1.
///
QXmppDataForm::Media::~Media() = default;

///
/// Assigns \a other to this media.
///
/// \deprecated This class is deprecated since QXmpp 1.1.
///
QXmppDataForm::Media &QXmppDataForm::Media::operator=(const QXmppDataForm::Media &other) = default;

///
/// Returns media's height.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppDataForm::Field::mediaSize().height() instead.
///
int QXmppDataForm::Media::height() const
{
    return d->size.height();
}

///
/// Sets media's \a height.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppDataForm::Field::mediaSize().setHeight() instead.
///
void QXmppDataForm::Media::setHeight(int height)
{
    d->size.setHeight(height);
}

///
/// Returns media's width.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppDataForm::Field::mediaSize().width() instead.
///
int QXmppDataForm::Media::width() const
{
    return d->size.width();
}

///
/// Sets media's \a width.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppDataForm::Field::mediaSize().setWidth() instead.
///
void QXmppDataForm::Media::setWidth(int width)
{
    d->size.setWidth(width);
}

///
/// Returns media's uris.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppDataForm::Field::mediaSources() instead.
///
QList<QPair<QString, QString>> QXmppDataForm::Media::uris() const
{
    return d->uris;
}

///
/// Sets media's \a uris.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppDataForm::Media::setMediaSources() instead.
///
void QXmppDataForm::Media::setUris(const QList<QPair<QString, QString>> &uris)
{
    d->uris = uris;
}

///
/// Returns true if no media tag present.
///
bool QXmppDataForm::Media::isNull() const
{
    return d->uris.isEmpty();
}

class QXmppDataFormFieldPrivate : public QSharedData
{
public:
    QString description;
    QString key;
    QString label;
    QList<QPair<QString, QString>> options;
    bool required = false;
    QXmppDataForm::Field::Type type = QXmppDataForm::Field::TextSingleField;
    QVariant value;
    QSize mediaSize;
    QVector<QXmppDataForm::MediaSource> mediaSources;
};

///
/// \class QXmppDataForm::Field
///
/// The QXmppDataForm::Field class represents a data form field as defined by
/// \xep{0004, Data Forms}.
///

///
/// Constructs a QXmppDataForm::Field of the specified \a type.
///
QXmppDataForm::Field::Field(QXmppDataForm::Field::Type type)
    : d(new QXmppDataFormFieldPrivate)
{
    d->type = type;
}

///
/// Constructs a QXmppDataForm::Field with the specified attributes.
///
/// \since QXmpp 1.3
///
QXmppDataForm::Field::Field(QXmppDataForm::Field::Type type,
                            const QString &key,
                            const QVariant &value,
                            bool isRequired,
                            const QString &label,
                            const QString &description,
                            const QList<QPair<QString, QString>> &options)
    : d(new QXmppDataFormFieldPrivate)
{
    d->type = type;
    d->key = key;
    d->value = value;
    d->required = isRequired;
    d->label = label;
    d->description = description;
    d->options = options;
}

/// Constructs a copy of \a other.
QXmppDataForm::Field::Field(const QXmppDataForm::Field &other) = default;

/// Destroys the form field.
QXmppDataForm::Field::~Field() = default;

/// Assigns \a other to this field.
QXmppDataForm::Field &QXmppDataForm::Field::operator=(const QXmppDataForm::Field &other) = default;

///
/// Returns the field's description.
///
QString QXmppDataForm::Field::description() const
{
    return d->description;
}

///
/// Sets the field's description.
///
/// \param description
///
void QXmppDataForm::Field::setDescription(const QString &description)
{
    d->description = description;
}

///
/// Returns the field's key.
///
QString QXmppDataForm::Field::key() const
{
    return d->key;
}

///
/// Sets the field's key.
///
/// \param key
///
void QXmppDataForm::Field::setKey(const QString &key)
{
    d->key = key;
}

///
/// Returns the field's label.
///
QString QXmppDataForm::Field::label() const
{
    return d->label;
}

///
/// Sets the field's label.
///
/// \param label
///
void QXmppDataForm::Field::setLabel(const QString &label)
{
    d->label = label;
}

///
/// Returns the field's media.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppDataForm::Field::mediaSources() or
/// \c QXmppDataForm::Field::mediaSize() instead.
///
QXmppDataForm::Media QXmppDataForm::Field::media() const
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    Media media;
    QList<QPair<QString, QString>> pairUris;
    pairUris.reserve(d->mediaSources.size());

    for (const auto &source : std::as_const(d->mediaSources)) {
        pairUris << qMakePair<QString, QString>(
            source.contentType().name(),
            source.uri().toString());
    }

    media.setHeight(d->mediaSize.height());
    media.setWidth(d->mediaSize.width());
    media.setUris(pairUris);
    return media;
    QT_WARNING_POP
}

///
/// Sets the field's \a media.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppDataForm::Field::setMediaSources() or
/// \c QXmppDataForm::Field::setMediaSize() instead.
///
void QXmppDataForm::Field::setMedia(const QXmppDataForm::Media &media)
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    const QList<QPair<QString, QString>> &uris = media.uris();

    QVector<QXmppDataForm::MediaSource> sources;
    sources.reserve(uris.size());

    for (const auto &pairUri : uris) {
        sources << QXmppDataForm::MediaSource(
            QUrl(pairUri.second),
            QMimeDatabase().mimeTypeForName(pairUri.first));
    }

    d->mediaSources = sources;
    d->mediaSize = QSize(media.width(), media.height());
    QT_WARNING_POP
}

///
/// Returns the field's options.
///
QList<QPair<QString, QString>> QXmppDataForm::Field::options() const
{
    return d->options;
}

///
/// Sets the field's options.
///
/// \param options
///
void QXmppDataForm::Field::setOptions(const QList<QPair<QString, QString>> &options)
{
    d->options = options;
}

///
/// Returns true if the field is required, false otherwise.
///
bool QXmppDataForm::Field::isRequired() const
{
    return d->required;
}

///
/// Set to true if the field is required, false otherwise.
///
/// \param required
///
void QXmppDataForm::Field::setRequired(bool required)
{
    d->required = required;
}

///
/// Returns the field's type.
///
QXmppDataForm::Field::Type QXmppDataForm::Field::type() const
{
    return d->type;
}

///
/// Sets the field's type.
///
/// \param type
///
void QXmppDataForm::Field::setType(QXmppDataForm::Field::Type type)
{
    d->type = type;
}

///
/// Returns the field's value.
///
QVariant QXmppDataForm::Field::value() const
{
    return d->value;
}

///
/// Sets the field's value.
///
/// \param value
///
void QXmppDataForm::Field::setValue(const QVariant &value)
{
    d->value = value;
}

///
/// Returns the size of the attached media according to \xep{0221}: Data Forms
/// Media Element.
///
/// \since QXmpp 1.1
///
QSize QXmppDataForm::Field::mediaSize() const
{
    return d->mediaSize;
}

///
/// Returns the size of the attached media according to \xep{0221}: Data Forms
/// Media Element.
///
/// \since QXmpp 1.1
///
QSize &QXmppDataForm::Field::mediaSize()
{
    return d->mediaSize;
}

///
/// Sets the size of the attached media according to \xep{0221}: Data Forms Media
/// Element.
///
/// \since QXmpp 1.1
///
void QXmppDataForm::Field::setMediaSize(const QSize &size)
{
    d->mediaSize = size;
}

///
/// Returns the sources for the attached media according to \xep{0221}: Data
/// Forms Media Element.
///
/// \since QXmpp 1.1
///
QVector<QXmppDataForm::MediaSource> QXmppDataForm::Field::mediaSources() const
{
    return d->mediaSources;
}

///
/// Returns the sources for the attached media according to \xep{0221}: Data
/// Forms Media Element.
///
/// \since QXmpp 1.1
///
QVector<QXmppDataForm::MediaSource> &QXmppDataForm::Field::mediaSources()
{
    return d->mediaSources;
}

///
/// Sets the sources to the attached media of the field according to \xep{0221}:
/// Data Forms Media Element.
///
/// \since QXmpp 1.1
///
void QXmppDataForm::Field::setMediaSources(const QVector<QXmppDataForm::MediaSource> &mediaSources)
{
    d->mediaSources = mediaSources;
}

///
/// Returns true if the other field is identical to this one.
///
/// \since QXmpp 1.1
///
bool QXmppDataForm::Field::operator==(const QXmppDataForm::Field &other) const
{
    return d->description == other.description() &&
        d->key == other.key() &&
        d->label == other.label() &&
        d->options == other.options() &&
        d->required == other.isRequired() &&
        d->type == other.type() &&
        d->value == other.value() &&
        d->mediaSources == other.mediaSources() &&
        d->mediaSize == other.mediaSize();
}

class QXmppDataFormPrivate : public QSharedData
{
public:
    QString instructions;
    QList<QXmppDataForm::Field> fields;
    QString title;
    QXmppDataForm::Type type = QXmppDataForm::None;
};

///
/// \class QXmppDataForm
///
/// The QXmppDataForm class represents a data form as defined by \xep{0004,
/// Data Forms}.
///

///
/// Constructs a QXmppDataForm of the specified \a type.
///
QXmppDataForm::QXmppDataForm(Type type)
    : d(new QXmppDataFormPrivate)
{
    d->type = type;
}

///
/// Constructs a QXmppDataForm with the specified attributes.
///
/// \since QXmpp 1.3
///
QXmppDataForm::QXmppDataForm(Type type,
                             const QList<Field> &fields,
                             const QString &title,
                             const QString &instructions)
    : d(new QXmppDataFormPrivate)
{
    d->type = type;
    d->fields = fields;
    d->title = title;
    d->instructions = instructions;
}

/// Constructs a copy of \a other.
QXmppDataForm::QXmppDataForm(const QXmppDataForm &other) = default;

/// Destroys the form.
QXmppDataForm::~QXmppDataForm() = default;

/// Assigns \a other to this form.
QXmppDataForm &QXmppDataForm::operator=(const QXmppDataForm &other) = default;

///
/// Returns the form's fields.
///
QList<QXmppDataForm::Field> QXmppDataForm::fields() const
{
    return d->fields;
}

///
/// Returns the form's fields by reference.
///
QList<QXmppDataForm::Field> &QXmppDataForm::fields()
{
    return d->fields;
}

///
/// \fn QXmppDataForm::constFields
///
/// Returns the form's fields by value and can be used safely on temporaries.
///
/// \since QXmpp 1.4.1
///

///
/// Sets the form's fields.
///
/// \param fields
///
void QXmppDataForm::setFields(const QList<QXmppDataForm::Field> &fields)
{
    d->fields = fields;
}

///
/// Returns the form's instructions.
///
QString QXmppDataForm::instructions() const
{
    return d->instructions;
}

///
/// Sets the form's instructions.
///
/// \param instructions
///
void QXmppDataForm::setInstructions(const QString &instructions)
{
    d->instructions = instructions;
}

///
/// Returns the form's title.
///
QString QXmppDataForm::title() const
{
    return d->title;
}

///
/// Sets the form's title.
///
/// \param title
///
void QXmppDataForm::setTitle(const QString &title)
{
    d->title = title;
}

///
/// Returns the form's type.
///
QXmppDataForm::Type QXmppDataForm::type() const
{
    return d->type;
}

///
/// Sets the form's type.
///
/// \param type
///
void QXmppDataForm::setType(QXmppDataForm::Type type)
{
    d->type = type;
}

///
/// Searches for a hidden field called 'FORM_TYPE' and returns its value.
///
/// \returns The string value of the field or a null string if the field
/// couldn't be found.
///
/// \since QXmpp 1.5
///
QString QXmppDataForm::formType() const
{
    const auto formTypeItr = std::find_if(d->fields.begin(), d->fields.end(), [](const QXmppDataForm::Field &field) {
        return field.type() == QXmppDataForm::Field::HiddenField &&
            field.key() == QStringLiteral("FORM_TYPE");
    });

    if (formTypeItr != d->fields.end()) {
        return formTypeItr->value().toString();
    }
    return {};
}

///
/// Returns true if the form has an unknown type.
///
bool QXmppDataForm::isNull() const
{
    return d->type == QXmppDataForm::None;
}

/// \cond
void QXmppDataForm::parse(const QDomElement &element)
{
    if (element.isNull())
        return;

    /* form type */
    if (const auto type = formTypeFromString(element.attribute("type"))) {
        d->type = *type;
    } else {
        qWarning() << "Unknown form type" << element.attribute("type");
        return;
    }

    /* form properties */
    d->title = element.firstChildElement("title").text();
    d->instructions = element.firstChildElement("instructions").text();

    for (auto fieldElement = element.firstChildElement("field");
         !fieldElement.isNull();
         fieldElement = fieldElement.nextSiblingElement("field")) {
        QXmppDataForm::Field field;

        /* field type */
        field.setType(fieldTypeFromString(fieldElement.attribute("type")).value_or(Field::TextSingleField));

        /* field attributes */
        field.setLabel(fieldElement.attribute("label"));
        field.setKey(fieldElement.attribute("var"));

        /* field value(s) */
        switch (field.type()) {
        case Field::BooleanField: {
            const auto valueStr = fieldElement.firstChildElement("value").text();
            field.setValue(valueStr == "1" || valueStr == "true");
            break;
        }
        case Field::ListMultiField:
        case Field::JidMultiField:
        case Field::TextMultiField: {
            QStringList values;
            for (auto element = fieldElement.firstChildElement("value");
                 !element.isNull();
                 element = element.nextSiblingElement("value")) {
                values << element.text();
            }
            field.setValue(values);
            break;
        }
        default:
            field.setValue(fieldElement.firstChildElement("value").text());
        }

        /* field media */
        if (const auto mediaElement = fieldElement.firstChildElement("media");
            mediaElement.namespaceURI() == ns_media_element) {
            field.mediaSize().setHeight(mediaElement.attribute("height", "-1").toInt());
            field.mediaSize().setWidth(mediaElement.attribute("width", "-1").toInt());

            QMimeDatabase database;

            for (auto element = mediaElement.firstChildElement(QStringLiteral("uri"));
                 !element.isNull();
                 element = element.nextSiblingElement(QStringLiteral("uri"))) {
                field.mediaSources() << MediaSource(
                    QUrl(element.text()),
                    database.mimeTypeForName(element.attribute(QStringLiteral("type"))));
            }
        }

        /* field options */
        switch (field.type()) {
        case Field::ListMultiField:
        case Field::ListSingleField: {
            QList<QPair<QString, QString>> options;
            for (auto element = fieldElement.firstChildElement("option");
                 !element.isNull();
                 element = element.nextSiblingElement("option")) {
                options << QPair(element.attribute("label"), element.firstChildElement("value").text());
            }
            field.setOptions(options);
        }
        default:
            break;
        }

        /* other properties */
        field.setDescription(fieldElement.firstChildElement("description").text());
        field.setRequired(!fieldElement.firstChildElement("required").isNull());

        d->fields.append(field);
    }
}

void QXmppDataForm::toXml(QXmlStreamWriter *writer) const
{
    if (isNull())
        return;

    writer->writeStartElement("x");
    writer->writeDefaultNamespace(ns_data);

    /* form type */
    writer->writeAttribute("type", formTypeToString(d->type));

    /* form properties */
    if (!d->title.isEmpty())
        writer->writeTextElement("title", d->title);
    if (!d->instructions.isEmpty())
        writer->writeTextElement("instructions", d->instructions);

    for (const auto &field : d->fields) {
        writer->writeStartElement("field");

        /* field type */
        writer->writeAttribute("type", fieldTypeToString(field.type()));

        /* field attributes */
        helperToXmlAddAttribute(writer, "label", field.label());
        helperToXmlAddAttribute(writer, "var", field.key());

        /* field value(s) */
        switch (field.type()) {
        case Field::BooleanField:
            helperToXmlAddTextElement(writer, "value", field.value().toBool() ? "1" : "0");
            break;
        case Field::ListMultiField:
        case Field::JidMultiField:
        case Field::TextMultiField: {
            const auto values = field.value().toStringList();
            for (const QString &value : values) {
                helperToXmlAddTextElement(writer, "value", value);
            }
            break;
        }
        default:
            if (const auto value = field.value().toString(); !value.isEmpty()) {
                helperToXmlAddTextElement(writer, "value", value);
            }
        }

        /* field media */
        if (!field.mediaSources().isEmpty()) {
            writer->writeStartElement("media");
            writer->writeDefaultNamespace(ns_media_element);

            // media width and height
            if (field.mediaSize().width() > 0)
                helperToXmlAddAttribute(
                    writer,
                    QStringLiteral("width"),
                    QString::number(field.mediaSize().width()));
            if (field.mediaSize().height() > 0)
                helperToXmlAddAttribute(
                    writer,
                    QStringLiteral("height"),
                    QString::number(field.mediaSize().height()));

            const auto sources = field.mediaSources();
            for (const auto &source : sources) {
                writer->writeStartElement(QStringLiteral("uri"));
                helperToXmlAddAttribute(writer, QStringLiteral("type"), source.contentType().name());
                writer->writeCharacters(source.uri().toString());
                writer->writeEndElement();
            }

            writer->writeEndElement();
        }

        /* field options */
        switch (field.type()) {
        case Field::ListMultiField:
        case Field::ListSingleField: {
            const auto options = field.options();
            for (const auto &option : options) {
                writer->writeStartElement("option");
                helperToXmlAddAttribute(writer, "label", option.first);
                helperToXmlAddTextElement(writer, "value", option.second);
                writer->writeEndElement();
            }
        }
        default:
            break;
        }

        /* other properties */
        if (!field.description().isEmpty())
            helperToXmlAddTextElement(writer, "description", field.description());
        if (field.isRequired())
            helperToXmlAddTextElement(writer, "required", "");

        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond
