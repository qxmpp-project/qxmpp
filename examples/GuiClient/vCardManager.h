#ifndef VCARDMANAGER_H
#define VCARDMANAGER_H

#include <QObject>
#include <QMap>
#include <QImage>
#include "QXmppVCard.h"

// use sqlite

class QXmppClient;

class vCardManager : public QObject
{
    Q_OBJECT

public:
    class vCard
    {
    public:
        QString imageHash;
        QImage image;
        QImage imageOriginal;
    };

    vCardManager(QXmppClient* client);
    void requestVCard(const QString& bareJid);
//    bool isVCardReceived(const QString& bareJid);
    bool isVCardAvailable(const QString& bareJid);

    vCardManager::vCard& getVCard(const QString& bareJid);
    void loadAllFromCache();
    void saveToCache(const QString& bareJid);
    QString getSelfFullName();

signals:
    void vCardReadyToUse(const QString& bareJid);

public slots:
    void vCardReceived(const QXmppVCard&);

private:
    QString m_selfFullName;
    QXmppClient* m_client;

//    QMap<QString, QXmppVCard> m_mapBareJidVcard;
    QMap<QString, vCardManager::vCard> m_mapBareJidVCard;
};

#endif // VCARDMANAGER_H
