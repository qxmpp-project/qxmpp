/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#include <QDebug>
#include <QDomElement>
#include <QSize>
#include <QStringList>

#include "QXmppConstants.h"
#include "QXmppDataForm.h"
#include "QXmppUtils.h"

struct field_type {
    QXmppDataForm::Field::Type type;
    const char *str;
};

static field_type field_types[] = {
    {QXmppDataForm::Field::BooleanField, "boolean"},
    {QXmppDataForm::Field::FixedField, "fixed"},
    {QXmppDataForm::Field::HiddenField, "hidden"},
    {QXmppDataForm::Field::JidMultiField, "jid-multi"},
    {QXmppDataForm::Field::JidSingleField, "jid-single"},
    {QXmppDataForm::Field::ListMultiField, "list-multi"},
    {QXmppDataForm::Field::ListSingleField, "list-single"},
    {QXmppDataForm::Field::TextMultiField, "text-multi"},
    {QXmppDataForm::Field::TextPrivateField, "text-private"},
    {QXmppDataForm::Field::TextSingleField, "text-single"},
    {static_cast<QXmppDataForm::Field::Type>(-1), NULL},
};

class QXmppDataFormMediaPrivate : public QSharedData
{
public:
    QSize size;
    QList<QPair<QString, QString> > uris;
};

/// Constructs an empty QXmppDataForm::Media.

QXmppDataForm::Media::Media()
    : d(new QXmppDataFormMediaPrivate)
{
}

/// Constructs a copy of \a other.

QXmppDataForm::Media::Media(const QXmppDataForm::Media &other)
    : d(other.d)
{
}

/// Destroys the media.

QXmppDataForm::Media::~Media()
{
}

/// Assigns \a other to this media.

QXmppDataForm::Media& QXmppDataForm::Media::operator=(const QXmppDataForm::Media &other)
{
    d = other.d;
    return *this;
}

/// Returns media's height.

int QXmppDataForm::Media::height() const
{
    return d->size.height();
}

/// Sets media's \a height.

void QXmppDataForm::Media::setHeight(int height)
{
    d->size.setHeight(height);
}

/// Returns media's width.

int QXmppDataForm::Media::width() const
{
    return d->size.width();
}

/// Sets media's \a width.

void QXmppDataForm::Media::setWidth(int width)
{
    d->size.setWidth(width);
}

/// Returns media's uris.

QList< QPair< QString, QString > > QXmppDataForm::Media::uris() const
{
    return d->uris;
}

/// Sets media's \a uris.

void QXmppDataForm::Media::setUris(const QList< QPair< QString, QString > > &uris)
{
    d->uris = uris;
}

/// Returns true if no media tag present.

bool QXmppDataForm::Media::isNull() const
{
    return d->uris.empty();
}

void QXmppDataForm::Media::parse(const QDomElement &element)
{
    setHeight(element.attribute("height", "-1").toInt());
    setWidth(element.attribute("width", "-1").toInt());

    QList<QPair<QString, QString> > uris;
    QDomElement uriElement = element.firstChildElement("uri");
    while (!uriElement.isNull()) {
        uris.append(QPair<QString, QString>(uriElement.attribute("type"),
            uriElement.text()));
        uriElement = uriElement.nextSiblingElement("uri");
    }
    setUris(uris);
}

void QXmppDataForm::Media::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("media");
    helperToXmlAddAttribute(writer, "xmlns", ns_media_element);
    if (height() > 0)
        helperToXmlAddAttribute(writer, "height", QString::number(height()));
    if (width() > 0)
        helperToXmlAddAttribute(writer, "width", QString::number(width()));

    QPair<QString, QString> uri;
    foreach(uri, uris()) {
        writer->writeStartElement("uri");
        helperToXmlAddAttribute(writer, "type", uri.first);
        writer->writeCharacters(uri.second);
        writer->writeEndElement();
    }
    writer->writeEndElement();
}

class QXmppDataFormFieldPrivate : public QSharedData
{
public:
    QXmppDataFormFieldPrivate();

