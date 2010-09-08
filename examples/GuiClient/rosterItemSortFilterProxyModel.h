#ifndef ROSTERITEMSORTFILTERPROXYMODEL_H
#define ROSTERITEMSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class rosterItemSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    rosterItemSortFilterProxyModel(QObject* parent = 0);
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // ROSTERITEMSORTFILTERPROXYMODEL_H
