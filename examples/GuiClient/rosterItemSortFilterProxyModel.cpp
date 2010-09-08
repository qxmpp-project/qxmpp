/*
 * Copyright (C) 2008-2010 The QXmpp developers
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


#include "rosterItemSortFilterProxyModel.h"
#include "rosterItem.h"
#include "utils.h"

rosterItemSortFilterProxyModel::rosterItemSortFilterProxyModel(QObject* parent):
                QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setFilterRole(Qt::DisplayRole);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

bool rosterItemSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    int leftPresenceType = sourceModel()->data(left, rosterItem::PresenceType).toInt();
    int leftStatusType = sourceModel()->data(left, rosterItem::StatusType).toInt();
    int rightPresenceType = sourceModel()->data(right, rosterItem::PresenceType).toInt();
    int rightStatusType = sourceModel()->data(right, rosterItem::StatusType).toInt();

    if(leftPresenceType == rightPresenceType)
    {
        if(leftStatusType == rightStatusType)
        {
            // based on display text
            int compare = left.data().toString().compare(right.data().toString(), Qt::CaseInsensitive);
            if(compare < 0)
                return true;
            else
                return false;
        }
        else
        {
            return comparisonWeightsPresenceStatusType(static_cast<QXmppPresence::Status::Type>(leftStatusType)) <
                    comparisonWeightsPresenceStatusType(static_cast<QXmppPresence::Status::Type>(rightStatusType));
        }
    }
    else
        return comparisonWeightsPresenceType(static_cast<QXmppPresence::Type>(leftPresenceType)) <
                comparisonWeightsPresenceType(static_cast<QXmppPresence::Type>(rightPresenceType));
}