    QString description;
    QString key;
    QString label;
    QXmppDataForm::Media media;
    QList<QPair<QString, QString> > options;
    bool required;
    QXmppDataForm::Field::Type type;
    QVariant value;
};

QXmppDataFormFieldPrivate::QXmppDataFormFieldPrivate()
    : required(false)
    , type(QXmppDataForm::Field::TextSingleField)
{
}

/// Constructs a QXmppDataForm::Field of the specified \a type.

QXmppDataForm::Field::Field(QXmppDataForm::Field::Type type)
    : d(new QXmppDataFormFieldPrivate)
{
    d->type = type;
}

/// Constructs a copy of \a other.

QXmppDataForm::Field::Field(const QXmppDataForm::Field &other)
    : d(other.d)
{
}

/// Destroys the form field.

QXmppDataForm::Field::~Field()
{
}

/// Assigns \a other to this field.

QXmppDataForm::Field& QXmppDataForm::Field::operator=(const QXmppDataForm::Field &other)
{
    d = other.d;
    return *this;
}

/// Returns the field's description.

QString QXmppDataForm::Field::description() const
{
    return d->description;
}

/// Sets the field's description.
///
/// \param description

void QXmppDataForm::Field::setDescription(const QString &description)
{
    d->description = description;
}

/// Returns the field's key.

QString QXmppDataForm::Field::key() const
{
    return d->key;
}

/// Sets the field's key.
///
/// \param key

void QXmppDataForm::Field::setKey(const QString &key)
{
    d->key = key;
}

/// Returns the field's label.

QString QXmppDataForm::Field::label() const
{
    return d->label;
}

/// Sets the field's label.
///
/// \param label

void QXmppDataForm::Field::setLabel(const QString &label)
{
    d->label = label;
}

/// Returns the field's media.

QXmppDataForm::Media QXmppDataForm::Field::media() const
{
    return d->media;
}

/// Sets the field's \a media.

void QXmppDataForm::Field::setMedia(const QXmppDataForm::Media &media)
{
    d->media = media;
}

/// Returns the field's options.

QList<QPair<QString, QString> > QXmppDataForm::Field::options() const
{
    return d->options;
}

/// Sets the field's options.
///
/// \param options

void QXmppDataForm::Field::setOptions(const QList<QPair<QString, QString> > &options)
{
    d->options = options;
}

/// Returns true if the field is required, false otherwise.

bool QXmppDataForm::Field::isRequired() const
{
    return d->required;
}

/// Set to true if the field is required, false otherwise.
///
/// \param required

void QXmppDataForm::Field::setRequired(bool required)
{
    d->required = required;
}

/// Returns the field's type.

QXmppDataForm::Field::Type QXmppDataForm::Field::type() const
{
    return d->type;
}

/// Sets the field's type.
///
/// \param type

void QXmppDataForm::Field::setType(QXmppDataForm::Field::Type type)
{
    d->type = type;
}

/// Returns the field's value.

QVariant QXmppDataForm::Field::value() const
{
    return d->value;
}

/// Sets the field's value.
///
/// \param value

void QXmppDataForm::Field::setValue(const QVariant &value)
{
    d->value = value;
}

