#ifndef MANUAL_GROUP_TABLE_H_
#define MANUAL_GROUP_TABLE_H_

#include <QtGui/QTableView>
#include <QtGui/QStandardItemModel>

class ManualGroupTable : public QTableView
{
    Q_OBJECT

public:
    ManualGroupTable();
    ~ManualGroupTable();

    void AddGroupInfo(int id, QString name, QColor color);

private:
    QStandardItemModel* data_model_;

    void InitTable();
};

#endif