#include "poseinfodialog.h"
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include "singleton/my_db.hpp"
#include "ui_poseinfodialog.h"

namespace {
	constexpr int MAX_IMAGE_WIDTH = 640;

	// BlazePoseのランドマーク接続定義
	// 手の詳細ランドマークは未対応。手首(15,16)と左右別の指付け根のみ使用
	constexpr int CONNECTIONS[][2] = {
		// 左腕
		{11, 13},
		{13, 15},
		// 右腕
		{12, 14},
		{14, 16},
		// 肩（左右跨ぎ）
		{11, 12},
		// 腰（左右跨ぎ）
		{23, 24},
		// 胴体（同側）
		{11, 23},
		{12, 24},
		// 左脚
		{23, 25},
		{25, 27},
		// 右脚
		{24, 26},
		{26, 28},
		// 左足
		{27, 29},
		{29, 31},
		// 右足
		{28, 30},
		{30, 32},

		// 左手（手首=15と付け根）
		{15, 21}, // 親指基部
		{15, 19}, // 人差し指基部
		{15, 17}, // 小指基部

		// 右手（手首=16と付け根）
		{16, 22}, // 親指基部
		{16, 20}, // 人差し指基部
		{16, 18}, // 小指基部
	};
	constexpr int CONNECTION_COUNT = sizeof(CONNECTIONS) / sizeof(CONNECTIONS[0]);

	inline bool isLeftIndex(int idx) {
		// 左側の体幹・脚・腕・手首・左手付け根
		return idx == 11 || idx == 13 || idx == 15 || idx == 23 || idx == 25 || idx == 27 || idx == 29 || idx == 31 ||
			idx == 21 || idx == 19 || idx == 17;
	}

	inline bool isRightIndex(int idx) {
		// 右側の体幹・脚・腕・手首・右手付け根
		return idx == 12 || idx == 14 || idx == 16 || idx == 24 || idx == 26 || idx == 28 || idx == 30 || idx == 32 ||
			idx == 22 || idx == 20 || idx == 18;
	}

	void drawConnection(QPainter &painter, const QPointF &p1, const QPointF &p2, bool left1, bool right1, bool left2,
						bool right2, const QPen &leftPen, const QPen &rightPen) {
		if (left1 && left2) {
			painter.setPen(leftPen);
			painter.drawLine(p1, p2);
		}
		else if (right1 && right2) {
			painter.setPen(rightPen);
			painter.drawLine(p1, p2);
		}
		else {
			// 左右が混在（例: 肩{11,12}, 腰{23,24}）
			const QPointF mid((p1.x() + p2.x()) * 0.5, (p1.y() + p2.y()) * 0.5);

			if (left1 || right2) {
				if (left1) {
					painter.setPen(leftPen);
					painter.drawLine(p1, mid);
				}
				if (right2) {
					painter.setPen(rightPen);
					painter.drawLine(p2, mid);
				}
			}
			else if (right1 || left2) {
				if (left2) {
					painter.setPen(leftPen);
					painter.drawLine(p2, mid);
				}
				if (right1) {
					painter.setPen(rightPen);
					painter.drawLine(p1, mid);
				}
			}
		}
	}
} // namespace

PoseInfoDialog::PoseInfoDialog(const int poseId, QWidget *const parent) : QDialog(parent), _ui(new Ui::PoseInfoDialog) {
	_ui->setupUi(this);

	// 画像ファイルパスを取得
	const QString filePath = myDb_c.getFilePath(myDb_c.getFileId(poseId));

	// 画像を読み込み
	QPixmap pix(filePath);
	if (pix.isNull())
		return;

	// 最大幅を超える場合は縮小（アスペクト比維持）
	if (pix.width() > MAX_IMAGE_WIDTH) {
		pix = pix.scaled(MAX_IMAGE_WIDTH, MAX_IMAGE_WIDTH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	_ui->imageView->setFixedSize(pix.size());
	adjustSize();

	const auto info = myDb_c.getPoseInfo(poseId);
	if (info.landmarks.empty())
		return;

	QImage img = pix.toImage();
	QPainter painter(&img);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QPen leftPen(QColor(0, 200, 0), 2);
	QPen rightPen(QColor(220, 0, 0), 2);

	const int w = img.width();
	const int h = img.height();

	for (int i = 0; i < CONNECTION_COUNT; ++i) {
		const int idx1 = CONNECTIONS[i][0];
		const int idx2 = CONNECTIONS[i][1];

		if (idx1 >= static_cast<int>(info.landmarks.size()) || idx2 >= static_cast<int>(info.landmarks.size())) {
			continue;
		}

		const auto &lm1 = info.landmarks[idx1];
		const auto &lm2 = info.landmarks[idx2];

		const QPointF p1F(lm1.x() * w, lm1.y() * h);
		const QPointF p2F(lm2.x() * w, lm2.y() * h);

		drawConnection(painter, p1F, p2F, isLeftIndex(idx1), isRightIndex(idx1), isLeftIndex(idx2), isRightIndex(idx2),
					   leftPen, rightPen);
	}

	painter.end();

	_ui->imageView->setFixedSize(img.size());
	_ui->imageView->setImage(img);
	_ui->imageView->update();
}
