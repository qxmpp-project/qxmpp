#include "rosterItemModel.h"

rosterItemModel::rosterItemModel(QObject* parent) : QStandardItemModel(parent)
{
//    addRosterItemIfDontExist("jkhjkhkhkhk");
//    addRosterItemIfDontExist("uuuu");
//    addRosterItemIfDontExist("kkkkkkk");
//    addRosterItemIfDontExist("jjjjjjjj");
}

rosterItem* rosterItemModel::getRosterItemFromBareJid(const QString& bareJid)
{
    if(m_jidRosterItemMap.contains(bareJid))
        return m_jidRosterItemMap[bareJid];
    else
        return 0;
}

void rosterItemModel::addRosterItemIfDontExist(const QString& bareJid)
{
    if(!m_jidRosterItemMap.contains(bareJid))
    {
        rosterItem* item = new rosterItem(bareJid);
        m_jidRosterItemMap[bareJid] = item;
        appendRow(item);
        item->setStatusText("Offline");
        item->setBareJid(bareJid);
    }
}

void rosterItemModel::updatePresence(const QString& bareJid, const QMap<QString, QXmppPresence>& presences)
{
    addRosterItemIfDontExist(bareJid);

    if(presences.count() > 0)
    {
        QString statusText = presences.begin().value().getStatus().getStatusText();
        QXmppPresence::Status::Type statusType = presences.begin().value().getStatus().getType();
        QXmppPresence::Type presenceType = presences.begin().value().getType();

        if(statusText.isEmpty())
        {
            if(presenceType == QXmppPresence::Available)
                statusText = "Available";
            else if(presenceType == QXmppPresence::Unavailable)
                statusText = "Offline";
        }
        getRosterItemFromBareJid(bareJid)->setStatusText(statusText);
        getRosterItemFromBareJid(bareJid)->setStatusType(statusType);
        getRosterItemFromBareJid(bareJid)->setPresenceType(presenceType);
    }
}

void rosterItemModel::updateRosterEntry(const QString& bareJid, const QXmppRosterIq::Item& rosterEntry)
{
    addRosterItemIfDontExist(bareJid);

    QString name = rosterEntry.getName();
    if(name.isEmpty())
    {
        name = bareJid;
    }

    if(getRosterItemFromBareJid(bareJid))
        getRosterItemFromBareJid(bareJid)->setName(name);
}

void rosterItemModel::updateAvatar(const QString& bareJid, const QImage& image)
{
    addRosterItemIfDontExist(bareJid);

    if(image.isNull())
        return;

    getRosterItemFromBareJid(bareJid)->setAvatar(image);
}

void rosterItemModel::clear()
{
    QStandardItemModel::clear();
    m_jidRosterItemMap.clear();
}
