#ifndef ACCOUNTSCACHE_H
#define ACCOUNTSCACHE_H

#include <QObject>
#include <QDomElement>
#include <QStringList>

class accountsCache : public QObject
{
    Q_OBJECT

public:
    explicit accountsCache(QObject *parent);
    QStringList getBareJids();
    QString getPassword(const QString& bareJid);

    void addAccount(const QString& bareJid, const QString& passwd);

public:
    void loadFromFile();

private:
    void saveToFile();

    QDomDocument m_accountsDocument;
};

#endif // ACCOUNTSCACHE_H