void QXmppDataForm::Field::parse(const QDomElement &element)
{
    /* field type */
    QXmppDataForm::Field::Type type = QXmppDataForm::Field::TextSingleField;
    const QString typeStr = element.attribute("type");
    struct field_type *ptr;
    for (ptr = field_types; ptr->str; ptr++)
    {
        if (typeStr == ptr->str)
        {
            type = ptr->type;
            break;
        }
    }
    setType(type);

    /* field attributes */
    setLabel(element.attribute("label"));
    setKey(element.attribute("var"));

    /* field value(s) */
    if (type == QXmppDataForm::Field::BooleanField)
    {
        const QString valueStr = element.firstChildElement("value").text();
        setValue(valueStr == "1" || valueStr == "true");
    }
    else if (type == QXmppDataForm::Field::ListMultiField ||
        type == QXmppDataForm::Field::JidMultiField ||
        type == QXmppDataForm::Field::TextMultiField)
    {
        QStringList values;
        QDomElement valueElement = element.firstChildElement("value");
        while (!valueElement.isNull())
        {
            values.append(valueElement.text());
            valueElement = valueElement.nextSiblingElement("value");
        }
        setValue(values);
    }
    else
    {
        setValue(element.firstChildElement("value").text());
    }

    /* field media */
    QDomElement mediaElement = element.firstChildElement("media");
    if (!mediaElement.isNull()) {
        Media media;
        media.parse(mediaElement);
        setMedia(media);
    }

    /* field options */
    if (type == QXmppDataForm::Field::ListMultiField ||
        type == QXmppDataForm::Field::ListSingleField)
    {
        QList<QPair<QString, QString> > options;
        QDomElement optionElement = element.firstChildElement("option");
        while (!optionElement.isNull())
        {
            options.append(QPair<QString, QString>(optionElement.attribute("label"),
                optionElement.firstChildElement("value").text()));
            optionElement = optionElement.nextSiblingElement("option");
        }
        setOptions(options);
    }

    /* other properties */
    setDescription(element.firstChildElement("description").text());
    setRequired(!element.firstChildElement("required").isNull());
}

void QXmppDataForm::Field::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("field");

    /* field type */
    const QXmppDataForm::Field::Type type = this->type();
    QString typeStr;
    struct field_type *ptr;
    for (ptr = field_types; ptr->str; ptr++)
    {
        if (type == ptr->type)
        {
            typeStr = ptr->str;
            break;
        }
    }
    writer->writeAttribute("type", typeStr);

    /* field attributes */
    helperToXmlAddAttribute(writer, "label", label());
    helperToXmlAddAttribute(writer, "var", key());

    /* field value(s) */
    if (type == QXmppDataForm::Field::BooleanField)
    {
        helperToXmlAddTextElement(writer, "value", value().toBool() ? "1" : "0");
    }
    else if (type == QXmppDataForm::Field::ListMultiField ||
        type == QXmppDataForm::Field::JidMultiField ||
        type == QXmppDataForm::Field::TextMultiField)
    {
        foreach (const QString &value, value().toStringList())
            helperToXmlAddTextElement(writer, "value", value);
    }
    else if (!value().isNull())
    {
        helperToXmlAddTextElement(writer, "value", value().toString());
    }

    /* field media */
    Media media = this->media();
    if (!media.isNull()) {
        media.toXml(writer);
    }

    /* field options */
    if (type == QXmppDataForm::Field::ListMultiField ||
        type == QXmppDataForm::Field::ListSingleField)
    {
        QPair<QString, QString> option;
        foreach (option, options())
        {
            writer->writeStartElement("option");
            helperToXmlAddAttribute(writer, "label", option.first);
            helperToXmlAddTextElement(writer, "value", option.second);
            writer->writeEndElement();
        }
    }

    /* other properties */
    if (!description().isEmpty())
        helperToXmlAddTextElement(writer, "description", description());
    if (isRequired())
        helperToXmlAddTextElement(writer, "required", "");

    writer->writeEndElement();
}

class QXmppDataFormItemPrivate : public QSharedData
{
public:
    QXmppDataFormItemPrivate();

    QList<QXmppDataForm::Field> fields;
};

QXmppDataFormItemPrivate::QXmppDataFormItemPrivate()
{
}

QXmppDataForm::Item::Item()
    : d(new QXmppDataFormItemPrivate)
{
}

QXmppDataForm::Item::~Item()
{
}

QXmppDataForm::Item::Item(const QXmppDataForm::Item &other)
    : d(other.d)
{
}

QXmppDataForm::Item& QXmppDataForm::Item::operator=(const QXmppDataForm::Item &other)
{
    d = other.d;
    return *this;
}

QList<QXmppDataForm::Field> QXmppDataForm::Item::fields() const
{
    return d->fields;
}

