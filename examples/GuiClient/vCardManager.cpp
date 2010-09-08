/*
 * Copyright (C) 2008-2010 The QXmpp developers
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


#include "vCardManager.h"
#include "QXmppClient.h"
#include "QXmppUtils.h"
#include "utils.h"
#include "QXmppVCardManager.h"
#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QCoreApplication>

vCardManager::vCardManager(QXmppClient* client) : QObject(client),
                m_client(client)
{
}

void vCardManager::vCardReceived(const QXmppVCard& vcard)
{
    QString from = vcard.getFrom();
    if(from.isEmpty() && m_client)
    {
        from = m_client->configuration().getJidBare();
        m_selfFullName = vcard.fullName();
    }

//    m_mapBareJidVcard[from] = vcard;

    if(m_mapBareJidVCard[from].imageHash == getImageHash(vcard.photo()))
    {
        emit vCardReadyToUse(from);
        return;
    }

    if(!m_mapBareJidVCard[from].imageHash.isEmpty())
    {
        QString fileName = getSettingsDir() + m_mapBareJidVCard[from].imageHash;
        QFile file1(fileName + "_original.png");
        QFile file2(fileName + "_scaled.png");
        file1.remove();
        file2.remove();
    }

    m_mapBareJidVCard[from].imageHash = getImageHash(vcard.photo());

    QImage image = getImageFromByteArray(vcard.photo());
    m_mapBareJidVCard[from].imageOriginal = image;

    if(!image.isNull())
        m_mapBareJidVCard[from].image = image.scaledToWidth(32);

    QString fileName = getSettingsDir() + m_mapBareJidVCard[from].imageHash;
    m_mapBareJidVCard[from].imageOriginal.save(fileName + "_original.png", "PNG");
    m_mapBareJidVCard[from].image.save(fileName + "_scaled.png", "PNG");

    saveToCache(from);

    emit vCardReadyToUse(from);
}

bool vCardManager::isVCardAvailable(const QString& bareJid)
{
    return m_mapBareJidVCard.contains(bareJid);
}

void vCardManager::requestVCard(const QString& bareJid)
{
    if(m_client)
        m_client->vCardManager().requestVCard(bareJid);
}

vCardManager::vCard& vCardManager::getVCard(const QString& bareJid)
{
    return m_mapBareJidVCard[bareJid];
}

void vCardManager::saveToCache(const QString& bareJid)
{
    QString fileBareJidImageMap = getSettingsDir() + m_client->configuration().getJidBare() + "_jidimage";
    QFile file(fileBareJidImageMap);

    if(!file.open(QIODevice::ReadWrite))
        return;

    QTextStream out(&file);
    QMap<QString, vCardManager::vCard>::const_iterator i = m_mapBareJidVCard.constBegin();
    while(i != m_mapBareJidVCard.constEnd())
    {
        out << i.key() << "\n" << i.value().imageHash << "\n";
        ++i;
    }
    file.close();

//    if(!m_mapBareJidVCard.contains(bareJid))
//        return;
//    QString fileName = getSettingsDir() + m_mapBareJidVCard[bareJid].imageHash;
//    m_mapBareJidVCard[bareJid].imageOriginal.save(fileName + "_original.png", "PNG");
//    m_mapBareJidVCard[bareJid].image.save(fileName + "_scaled.png", "PNG");
//
//    QString fileBareJidImageMap = getSettingsDir() + m_client->configuration().getJidBare() + "_jidimage.xml";
//
//    if(m_domJidImage.documentElement().isNull())
//    {
//        QDomElement root = m_domJidImage.createElement("jidimage");
//        m_domJidImage.appendChild(root);
//    }
//
//    QFile file(fileBareJidImageMap);
//    if(!file.open(QIODevice::ReadWrite))
//        return;
//
//    if(m_domJidImage.documentElement().firstChildElement(bareJid).isNull())
//    {
//        QDomElement tag = m_domJidImage.createElement(bareJid);
//        m_domJidImage.documentElement().appendChild(tag);
//        QDomText t = m_domJidImage.createTextNode(m_mapBareJidVCard[bareJid].imageHash);
//        tag.appendChild(t);
//    }
//    else
//    {
//        m_domJidImage.documentElement().firstChildElement(bareJid).firstChild().setNodeValue(m_mapBareJidVCard[bareJid].imageHash);
//    }
//
//    QTextStream out(&file);
//    m_domJidImage.save(out, 4);
//    file.close();
}

void vCardManager::loadAllFromCache()
{
    QString fileBareJidImageMap = getSettingsDir() + m_client->configuration().getJidBare() + "_jidimage";
    QFile file(fileBareJidImageMap);
    if(!file.open(QIODevice::ReadOnly))
        return;

    QString bareJid, imageHash;
    while(!file.atEnd())
    {
        bareJid = file.readLine();
        bareJid = bareJid.trimmed();
        imageHash = file.readLine();
        imageHash = imageHash.trimmed();
        m_mapBareJidVCard[bareJid].imageHash = imageHash;
        m_mapBareJidVCard[bareJid].image.load(
                getSettingsDir() + m_mapBareJidVCard[bareJid].imageHash + "_scaled.png");
        m_mapBareJidVCard[bareJid].imageOriginal.load(
                getSettingsDir() + m_mapBareJidVCard[bareJid].imageHash + "_original.png");
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    file.close();

//    QString fileBareJidImageMap = getSettingsDir() + m_client->configuration().getJidBare() + "_jidimage.xml";
//    QDomDocument doc(m_client->configuration().getJidBare() + "_jidimage");
//    QFile file(fileBareJidImageMap);
//
//    if(!file.open(QIODevice::ReadOnly))
//        return;
//
//    QString error;
//    if(m_domJidImage.setContent(&file, false, &error))
//    {
//        QDomElement nodeRecv = doc.documentElement().firstChildElement();
//        while(!nodeRecv.isNull())
//        {
//            m_mapBareJidVCard[nodeRecv.tagName()].imageHash = nodeRecv.text();
//            bool check = m_mapBareJidVCard[nodeRecv.tagName()].image.load(
//                    getSettingsDir() + m_mapBareJidVCard[nodeRecv.tagName()].imageHash + "_scaled.png");
//            m_mapBareJidVCard[nodeRecv.tagName()].imageOriginal.load(
//                    getSettingsDir() + m_mapBareJidVCard[nodeRecv.tagName()].imageHash + "_original.png");
//            nodeRecv = nodeRecv.nextSiblingElement();
//        }
//    }
}

QString vCardManager::getSelfFullName()
{
    return m_selfFullName;
}
