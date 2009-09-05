#ifndef QXMPPRECONNECTIONMANAGER_H
#define QXMPPRECONNECTIONMANAGER_H

#include <QObject>
#include <QTimer>
#include "QXmppClient.h"

class QXmppClient;

class QXmppReconnectionManager : public QObject
{
    Q_OBJECT

public:
    QXmppReconnectionManager(QXmppClient* client);

signals:
    void reconnectingIn(int);
    void reconnectingNow();

public slots:
    void cancelReconnection();

private slots:
    void connected();
    void error(QXmppClient::Error);
    void reconnect();

private:
    int getNextReconnectingInTime();
    int m_reconnectionTries;
    QTimer m_timer;

    // reference to to client object (no ownership)
    QXmppClient* m_client;
};

#endif // QXMPPRECONNECTIONMANAGER_H
