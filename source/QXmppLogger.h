/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

/// \brief The QXmppLogger class represents a sink for logging messages. 
///
/// \ingroup Core

class QXmppLogger : public QObject
{
    Q_OBJECT

public:
    /// This enum describes how log message are handled.
    enum LoggingType
    {
        NoLogging = 0,      ///< Log messages are discarded
        FileLogging = 1,    ///< Log messages are written to a file
        StdoutLogging = 2,  ///< Log messages are written to the standard output
        SignalLogging = 4,  ///< Log messages are emitted as a signal

        // Deprecated
        /// \cond
        NONE = 0,   ///< DEPRECATED Log messages are discarded
        FILE = 1,   ///< DEPRECATED Log messages are written to a file
        STDOUT = 2  ///< DEPRECATED Log messages are written to the standard output
        /// \endcond
    };

    /// This enum describes a type of log message.
    enum MessageType
    {
        DebugMessage = 0,   ///< Debugging message
        InformationMessage, ///< Informational message
        WarningMessage,     ///< Warning message
        ReceivedMessage,    ///< Message received from server
        SentMessage,        ///< Message sent to server
    };

    QXmppLogger(QObject *parent = 0);
    static QXmppLogger* getLogger();

    QXmppLogger::LoggingType loggingType();
    void setLoggingType(QXmppLogger::LoggingType type);

    QString logFilePath();
    void setLogFilePath(const QString &path);

public slots:
    void log(QXmppLogger::MessageType type, const QString& text);

signals:
    /// This signal is emitted whenever a log message is received.
    void message(QXmppLogger::MessageType type, const QString &text);

private:
    static QXmppLogger* m_logger;
    QXmppLogger::LoggingType m_loggingType;
    QString m_logFilePath;
};

#endif // QXMPPLOGGER_H