QList<QXmppDataForm::Field> &QXmppDataForm::Item::fields()
{
    return d->fields;
}

void QXmppDataForm::Item::setFields(const QList<QXmppDataForm::Field> &fields)
{
    d->fields = fields;
}

void QXmppDataForm::Item::parse(const QDomElement &element)
{
    QDomElement fieldElement = element.firstChildElement("field");
    while (!fieldElement.isNull())
    {
        QXmppDataForm::Field field;
        field.parse(fieldElement);
        d->fields.append(field);
        fieldElement = fieldElement.nextSiblingElement("field");
    }
}

void QXmppDataForm::Item::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("item");
    foreach (const QXmppDataForm::Field &field, d->fields) {
        field.toXml(writer);
    }
    writer->writeEndElement();
}

class QXmppDataFormReportedPrivate : public QSharedData
{
public:
    QXmppDataFormReportedPrivate();

    QList<QXmppDataForm::Field> fields;
};

QXmppDataFormReportedPrivate::QXmppDataFormReportedPrivate()
{
}

QXmppDataForm::Reported::Reported()
    : d(new QXmppDataFormReportedPrivate)
{
}

QXmppDataForm::Reported::~Reported()
{
}

QXmppDataForm::Reported::Reported(const QXmppDataForm::Reported &other)
    : d(other.d)
{
}

QXmppDataForm::Reported& QXmppDataForm::Reported::operator=(const QXmppDataForm::Reported &other)
{
    d = other.d;
    return *this;
}

QList<QXmppDataForm::Field> QXmppDataForm::Reported::fields() const
{
    return d->fields;
}

QList<QXmppDataForm::Field> &QXmppDataForm::Reported::fields()
{
    return d->fields;
}

void QXmppDataForm::Reported::setFields(const QList<QXmppDataForm::Field> &fields)
{
    d->fields = fields;
}

void QXmppDataForm::Reported::parse(const QDomElement &element)
{
    QDomElement fieldElement = element.firstChildElement("field");
    while (!fieldElement.isNull())
    {
        QXmppDataForm::Field field;
        field.parse(fieldElement);
        d->fields.append(field);
        fieldElement = fieldElement.nextSiblingElement("field");
    }
}

void QXmppDataForm::Reported::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("reported");
    foreach (const QXmppDataForm::Field &field, d->fields) {
        field.toXml(writer);
    }
    writer->writeEndElement();
}

class QXmppDataFormPrivate : public QSharedData
{
public:
    QXmppDataFormPrivate();

    QString instructions;
    QList<QXmppDataForm::Field> fields;
    QList<QXmppDataForm::Item> items;
    QXmppDataForm::Reported reported;
    QString title;
    QXmppDataForm::Type type;
};

QXmppDataFormPrivate::QXmppDataFormPrivate()
    : type(QXmppDataForm::None)
{
}

/// Constructs a QXmppDataForm of the specified \a type.

QXmppDataForm::QXmppDataForm(QXmppDataForm::Type type)
    : d(new QXmppDataFormPrivate)
{
    d->type = type;
}

/// Constructs a copy of \a other.

QXmppDataForm::QXmppDataForm(const QXmppDataForm &other)
    : d(other.d)
{
}

/// Destroys the form.

QXmppDataForm::~QXmppDataForm()
{
}

/// Assigns \a other to this form.

QXmppDataForm& QXmppDataForm::operator=(const QXmppDataForm &other)
{
    d = other.d;
    return *this;
}

/// Returns the form's fields.

QList<QXmppDataForm::Field> QXmppDataForm::fields() const
{
    return d->fields;
}

/// Returns the form's fields by reference.

QList<QXmppDataForm::Field> &QXmppDataForm::fields()
{
    return d->fields;
}

/// Sets the form's fields.
///
/// \param fields

void QXmppDataForm::setFields(const QList<QXmppDataForm::Field> &fields)
{
    d->fields = fields;
}

