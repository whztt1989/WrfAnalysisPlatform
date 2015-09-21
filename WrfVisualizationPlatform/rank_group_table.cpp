#include "rank_group_table.h"
#include "disable_column_edit_delegate.h"

RankGroupTable::RankGroupTable(){
    InitTable();
}

RankGroupTable::~RankGroupTable(){

}

void RankGroupTable::InitTable(){
    data_model_ = new QStandardItemModel;

    QStringList headers;
    headers << "ID" << "Is Axis" << "Group Name" << "Color" << "Number";
    data_model_->setHorizontalHeaderLabels(headers);
    DisableColumnEidtDelegate* dced = new DisableColumnEidtDelegate();
    this->setItemDelegateForColumn( 0, dced );
    this->setItemDelegateForColumn( 1, dced );

    this->setModel(data_model_);
}

void RankGroupTable::AddGroupInfo(int id, bool is_axis, QString name, QColor color, int number){
    int row_count = data_model_->rowCount();
    data_model_->insertRow(row_count);

    data_model_->setData(data_model_->index(row_count, 0), id, Qt::DisplayRole);
    data_model_->setData(data_model_->index(row_count, 1), is_axis? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
    data_model_->setData(data_model_->index(row_count, 2), name, Qt::DisplayRole);
    data_model_->setData(data_model_->index(row_count, 3), QVariant(color), Qt::DecorationRole);
    data_model_->setData(data_model_->index(row_count, 4), number, Qt::DisplayRole);

    this->resizeColumnsToContents();
    this->setColumnHidden(0, true);
}