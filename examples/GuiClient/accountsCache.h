#ifndef ACCOUNTSCACHE_H
#define ACCOUNTSCACHE_H

#include <QObject>
#include <QDomElement>
#include <QStringList>

class accountsCache : public QObject
{
    Q_OBJECT

public:
    explicit accountsCache(QObject *parent = 0);
    QStringList getBareJids();
    QString getPassword(const QString& bareJid);

    void addAccount(const QString& bareJid, const QString& passwd);

public:
    void loadFromFile();

private:
    void saveToFile();

    QDomElement m_accountsElement;
};

#endif // ACCOUNTSCACHE_H
