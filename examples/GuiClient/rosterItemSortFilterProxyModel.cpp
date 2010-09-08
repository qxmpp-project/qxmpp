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

