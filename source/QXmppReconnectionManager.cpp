#include "QXmppReconnectionManager.h"
#include "QXmppClient.h"
#include "QXmppUtils.h"

QXmppReconnectionManager::QXmppReconnectionManager(QXmppClient* client):m_timer(this),
    m_reconnectionTries(0), m_client(client), QObject(client)
{
    m_timer.setSingleShot(true);
    bool check = connect(&m_timer, SIGNAL(timeout()), SLOT(reconnect()));
    Q_ASSERT(check);
}

void QXmppReconnectionManager::connected()
{
    m_reconnectionTries = 0;
}

void QXmppReconnectionManager::error(QXmppClient::Error error)
{   
    if(m_client && error == QXmppClient::SocketError)
    {
        int time = getNextReconnectingInTime();

        // time is in sec
        m_timer.start(time*1000);
        emit reconnectingIn(time);
    }
}

int QXmppReconnectionManager::getNextReconnectingInTime()
{
    int reconnectingIn;
    if(m_reconnectionTries < 5)
        reconnectingIn = 10;
    else if(m_reconnectionTries < 10)
        reconnectingIn = 20;
    else if(m_reconnectionTries < 15)
        reconnectingIn = 40;
    else
        reconnectingIn = 60;

    return reconnectingIn;
}

void QXmppReconnectionManager::reconnect()
{
    if(m_client)
    {
        log(QString("QXmppReconnectionManager::reconnect()"));
        emit reconnectingNow();
        m_client->connectToServer(m_client->getConfiguration());
    }
}

void QXmppReconnectionManager::cancelReconnection()
{
    m_timer.stop();
    m_reconnectionTries = 0;
}
