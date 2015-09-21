#ifndef RANK_GROUP_TABLE_H_
#define RANK_GROUP_TABLE_H_

#include <QtGui/QTableView>
#include <QtGui/QStandardItemModel>

class RankGroupTable : public QTableView
{
    Q_OBJECT

public:
    RankGroupTable();
    ~RankGroupTable();

    void AddGroupInfo(int id, bool is_axis, QString name, QColor color, int number);

private:
    QStandardItemModel* data_model_;

    void InitTable();
};

#endif