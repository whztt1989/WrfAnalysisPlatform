#ifndef MUTUAL_MATRIX_VIEWER_H_
#define MUTUAL_MATRIX_VIEWER_H_

#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <vector>

class Node;

class MutualMatrixViewer : public QWidget
{
	Q_OBJECT

public:
	MutualMatrixViewer();
	~MutualMatrixViewer();

	std::vector< QPixmap* >* stamp_pixmaps;
	std::vector< std::vector< float > >* mutual_info;
	std::vector< Node* >* stamp_items;

	void UpdateViewer();

	public slots:
		void OnSelectionChanged();

protected:
	void mouseDoubleClickEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
};

#endif