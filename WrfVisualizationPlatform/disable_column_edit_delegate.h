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

/// Ӧ�ô� delegate ���н���Ϊ����˫���༭���༭�������������Ӧʵ��
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