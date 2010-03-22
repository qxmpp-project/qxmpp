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

class QXmppLogger : public QObject
{
    Q_OBJECT

public:
    enum LoggingType
    {
        NoLogging = 0,  ///< Log messages are discarded
        FileLogging,     ///< Log messages are written to a file
        StdoutLogging,   ///< Log messages are written to the standard output
        SignalLogging,   ///< Log messages are emitted as a signal

        // Deprecated
        NONE = 0, ///< DEPRECATED Log messages are discarded
        FILE,     ///< DEPRECATED Log messages are written to a file
        STDOUT   ///< DEPRECATED Log messages are written to the standard output
    };

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
    void setLoggingType(QXmppLogger::LoggingType);

    void setLogFilePath(const QString&);
    QString logFilePath();

    // deprecated accessors, use the form without "get" instead
    QXmppLogger::LoggingType Q_DECL_DEPRECATED getLoggingType();

public slots:
    void log(QXmppLogger::MessageType type, const QString& str);

signals:
    void message(QXmppLogger::MessageType type, const QString &str);

private:
    static QXmppLogger* m_logger;
    QXmppLogger::LoggingType m_loggingType;
    QString m_logFilePath;
};

#endif // QXMPPLOGGER_H
