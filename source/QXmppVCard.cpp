/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
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


#include "QXmppVCard.h"
#include "QXmppUtils.h"
#include "QXmppConstants.h"

#include <QXmlStreamWriter>
#include <QImage>
#include <QBuffer>

QXmppVCard::QXmppVCard(const QString& jid) : QXmppIq(QXmppIq::Get)
{
    // for self jid should be empty
    setTo(jid);
}

QString QXmppVCard::getFullName() const
{
    return m_fullName;
}

void QXmppVCard::setFullName(const QString& str)
{
    m_fullName = str;
}

const QByteArray& QXmppVCard::getPhoto() const
{
    return m_photo;
}

void QXmppVCard::setPhoto(const QByteArray& photo)
{
    m_photo = photo;
}

void QXmppVCard::setPhoto(const QImage& image)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    m_photo = ba;
}

void QXmppVCard::parse(const QDomElement& nodeRecv)
{
    QString id = nodeRecv.attribute("id");
    QString to = nodeRecv.attribute("to");
    QString from = nodeRecv.attribute("from");
    QString type = nodeRecv.attribute("type");
    setTypeFromStr(type);
    setId(id);
    setTo(to);
    setFrom(from);

    // vCard
    setFullName(nodeRecv.firstChildElement("vCard").
                firstChildElement("FN").text());
    QByteArray base64data = nodeRecv.firstChildElement("vCard").
                            firstChildElement("PHOTO").
                            firstChildElement("BINVAL").text().toAscii();
    setPhoto(QByteArray::fromBase64(base64data));
}

void QXmppVCard::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("vCard");
    helperToXmlAddAttribute(writer,"xmlns", ns_vcard);
    helperToXmlAddTextElement(writer, "FN", getFullName());

    if(!getPhoto().isEmpty())
    {
        writer->writeStartElement("PHOTO");
        helperToXmlAddTextElement(writer, "TYPE", getImageType(getPhoto()));
        helperToXmlAddTextElement(writer, "BINVAL", getPhoto().toBase64());
        writer->writeEndElement();
    }

    writer->writeEndElement();
}

QImage QXmppVCard::getPhotoAsImage() const
{
    return getImageFromByteArray(getPhoto());
}
