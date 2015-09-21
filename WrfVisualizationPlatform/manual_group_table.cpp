#include "manual_group_table.h"
#include "disable_column_edit_delegate.h"

ManualGroupTable::ManualGroupTable(){
    InitTable();
}

ManualGroupTable::~ManualGroupTable(){

}

void ManualGroupTable::AddGroupInfo(int id, QString name, QColor color){
    int row_count = data_model_->rowCount();
    data_model_->insertRow(row_count);

    data_model_->setData(data_model_->index(row_count, 0), id, Qt::DisplayRole);
    data_model_->setData(data_model_->index(row_count, 1), name, Qt::DisplayRole);
    data_model_->setData(data_model_->index(row_count, 2), QVariant(color), Qt::DecorationRole);

    this->resizeColumnsToContents();
    this->setColumnHidden(0, true);
}

void ManualGroupTable::InitTable(){
    data_model_ = new QStandardItemModel;

    QStringList headers;
    headers << "ID" << "Group Name" << "Color";
    data_model_->setHorizontalHeaderLabels(headers);

    DisableColumnEidtDelegate* dced = new DisableColumnEidtDelegate();
    this->setItemDelegateForColumn( 0, dced );

    this->setModel(data_model_);
}