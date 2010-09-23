#include "accountsCache.h"
#include "utils.h"
#include <QDir>
#include <QTextStream>

accountsCache::accountsCache(QObject *parent) :
    QObject(parent)
{
}

QStringList accountsCache::getBareJids()
{
    QStringList list;
    QDomElement element = m_accountsElement.firstChildElement("account");
    while(!element.isNull())
    {
        list << element.firstChildElement("bareJid").text();
        element = element.nextSiblingElement("account");
    }

    return list;
}

QString accountsCache::getPassword(const QString& bareJid)
{
    QDomElement element = m_accountsElement.firstChildElement("account");
    while(!element.isNull())
    {
        if(element.firstChildElement("bareJid").text() == bareJid)
        {
            return element.firstChildElement("password").text();
        }
        element = element.nextSiblingElement("account");
    }

    return "";
}

void accountsCache::addAccount(const QString& bareJid, const QString& passwd)
{
    QDomElement element = m_accountsElement.firstChildElement("account");
    while(!element.isNull())
    {
        if(element.firstChildElement("bareJid").text() == bareJid)
        {
            element.firstChildElement("password").setNodeValue(passwd);
            return;
        }
        element = element.nextSiblingElement("account");
    }

    QDomElement newElement;
    newElement.setTagName("account");

    QDomElement newElementBareJid;
    newElementBareJid.setTagName("bareJid");
    newElementBareJid.setNodeValue(bareJid);

    QDomElement newElementPasswd;
    newElementPasswd.setTagName("password");
    newElementPasswd.setNodeValue(passwd);

    newElement.appendChild(newElementBareJid);
    newElement.appendChild(newElementPasswd);

    m_accountsElement.appendChild(newElement);
}

void accountsCache::loadFromFile()
{
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
        m_accountsElement.save(tstream, 2);
        file.close();
    }
}
