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


#ifndef ROSTERITEMSORTFILTERPROXYMODEL_H
#define ROSTERITEMSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class rosterItemSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    rosterItemSortFilterProxyModel(QObject* parent = 0);

public slots:
    void setShowOfflineContacts(bool);
    void sortByName(bool);

private:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    bool filterAcceptsRow(int, const QModelIndex&) const;

    bool m_showOfflineContacts;
    bool m_sortByName;
};

#endif // ROSTERITEMSORTFILTERPROXYMODEL_H
