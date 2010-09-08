#include "utils.h"
#include <QDir>
#include <QDesktopServices>

int comparisonWeightsPresenceStatusType(QXmppPresence::Status::Type statusType)
{
    switch(statusType)
    {
        case QXmppPresence::Status::Online:
        case QXmppPresence::Status::Chat:
            return 0;
        case QXmppPresence::Status::DND:
            return 1;
        case QXmppPresence::Status::Away:
        case QXmppPresence::Status::XA:
            return 2;
        case QXmppPresence::Status::Offline:
        case QXmppPresence::Status::Invisible:
            return 3;
    }
}

int comparisonWeightsPresenceType(QXmppPresence::Type type)
{
    switch(type)
    {
        case QXmppPresence::Available:
            return 0;
        case QXmppPresence::Unavailable:
            return 1;
        case QXmppPresence::Error:
        case QXmppPresence::Subscribe:
        case QXmppPresence::Subscribed:
        case QXmppPresence::Unsubscribe:
        case QXmppPresence::Unsubscribed:
        case QXmppPresence::Probe:
            return 3;
    }
}

QString presenceToStatusText(const QXmppPresence& presence)
{
    QString statusText = presence.getStatus().getStatusText();
    if(statusText.isEmpty())
    {
        if(presence.getType() == QXmppPresence::Available)
        {
            switch(presence.getStatus().getType())
            {
            case QXmppPresence::Status::Invisible:
            case QXmppPresence::Status::Offline:
                statusText = "Offline";
                break;
            case QXmppPresence::Status::Online:
            case QXmppPresence::Status::Chat:
                statusText = "Available";
                break;
            case QXmppPresence::Status::Away:
            case QXmppPresence::Status::XA:
                statusText = "Idle";
                break;
            case QXmppPresence::Status::DND:
                statusText = "Busy";
                break;
            }
        }
        else
            statusText = "Offline";
    }

    return statusText;
}

QString getSettingsDir()
{
    return "appCache/";
}

QString getImageHash(const QByteArray& image)
{
    if(image.isEmpty())
        return "";
    else
        return QString(QCryptographicHash::hash(image,
            QCryptographicHash::Sha1).toHex());
}

QImage getImageFromByteArray(const QByteArray& image)
{
    QBuffer buffer;
    buffer.setData(image);
    buffer.open(QIODevice::ReadOnly);
    QImageReader imageReader(&buffer);
    return imageReader.read();
}

QString getImageType1(const QByteArray& image)
{
    QBuffer buffer;
    buffer.setData(image);
    buffer.open(QIODevice::ReadOnly);
    QString format = QImageReader::imageFormat(&buffer);

    if(format.toUpper() == "PNG")
        return "image/png";
    else if(format.toUpper() == "MNG")
        return "video/x-mng";
    else if(format.toUpper() == "GIF")
        return "image/gif";
    else if(format.toUpper() == "BMP")
        return "image/bmp";
    else if(format.toUpper() == "XPM")
        return "image/x-xpm";
    else if(format.toUpper() == "SVG")
        return "image/svg+xml";
    else if(format.toUpper() == "JPEG")
        return "image/jpeg";

    return "image/unknown";
}
