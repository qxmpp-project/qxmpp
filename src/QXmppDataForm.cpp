/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

QXmppDataForm::Field::Field(QXmppDataForm::Field::Type type)
    : m_type(type)
{
}

QString QXmppDataForm::Field::description() const
{
    return m_description;
}

void QXmppDataForm::Field::setDescription(const QString &description)
{
    m_description = description;
}

QString QXmppDataForm::Field::key() const
{
    return m_key;
}

void QXmppDataForm::Field::setKey(const QString &key)
{
    m_key = key;
}

QString QXmppDataForm::Field::label() const
{
    return m_label;
}

void QXmppDataForm::Field::setLabel(const QString &label)
{
    m_label = label;
}

QList<QPair<QString, QString> > QXmppDataForm::Field::options() const
{
    return m_options;
}

void QXmppDataForm::Field::setOptions(const QList<QPair<QString, QString> > &options)
{
    m_options = options;
}

bool QXmppDataForm::Field::isRequired() const
{
    return m_required;
}

void QXmppDataForm::Field::setRequired(bool required)
{
    m_required = required;
}

QXmppDataForm::Field::Type QXmppDataForm::Field::type() const
{
    return m_type;
}

void QXmppDataForm::Field::setType(QXmppDataForm::Field::Type type)
{
    m_type = type;
}

QVariant QXmppDataForm::Field::value() const
{
    return m_value;
}

void QXmppDataForm::Field::setValue(const QVariant &value)
{
    m_value = value;
}

QXmppDataForm::QXmppDataForm(QXmppDataForm::Type type)
    : m_type(type)
{
}

QList<QXmppDataForm::Field> QXmppDataForm::fields() const
{
    return m_fields;
}

QList<QXmppDataForm::Field> &QXmppDataForm::fields()
{
    return m_fields;
}

void QXmppDataForm::setFields(const QList<QXmppDataForm::Field> &fields)
{
    m_fields = fields;
}

QString QXmppDataForm::instructions() const
{
    return m_instructions;
}

void QXmppDataForm::setInstructions(const QString &instructions)
{
    m_instructions = instructions;
}

QString QXmppDataForm::title() const
{
    return m_title;
}

void QXmppDataForm::setTitle(const QString &title)
{
    m_title = title;
}

QXmppDataForm::Type QXmppDataForm::type() const
{
    return m_type;
}

void QXmppDataForm::setType(QXmppDataForm::Type type)
{
    m_type = type;
}

bool QXmppDataForm::isNull() const
{
    return m_type == QXmppDataForm::None;
}

