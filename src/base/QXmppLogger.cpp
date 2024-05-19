// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppLogger.h"

#include "StringLiterals.h"

#include <iostream>

#include <QChildEvent>
#include <QDateTime>
#include <QFile>
#include <QMetaType>
#include <QTextStream>

QXmppLogger *QXmppLogger::m_logger = nullptr;

static QStringView typeName(QXmppLogger::MessageType type)
{
    switch (type) {
    case QXmppLogger::DebugMessage:
        return u"DEBUG";
    case QXmppLogger::InformationMessage:
        return u"INFO";
    case QXmppLogger::WarningMessage:
        return u"WARNING";
    case QXmppLogger::ReceivedMessage:
        return u"RECEIVED";
    case QXmppLogger::SentMessage:
        return u"SENT";
    default:
        return {};
    }
}

static QString formatted(QXmppLogger::MessageType type, const QString &text)
{
    return QDateTime::currentDateTime().toString() + u' ' + typeName(type) + u' ' + text;
}

static void relaySignals(QXmppLoggable *from, QXmppLoggable *to)
{
    QObject::connect(from, &QXmppLoggable::logMessage,
                     to, &QXmppLoggable::logMessage);
    QObject::connect(from, &QXmppLoggable::setGauge,
                     to, &QXmppLoggable::setGauge);
    QObject::connect(from, &QXmppLoggable::updateCounter,
                     to, &QXmppLoggable::updateCounter);
}

/// Constructs a new QXmppLoggable.
///
/// \param parent

QXmppLoggable::QXmppLoggable(QObject *parent)
    : QObject(parent)
{
    auto *logParent = qobject_cast<QXmppLoggable *>(parent);
    if (logParent) {
        relaySignals(this, logParent);
    }
}

/// \cond
void QXmppLoggable::childEvent(QChildEvent *event)
{
    auto *child = qobject_cast<QXmppLoggable *>(event->child());
    if (!child) {
        return;
    }

    if (event->added()) {
        relaySignals(child, this);
    } else if (event->removed()) {
        disconnect(child, &QXmppLoggable::logMessage,
                   this, &QXmppLoggable::logMessage);
        disconnect(child, &QXmppLoggable::setGauge,
                   this, &QXmppLoggable::setGauge);
        disconnect(child, &QXmppLoggable::updateCounter,
                   this, &QXmppLoggable::updateCounter);
    }
}
/// \endcond

class QXmppLoggerPrivate
{
public:
    QXmppLoggerPrivate();

    QXmppLogger::LoggingType loggingType;
    QFile *logFile;
    QString logFilePath;
    QXmppLogger::MessageTypes messageTypes;
};

QXmppLoggerPrivate::QXmppLoggerPrivate()
    : loggingType(QXmppLogger::NoLogging),
      logFile(nullptr),
      logFilePath(u"QXmppClientLog.log"_s),
      messageTypes(QXmppLogger::AnyMessage)
{
}

///
/// Constructs a new QXmppLogger.
///
/// \param parent
///
QXmppLogger::QXmppLogger(QObject *parent)
    : QObject(parent),
      d(std::make_unique<QXmppLoggerPrivate>())
{
    // make it possible to pass QXmppLogger::MessageType between threads
    qRegisterMetaType<QXmppLogger::MessageType>("QXmppLogger::MessageType");
}

QXmppLogger::~QXmppLogger() = default;

///
/// Returns the default logger.
///
QXmppLogger *QXmppLogger::getLogger()
{
    if (!m_logger) {
        m_logger = new QXmppLogger();
    }

    return m_logger;
}

QXmppLogger::LoggingType QXmppLogger::loggingType()
{
    return d->loggingType;
}

/// Sets the handler for logging messages.
void QXmppLogger::setLoggingType(QXmppLogger::LoggingType type)
{
    if (d->loggingType != type) {
        d->loggingType = type;
        reopen();
        Q_EMIT loggingTypeChanged();
    }
}

///
/// \fn QXmppLogger::loggingTypeChanged()
///
/// Emitted when the logging type has been changed.
///
/// \since QXmpp 1.7
///

QXmppLogger::MessageTypes QXmppLogger::messageTypes()
{
    return d->messageTypes;
}

/// Sets the types of messages to log.
void QXmppLogger::setMessageTypes(QXmppLogger::MessageTypes types)
{
    if (d->messageTypes != types) {
        d->messageTypes = types;
        Q_EMIT messageTypesChanged();
    }
}

///
/// \fn QXmppLogger::messageTypesChanged()
///
/// Emitted when the message types have been changed.
///
/// \since QXmpp 1.7
///

/// Add a logging message.
void QXmppLogger::log(QXmppLogger::MessageType type, const QString &text)
{
    // filter messages
    if (!d->messageTypes.testFlag(type)) {
        return;
    }

    switch (d->loggingType) {
    case QXmppLogger::FileLogging:
        if (!d->logFile) {
            d->logFile = new QFile(d->logFilePath);
            d->logFile->open(QIODevice::WriteOnly | QIODevice::Append);
        }
        QTextStream(d->logFile) << formatted(type, text) << "\n";
        break;
    case QXmppLogger::StdoutLogging:
        std::cout << qPrintable(formatted(type, text)) << std::endl;
        break;
    case QXmppLogger::SignalLogging:
        Q_EMIT message(type, text);
        break;
    default:
        break;
    }
}

///
/// Sets the given \a gauge to \a value.
///
/// NOTE: the base implementation does nothing.
///
void QXmppLogger::setGauge(const QString &gauge, double value)
{
    Q_UNUSED(gauge)
    Q_UNUSED(value)
}

///
/// Updates the given \a counter by \a amount.
///
/// NOTE: the base implementation does nothing.
///
void QXmppLogger::updateCounter(const QString &counter, qint64 amount)
{
    Q_UNUSED(counter)
    Q_UNUSED(amount)
}

QString QXmppLogger::logFilePath()
{
    return d->logFilePath;
}

///
/// Sets the path to which logging messages should be written.
///
/// \param path
///
/// \sa setLoggingType()
///
void QXmppLogger::setLogFilePath(const QString &path)
{
    if (d->logFilePath != path) {
        d->logFilePath = path;
        reopen();
        Q_EMIT logFilePathChanged();
    }
}

///
/// \fn QXmppLogger::logFilePathChanged()
///
/// Emitted when the log file path has been changed.
///
/// \since QXmpp 1.7
///

/// If logging to a file, causes the file to be re-opened.
void QXmppLogger::reopen()
{
    if (d->logFile) {
        delete d->logFile;
        d->logFile = nullptr;
    }
}