/// Returns the form's instructions.

QString QXmppDataForm::instructions() const
{
    return d->instructions;
}

/// Sets the form's instructions.
///
/// \param instructions

void QXmppDataForm::setInstructions(const QString &instructions)
{
    d->instructions = instructions;
}

/// Returns the form's title.

QString QXmppDataForm::title() const
{
    return d->title;
}

/// Sets the form's title.
///
/// \param title

void QXmppDataForm::setTitle(const QString &title)
{
    d->title = title;
}

/// Returns the form's type.

QXmppDataForm::Type QXmppDataForm::type() const
{
    return d->type;
}

/// Sets the form's type.
///
/// \param type

void QXmppDataForm::setType(QXmppDataForm::Type type)
{
    d->type = type;
}

/// Returns true if the form has an unknown type.

bool QXmppDataForm::isNull() const
{
    return d->type == QXmppDataForm::None;
}

QList<QXmppDataForm::Item> QXmppDataForm::items() const
{
    return d->items;
}

QList<QXmppDataForm::Item> &QXmppDataForm::items()
{
    return d->items;
}

void QXmppDataForm::setItems(const QList<QXmppDataForm::Item> &items)
{
    d->items = items;
}

QXmppDataForm::Reported QXmppDataForm::reported() const
{
    return d->reported;
}

void QXmppDataForm::setReported(const QXmppDataForm::Reported reported)
{
    d->reported = reported;
}

/// \cond
void QXmppDataForm::parse(const QDomElement &element)
{
    if (element.isNull())
        return;

    /* form type */
    const QString typeStr = element.attribute("type");
    if (typeStr == "form")
        d->type = QXmppDataForm::Form;
    else if (typeStr == "submit")
        d->type = QXmppDataForm::Submit;
    else if (typeStr == "cancel")
        d->type = QXmppDataForm::Cancel;
    else if (typeStr == "result")
        d->type = QXmppDataForm::Result;
    else
    {
        qWarning() << "Unknown form type" << typeStr;
        return;
    }

    /* form properties */
    d->title = element.firstChildElement("title").text();
    d->instructions = element.firstChildElement("instructions").text();

    QDomElement fieldElement = element.firstChildElement("field");
    while (!fieldElement.isNull())
    {
        QXmppDataForm::Field field;
        field.parse(fieldElement);
        d->fields.append(field);
        fieldElement = fieldElement.nextSiblingElement("field");
    }

    QDomElement reportedElement = element.firstChildElement("reported");
    if (!reportedElement.isNull())
    {
        d->reported.parse(reportedElement);
    }

    QDomElement itemElement = element.firstChildElement("item");
    while (!itemElement.isNull())
    {
        QXmppDataForm::Item item;
        item.parse(itemElement);
        d->items.append(item);
        itemElement = itemElement.nextSiblingElement("item");
    }
}

void QXmppDataForm::toXml(QXmlStreamWriter *writer) const
{
    if (isNull())
        return;

    writer->writeStartElement("x");
    writer->writeAttribute("xmlns", ns_data);

    /* form type */
    QString typeStr;
    if (d->type == QXmppDataForm::Form)
        typeStr = "form";
    else if (d->type == QXmppDataForm::Submit)
        typeStr = "submit";
    else if (d->type == QXmppDataForm::Cancel)
        typeStr = "cancel";
    else if (d->type == QXmppDataForm::Result)
        typeStr = "result";
    writer->writeAttribute("type", typeStr);

    /* form properties */
    if (!d->title.isEmpty())
        writer->writeTextElement("title", d->title);
    if (!d->instructions.isEmpty())
        writer->writeTextElement("instructions", d->instructions);

    foreach (const QXmppDataForm::Field &field, d->fields) {
        field.toXml(writer);
    }

    if (!d->reported.fields().isEmpty())
        d->reported.toXml(writer);

    foreach (const QXmppDataForm::Item &item, d->items) {
        item.toXml(writer);
    }
    writer->writeEndElement();
}
/// \endcond
