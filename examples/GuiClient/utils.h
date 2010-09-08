#ifndef CLIENTUTILS_H
#define CLIENTUTILS_H

#include "QXmppPresence.h"

#include <QCryptographicHash>
#include <QBuffer>
#include <QImageReader>

int comparisonWeightsPresenceStatusType(QXmppPresence::Status::Type);
int comparisonWeightsPresenceType(QXmppPresence::Type);

QString presenceToStatusText(const QXmppPresence& presence);

QString getSettingsDir();

QString getImageHash(const QByteArray& image);
QImage getImageFromByteArray(const QByteArray& image);
QString getImageType1(const QByteArray& image);

#endif // CLIENTUTILS_H
