#include <QListView>

class ResultView : public QListView {
		Q_OBJECT
	public:
		explicit ResultView(QWidget *parent = nullptr);

	protected:
		void startDrag(Qt::DropActions supportedActions) override;
		void contextMenuEvent(QContextMenuEvent *event) override;
};
