#include "rosterItem.h"
#include <QImage>

rosterItem::rosterItem(const QString& bareJid) //: QStandardItem(bareJid)
{
    setStatusType(QXmppPresence::Status::Offline);
    setStatusText("Offline");
}

void rosterItem::setName(const QString& name)
{
    setText(name);
}

QString rosterItem::getName()
{
    return text();
}

void rosterItem::setBareJid(const QString& bareJid)
{
    setData(bareJid, rosterItem::BareJid);
}

void rosterItem::setStatusText(const QString& text)
{
    setData(text, rosterItem::StatusText);
}

QString rosterItem::getBareJid()
{
    return data(rosterItem::BareJid).toString();
}

QString rosterItem::getStatusText()
{
    return data(rosterItem::StatusText).toString();
}

void rosterItem::setStatusType(QXmppPresence::Status::Type type)
{
    setData(static_cast<int>(type), StatusType);
    QString icon;
    switch(type)
    {
    case QXmppPresence::Status::Online:
    case QXmppPresence::Status::Chat:
        icon = "green";
        break;
    case QXmppPresence::Status::Away:
    case QXmppPresence::Status::XA:
        icon = "orange";
        break;
    case QXmppPresence::Status::DND:
        icon = "red";
        break;
    case QXmppPresence::Status::Invisible:
    case QXmppPresence::Status::Offline:
        icon = "gray";
        break;
    }
    if(!icon.isEmpty())
        setIcon(QIcon(":/icons/resource/"+icon+".png"));
}

QXmppPresence::Status::Type rosterItem::getStatusType()
{
    return static_cast<QXmppPresence::Status::Type>(data(StatusType).toInt());
}

void rosterItem::setPresenceType(QXmppPresence::Type type)
{
    setData(static_cast<int>(type), PresenceType);
    QString icon;
    switch(type)
    {
    case QXmppPresence::Available:
        break;
    case QXmppPresence::Unavailable:
        icon = "gray";
        break;
    case QXmppPresence::Error:
    case QXmppPresence::Subscribe:
    case QXmppPresence::Subscribed:
    case QXmppPresence::Unsubscribe:
    case QXmppPresence::Unsubscribed:
    case QXmppPresence::Probe:
        break;
    }
    if(!icon.isEmpty())
        setIcon(QIcon(":/icons/resource/"+icon+".png"));
}

QXmppPresence::Type rosterItem::getPresenceType()
{
    return static_cast<QXmppPresence::Type>(data(PresenceType).toInt());
}

void rosterItem::setAvatar(const QImage& image)
{
    setData(QVariant(image), rosterItem::Avatar);
}

QImage rosterItem::getAvatar()
{
    return qvariant_cast<QImage>(data(rosterItem::Avatar));
}
