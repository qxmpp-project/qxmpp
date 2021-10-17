/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

#ifndef QXMPPSCEENVELOPE_P_H
#define QXMPPSCEENVELOPE_P_H

#include <QDomElement>
#include <QDateTime>
#include "QXmppUtils.h"

class QXmppSceEnvelopeReader
{
public:
    QXmppSceEnvelopeReader(const QDomElement &element)
        : element(element)
    {
    }

    inline QDomElement contentElement()
    {
        return element.firstChildElement(QStringLiteral("content"));
    }
    inline QString from()
    {
        return element.firstChildElement(QStringLiteral("from")).attribute(QStringLiteral("jid"));
    }
    inline QString to()
    {
        return element.firstChildElement(QStringLiteral("to")).attribute(QStringLiteral("jid"));
    }
    inline QDateTime timestamp()
    {
        return QXmppUtils::datetimeFromString(
                    element.firstChildElement(QStringLiteral("time")).attribute(QStringLiteral("stamp")));
    }

    // rpad is usually not needed (but can be parsed manually if really needed)

private:
    const QDomElement &element;
};

class QXmppSceEnvelopeWriter
{
public:
    QXmppSceEnvelopeWriter(QXmlStreamWriter &writer)
        : writer(writer)
    {
    }

    inline void start()
    {
        writer.writeStartElement(QStringLiteral("envelope"));
        writer.writeDefaultNamespace(QStringLiteral("urn:xmpp:sce:1"));
    }
    inline void end()
    {
        writer.writeEndElement();
    }
    template<typename Functor>
    void writeContent(Functor writeContent)
    {
        writer.writeStartElement(QStringLiteral("content"));
        writeContent();
        writer.writeEndElement();
    }
    inline void writeFrom(const QString &jid)
    {
        writer.writeStartElement(QStringLiteral("from"));
        writer.writeAttribute(QStringLiteral("jid"), jid);
        writer.writeEndElement();
    }
    inline void writeTo(const QString &jid)
    {
        writer.writeStartElement(QStringLiteral("to"));
        writer.writeAttribute(QStringLiteral("jid"), jid);
        writer.writeEndElement();
    }
    inline void writeTimestamp(const QDateTime &timestamp)
    {
        writer.writeStartElement(QStringLiteral("time"));
        writer.writeAttribute(QStringLiteral("stamp"), QXmppUtils::datetimeToString(timestamp));
        writer.writeEndElement();
    }
    inline void writeRpad(const QString &value)
    {
        writer.writeStartElement(QStringLiteral("rpad"));
        writer.writeCharacters(value);
        writer.writeEndElement();
    }

private:
    QXmlStreamWriter &writer;
};

#endif // QXMPPSCEENVELOPE_P_H
