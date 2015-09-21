///////////////////////////////////////////////////////////////////////////////
///  Copyright ( c ) Visualization Group                                        
///  Institute of Computer Graphics and Computer Aided Design                 
///  School of Software, Tsinghua University                                  
///                                                                           
///  Created by : Zhang Zhe 2013-03-21
///  Updated by : None                                            
///                                                                           
///  File : [path]\disable_column_edit_delegate.h
///////////////////////////////////////////////////////////////////////////////

#ifndef DISABLE_COLUMN_EDIT_DELEGATE_
#define DISABLE_COLUMN_EDIT_DELEGATE_

#include <QtGui/QStyledItemDelegate>

/// 应用此 delegate 的列将变为不可双击编辑，编辑方法需由鼠标响应实现
class DisableColumnEidtDelegate : public QStyledItemDelegate   
{   
    Q_OBJECT   
public:   
    DisableColumnEidtDelegate(QObject *parent = 0): QStyledItemDelegate(parent) { }   
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,   
        const QModelIndex &index) const   
    {
        return NULL;   
    }
};

#endif 