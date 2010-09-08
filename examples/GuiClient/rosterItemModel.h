#ifndef ROSTERITEMMODEL_H
#define ROSTERITEMMODEL_H

#include <QStandardItemModel>
#include "rosterItem.h"
#include "QXmppRoster.h"
#include "QXmppPresence.h"

class rosterItemModel : public QStandardItemModel
{
public:
    rosterItemModel(QObject* parent);
    rosterItem* getRosterItemFromBareJid(const QString& bareJid);

    void updatePresence(const QString& bareJid, const QMap<QString, QXmppPresence>& presences);
    void updateRosterEntry(const QString& bareJid, const QXmppRosterIq::Item& rosterEntry);
    void updateAvatar(const QString& bareJid, const QImage& image);

    void clear();
private:
    QMap<QString, rosterItem*> m_jidRosterItemMap;
    void addRosterItemIfDontExist(const QString& bareJid);
};

#endif // ROSTERITEMMODEL_H
