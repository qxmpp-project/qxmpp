/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
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


#ifndef ACCOUNTSCACHE_H
#define ACCOUNTSCACHE_H

#include <QObject>
#include <QDomElement>

class QStringList;

class accountsCache : public QObject
{
    Q_OBJECT

public:
    explicit accountsCache(QObject *parent);
    QStringList getBareJids();
    QString getPassword(const QString& bareJid);

    void addAccount(const QString& bareJid, const QString& passwd);

public:
    void loadFromFile();

private:
    void saveToFile();

    QDomDocument m_accountsDocument;
};

#endif // ACCOUNTSCACHE_H
