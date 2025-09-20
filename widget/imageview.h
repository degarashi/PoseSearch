#include <QImage>
#include <QPainter>
#include <QWidget>

class ImageView : public QWidget {
		QImage image;

	public:
		ImageView(QWidget *parent = nullptr);
		void setImage(const QImage& img);

	protected:
		void paintEvent(QPaintEvent *) override;
};