void QXmppDataForm::parse(const QDomElement &element)
{
    if (element.isNull())
        return;

    /* form type */
    const QString typeStr = element.attribute("type");
    if (typeStr == "form")
        m_type = QXmppDataForm::Form;
    else if (typeStr == "submit")
        m_type = QXmppDataForm::Submit;
    else if (typeStr == "cancel")
        m_type = QXmppDataForm::Cancel;
    else if (typeStr == "result")
        m_type = QXmppDataForm::Result;
    else
    {
        qWarning() << "Unknown form type" << typeStr;
        return;
    }

    /* form properties */
    m_title = element.firstChildElement("title").text();
    m_instructions = element.firstChildElement("instructions").text();

    QDomElement fieldElement = element.firstChildElement("field");
    while (!fieldElement.isNull())
    {
        QXmppDataForm::Field field;

        /* field type */
        QXmppDataForm::Field::Type type = static_cast<QXmppDataForm::Field::Type>(-1);
        const QString typeStr = fieldElement.attribute("type");
        struct field_type *ptr;
        for (ptr = field_types; ptr->str; ptr++)
        {
            if (typeStr == ptr->str)
            {
                type = ptr->type;
                break;
            }
        }
        if (type < 0)
            qWarning() << "Unknown field type" << typeStr;
        field.setType(type);

        /* field attributes */
        field.setLabel(fieldElement.attribute("label"));
        field.setKey(fieldElement.attribute("var"));

        /* field value(s) */
        if (type == QXmppDataForm::Field::BooleanField)
        {
            const QString valueStr = fieldElement.firstChildElement("value").text();
            field.setValue(valueStr == "1" || valueStr == "true");
        }
        else if (type == QXmppDataForm::Field::ListMultiField ||
            type == QXmppDataForm::Field::JidMultiField || 
            type == QXmppDataForm::Field::TextMultiField) 
        {
            QStringList values;
            QDomElement valueElement = fieldElement.firstChildElement("value");
            while (!valueElement.isNull())
            {
                values.append(valueElement.text());
                valueElement = valueElement.nextSiblingElement("value");
            }
            field.setValue(values);
        }
        else
        {
            field.setValue(fieldElement.firstChildElement("value").text());
        }

        /* field options */
        if (type == QXmppDataForm::Field::ListMultiField ||
            type == QXmppDataForm::Field::ListSingleField)
        { 
            QList<QPair<QString, QString> > options;
            QDomElement optionElement = fieldElement.firstChildElement("option");
            while (!optionElement.isNull())
            {
                options.append(QPair<QString, QString>(optionElement.attribute("label"),
                    optionElement.firstChildElement("value").text()));
                optionElement = optionElement.nextSiblingElement("option"); 
            }
            field.setOptions(options);
        }

        /* other properties */
        field.setDescription(fieldElement.firstChildElement("description").text());
        field.setRequired(!fieldElement.firstChildElement("required").isNull());

        m_fields.append(field);

        fieldElement = fieldElement.nextSiblingElement("field");
    }
}

void QXmppDataForm::toXml(QXmlStreamWriter *writer) const
{
    if (isNull())
        return;

    writer->writeStartElement("x");
    helperToXmlAddAttribute(writer, "xmlns", ns_data);

    /* form type */
    QString typeStr;
    if (m_type == QXmppDataForm::Form)
        typeStr = "form";
    else if (m_type == QXmppDataForm::Submit)
        typeStr = "submit";
    else if (m_type == QXmppDataForm::Cancel)
        typeStr = "cancel";
    else if (m_type == QXmppDataForm::Result)
        typeStr = "result";
    helperToXmlAddAttribute(writer, "type", typeStr);

    /* form properties */
    if (!m_title.isEmpty())
        helperToXmlAddTextElement(writer, "title", m_title);
    if (!m_instructions.isEmpty())
        helperToXmlAddTextElement(writer, "instructions", m_instructions);

   
    foreach (const QXmppDataForm::Field &field, m_fields)
    {
        writer->writeStartElement("field");

        /* field type */
        const QXmppDataForm::Field::Type type = field.type();
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
        helperToXmlAddAttribute(writer, "type", typeStr);

        /* field attributes */
        helperToXmlAddAttribute(writer, "label", field.label());
        helperToXmlAddAttribute(writer, "var", field.key());

        /* field value(s) */
        if (type == QXmppDataForm::Field::BooleanField)
        {
            helperToXmlAddTextElement(writer, "value", field.value().toBool() ? "1" : "0");
        } 
        else if (type == QXmppDataForm::Field::ListMultiField ||
            type == QXmppDataForm::Field::JidMultiField ||
            type == QXmppDataForm::Field::TextMultiField)
        {
            foreach (const QString &value, field.value().toStringList())
                helperToXmlAddTextElement(writer, "value", value);
        }
        else
        {
            helperToXmlAddTextElement(writer, "value", field.value().toString());
        }

        /* field options */
        if (type == QXmppDataForm::Field::ListMultiField ||
            type == QXmppDataForm::Field::ListSingleField)
        {
            QPair<QString, QString> option;
            foreach (option, field.options())
            {
                writer->writeStartElement("option");
                helperToXmlAddAttribute(writer, "label", option.first);
                helperToXmlAddTextElement(writer, "value", option.second);
                writer->writeEndElement();
            }
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

