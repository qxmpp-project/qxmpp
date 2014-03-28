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


#include "accountsCache.h"
#include "utils.h"

#include <QDir>
#include <QTextStream>
#include <QStringList>

accountsCache::accountsCache(QObject *parent) :
    QObject(parent)
{
}

QStringList accountsCache::getBareJids()
{
    QStringList list;
    QDomElement element = m_accountsDocument.documentElement().firstChildElement("account");
    while(!element.isNull())
    {
        list << element.firstChildElement("bareJid").text();
        element = element.nextSiblingElement("account");
    }

    return list;
}

QString accountsCache::getPassword(const QString& bareJid)
{
    QDomElement element = m_accountsDocument.documentElement().firstChildElement("account");
    while(!element.isNull())
    {
        if(element.firstChildElement("bareJid").text() == bareJid)
        {
            QByteArray passwdEncryptedBa = QByteArray::fromBase64(
                    element.firstChildElement("password").text().toUtf8());
            QString passwd = calculateXor(passwdEncryptedBa, bareJid.toUtf8());
            return passwd;
        }
        element = element.nextSiblingElement("account");
    }

    return "";
}

void accountsCache::addAccount(const QString& bareJid, const QString& passwd)
{
    if(m_accountsDocument.documentElement().isNull())
    {
        m_accountsDocument.appendChild(m_accountsDocument.createElement("accounts"));
    }

    QDomElement element = m_accountsDocument.documentElement().firstChildElement("account");
    while(!element.isNull())
    {
        if(element.firstChildElement("bareJid").text() == bareJid)
        {
            m_accountsDocument.documentElement().removeChild(element);
            break;
        }
        element = element.nextSiblingElement("account");
    }

    QDomElement newElement = m_accountsDocument.createElement("account");

    QDomElement newElementBareJid = m_accountsDocument.createElement("bareJid");
    newElementBareJid.appendChild(m_accountsDocument.createTextNode(bareJid));
    newElement.appendChild(newElementBareJid);

    QDomElement newElementPasswd = m_accountsDocument.createElement("password");
    newElementPasswd.appendChild(m_accountsDocument.createTextNode(
            calculateXor(passwd.toUtf8(), bareJid.toUtf8()).toBase64()));
    newElement.appendChild(newElementPasswd);

    m_accountsDocument.documentElement().appendChild(newElement);

    saveToFile();
}

void accountsCache::loadFromFile()
{
    QDir dirSettings(getSettingsDir());
    if(dirSettings.exists())
    {
        QFile file(getSettingsDir()+ "accounts.xml");
        if(file.open(QIODevice::ReadOnly))
        {
            m_accountsDocument.setContent(&file, true);
        }
    }
}

void accountsCache::saveToFile()
{
    QDir dir;
    if(!dir.exists(getSettingsDir()))
        dir.mkpath(getSettingsDir());

    QString fileAccounts = getSettingsDir() + "accounts.xml";
    QFile file(fileAccounts);
    if(file.open(QIODevice::ReadWrite))
    {
        QTextStream tstream(&file);
        m_accountsDocument.save(tstream, 2);
        file.close();
    }
}
