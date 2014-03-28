/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */


#ifndef QXMPPLOGGER_H
#define QXMPPLOGGER_H

#include <QObject>

#include "QXmppGlobal.h"

#ifdef QXMPP_LOGGABLE_TRACE
#define qxmpp_loggable_trace(x) QString("%1(0x%2) %3").arg(metaObject()->className(), QString::number(reinterpret_cast<qint64>(this), 16), x)
#else
#define qxmpp_loggable_trace(x) (x)
#endif

class QXmppLoggerPrivate;

/// \brief The QXmppLogger class represents a sink for logging messages.
///
/// \ingroup Core

class QXMPP_EXPORT QXmppLogger : public QObject
{
    Q_OBJECT
    Q_ENUMS(LoggingType)
    Q_FLAGS(MessageType MessageTypes)
    Q_PROPERTY(QString logFilePath READ logFilePath WRITE setLogFilePath)
    Q_PROPERTY(LoggingType loggingType READ loggingType WRITE setLoggingType)
    Q_PROPERTY(MessageTypes messageTypes READ messageTypes WRITE setMessageTypes)

public:
    /// This enum describes how log message are handled.
    enum LoggingType
    {
        NoLogging = 0,      ///< Log messages are discarded
        FileLogging = 1,    ///< Log messages are written to a file
        StdoutLogging = 2,  ///< Log messages are written to the standard output
        SignalLogging = 4,  ///< Log messages are emitted as a signal
    };

    /// This enum describes a type of log message.
    enum MessageType
    {
        NoMessage = 0,          ///< No message type
        DebugMessage = 1,       ///< Debugging message
        InformationMessage = 2, ///< Informational message
        WarningMessage = 4,     ///< Warning message
        ReceivedMessage = 8,    ///< Message received from server
        SentMessage = 16,       ///< Message sent to server
        AnyMessage = 31,        ///< Any message type
    };
    Q_DECLARE_FLAGS(MessageTypes, MessageType)

    QXmppLogger(QObject *parent = 0);
    ~QXmppLogger();

    static QXmppLogger* getLogger();

    QXmppLogger::LoggingType loggingType();
    void setLoggingType(QXmppLogger::LoggingType type);

    QString logFilePath();
    void setLogFilePath(const QString &path);

    QXmppLogger::MessageTypes messageTypes();
    void setMessageTypes(QXmppLogger::MessageTypes types);

public slots:
    virtual void setGauge(const QString &gauge, double value);
    virtual void updateCounter(const QString &counter, qint64 amount);

    void log(QXmppLogger::MessageType type, const QString& text);
    void reopen();

signals:
    /// This signal is emitted whenever a log message is received.
    void message(QXmppLogger::MessageType type, const QString &text);

private:
    static QXmppLogger* m_logger;
    QXmppLoggerPrivate *d;
};

/// \brief The QXmppLoggable class represents a source of logging messages.
///
/// \ingroup Core

class QXMPP_EXPORT QXmppLoggable : public QObject
{
    Q_OBJECT

public:
    QXmppLoggable(QObject *parent = 0);

protected:
    /// \cond
    virtual void childEvent(QChildEvent *event);
    /// \endcond

    /// Logs a debugging message.
    ///
    /// \param message

    void debug(const QString &message)
    {
        emit logMessage(QXmppLogger::DebugMessage, qxmpp_loggable_trace(message));
    }

    /// Logs an informational message.
    ///
    /// \param message

    void info(const QString &message)
    {
        emit logMessage(QXmppLogger::InformationMessage, qxmpp_loggable_trace(message));
    }

    /// Logs a warning message.
    ///
    /// \param message

    void warning(const QString &message)
    {
        emit logMessage(QXmppLogger::WarningMessage, qxmpp_loggable_trace(message));
    }

    /// Logs a received packet.
    ///
    /// \param message

    void logReceived(const QString &message)
    {
        emit logMessage(QXmppLogger::ReceivedMessage, qxmpp_loggable_trace(message));
    }

    /// Logs a sent packet.
    ///
    /// \param message

    void logSent(const QString &message)
    {
        emit logMessage(QXmppLogger::SentMessage, qxmpp_loggable_trace(message));
    }

signals:
    /// Sets the given \a gauge to \a value.
    void setGauge(const QString &gauge, double value);

    /// This signal is emitted to send logging messages.
    void logMessage(QXmppLogger::MessageType type, const QString &msg);

    /// Updates the given \a counter by \a amount.
    void updateCounter(const QString &counter, qint64 amount = 1);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QXmppLogger::MessageTypes)
#endif // QXMPPLOGGER_H
