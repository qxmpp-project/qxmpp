/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
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


#include "vCardCache.h"
#include "utils.h"

#include "QXmppClient.h"
#include "QXmppUtils.h"
#include "QXmppVCardManager.h"

#include <QDir>
#include <QDomDocument>
#include <QCoreApplication>

vCardCache::vCardCache(QXmppClient* client) : QObject(client),
                m_client(client)
{
}

void vCardCache::vCardReceived(const QXmppVCardIq& vcard)
{
    QString from = vcard.from();

    if(from.isEmpty() && m_client)
        from = m_client->configuration().jidBare();

    m_mapBareJidVcard[from] = vcard;

    saveToFile(from);

    emit vCardReadyToUse(from);
}

bool vCardCache::isVCardAvailable(const QString& bareJid) const
{
    return m_mapBareJidVcard.contains(bareJid);
}

// TODO don't request again, if it is already requested
void vCardCache::requestVCard(const QString& bareJid)
{
    if(m_client)
        m_client->vCardManager().requestVCard(bareJid);
}

//TODO not a good way to handle
QXmppVCardIq& vCardCache::getVCard(const QString& bareJid)
{
    return m_mapBareJidVcard[bareJid];
}

void vCardCache::saveToFile(const QString& bareJid)
{
    QDir dir;
    if(!dir.exists(getSettingsDir(m_client->configuration().jidBare())))
        dir.mkpath(getSettingsDir(m_client->configuration().jidBare()));

    QDir dir2;
    if(!dir2.exists(getSettingsDir(m_client->configuration().jidBare())+ "vCards/"))
        dir2.mkpath(getSettingsDir(m_client->configuration().jidBare())+ "vCards/");

    if(m_mapBareJidVcard.contains(bareJid))
    {
        QString fileVCard = getSettingsDir(m_client->configuration().jidBare()) + "vCards/" + bareJid + ".xml";
        QFile file(fileVCard);

        if(file.open(QIODevice::ReadWrite))
        {
            QXmlStreamWriter stream(&file);
            stream.setAutoFormatting(true);
            stream.setAutoFormattingIndent(2);
            m_mapBareJidVcard[bareJid].toXml(&stream);
            file.close();
        }
    }
}

void vCardCache::loadFromFile()
{
    m_mapBareJidVcard.clear();

    QDir dirVCards(getSettingsDir(m_client->configuration().jidBare())+ "vCards/");
    if(dirVCards.exists())
    {
        QStringList list = dirVCards.entryList(QStringList("*.xml"));
        foreach(QString fileName, list)
        {
            QFile file(getSettingsDir(m_client->configuration().jidBare())+ "vCards/" + fileName);
            QString bareJid = fileName;
            bareJid.chop(4);
            if(file.open(QIODevice::ReadOnly))
            {
                QDomDocument doc;
                if(doc.setContent(&file, true))
                {
                    QXmppVCardIq vCardIq;
                    vCardIq.parse(doc.documentElement());
                    m_mapBareJidVcard[bareJid] = vCardIq;
                    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                }
            }
        }
    }
}

//TODO: this should return scaled image
QImage vCardCache::getAvatar(const QString& bareJid) const
{
    if(m_mapBareJidVcard.contains(bareJid))
        return getImageFromByteArray(m_mapBareJidVcard[bareJid].photo());
    else
        return QImage();
}

QByteArray vCardCache::getPhotoHash(const QString& bareJid) const
{
    if(!m_mapBareJidVcard.contains(bareJid))
        return QByteArray();

    if(m_mapBareJidVcard[bareJid].photo().isEmpty())
        return QByteArray();
    else
        return QCryptographicHash::hash(m_mapBareJidVcard[bareJid].photo(),
                                    QCryptographicHash::Sha1);
}
