/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#ifndef QXMPPSRVINFO_P_H
#define QXMPPSRVINFO_P_H

#include <QRunnable>
#include <QThreadPool>

class QXmppSrvInfo;

class QXmppSrvInfoLookupManager : public QThreadPool
{
    Q_OBJECT

public:
    QXmppSrvInfoLookupManager();

private slots:
    void waitForThreadPoolDone() { waitForDone(); }
};

class QXmppSrvInfoLookupRunnable : public QObject, public QRunnable
{
    Q_OBJECT

public:
    QXmppSrvInfoLookupRunnable(const QString &name)
        : lookupName(name)
    { }
    void run();

signals:
    void foundInfo(const QXmppSrvInfo &info);

private:
    QString lookupName;
};

#endif
