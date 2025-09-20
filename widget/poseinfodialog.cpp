// ------ File: poseinfodialog.cpp ------
#include "poseinfodialog.h"
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include "singleton/my_db.hpp"
#include "ui_poseinfodialog.h"

namespace {
	constexpr int MAX_IMAGE_WIDTH = 640;

	// BlazePoseのランドマーク接続定義（例: MediaPipe Poseの一部）
	// 左右にまたがる線分（肩・腰）は混色描画（左側を緑、右側を赤）で対応する
	constexpr int CONNECTIONS[][2] = {
		{11, 13}, {13, 15}, // 左腕
		{12, 14}, {14, 16}, // 右腕
		{11, 12}, // 肩（左右跨ぎ）
		{23, 24}, // 腰（左右跨ぎ）
		{11, 23}, {12, 24}, // 胴体（同側）
		{23, 25}, {25, 27}, // 左脚
		{24, 26}, {26, 28} // 右脚
	};
	constexpr int CONNECTION_COUNT = sizeof(CONNECTIONS) / sizeof(CONNECTIONS[0]);

	inline bool isLeftIndex(int idx) {
		// BlazePose 左: 11,13,15,23,25,27
		switch (idx) {
			case 11:
			case 13:
			case 15:
			case 23:
			case 25:
			case 27:
				return true;
			default:
				return false;
		}
	}

	inline bool isRightIndex(int idx) {
		// BlazePose 右: 12,14,16,24,26,28
		switch (idx) {
			case 12:
			case 14:
			case 16:
			case 24:
			case 26:
			case 28:
				return true;
			default:
				return false;
		}
	}
} // namespace

PoseInfoDialog::PoseInfoDialog(const int poseId, QWidget *const parent) : QDialog(parent), _ui(new Ui::PoseInfoDialog) {
	_ui->setupUi(this);

	// 画像ファイルパスを取得
	const QString filePath = myDb_c.getFilePath(myDb_c.getFileId(poseId));

	// 画像を読み込み
	QPixmap pix(filePath);
	if (pix.isNull()) {
		return; // 読み込み失敗時はサイズ変更せず終了
	}

	// 最大幅を超える場合は縮小（アスペクト比維持）
	if (pix.width() > MAX_IMAGE_WIDTH) {
		pix = pix.scaled(MAX_IMAGE_WIDTH, MAX_IMAGE_WIDTH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	// imageView のサイズを画像に合わせる
	_ui->imageView->setFixedSize(pix.size());

	// ダイアログ全体のサイズを調整
	adjustSize();

	const auto info = myDb_c.getPoseInfo(poseId);

	// info.landmarksにblazeposeの2dランドマークが入っているのでそれを使い、画像の上に線分で関節を描画
	if (!info.landmarks.empty()) {
		// QImageに変換して描画
		QImage img = pix.toImage();
		QPainter painter(&img);
		painter.setRenderHint(QPainter::Antialiasing, true);

		// ペンを準備（左=緑、右=赤）
		QPen leftPen(QColor(0, 200, 0), 2); // 少し落ち着いた緑
		QPen rightPen(QColor(220, 0, 0), 2);

		// landmarksは正規化座標(0〜1)と仮定
		const int w = img.width();
		const int h = img.height();

		for (int i = 0; i < CONNECTION_COUNT; ++i) {
			const int idx1 = CONNECTIONS[i][0];
			const int idx2 = CONNECTIONS[i][1];

			if (idx1 < static_cast<int>(info.landmarks.size()) && idx2 < static_cast<int>(info.landmarks.size())) {
				const auto &lm1 = info.landmarks[idx1];
				const auto &lm2 = info.landmarks[idx2];

				// 0〜1をピクセルに変換
				const QPointF p1F(lm1.x() * w, lm1.y() * h);
				const QPointF p2F(lm2.x() * w, lm2.y() * h);

				const bool left1 = isLeftIndex(idx1);
				const bool right1 = isRightIndex(idx1);
				const bool left2 = isLeftIndex(idx2);
				const bool right2 = isRightIndex(idx2);

				// 同側なら単色で描画、左右跨ぎなら中点で分割して左側を緑、右側を赤で描く
				if (left1 && left2) {
					painter.setPen(leftPen);
					painter.drawLine(p1F, p2F);
				}
				else if (right1 && right2) {
					painter.setPen(rightPen);
					painter.drawLine(p1F, p2F);
				}
				else {
					// 左右が混在（例: 肩{11,12}, 腰{23,24}）
					const QPointF mid((p1F.x() + p2F.x()) * 0.5, (p1F.y() + p2F.y()) * 0.5);

					// どちらが左側ランドマークかで分割方向を決める
					if (left1 || right2) {
						// p1が左 or p2が右 → p1-中点を緑、p2-中点を赤
						if (left1) {
							painter.setPen(leftPen);
							painter.drawLine(p1F, mid);
						}
						if (right2) {
							painter.setPen(rightPen);
							painter.drawLine(p2F, mid);
						}
					}
					else if (right1 || left2) {
						// p1が右 or p2が左 → p2-中点を緑、p1-中点を赤
						if (left2) {
							painter.setPen(leftPen);
							painter.drawLine(p2F, mid);
						}
						if (right1) {
							painter.setPen(rightPen);
							painter.drawLine(p1F, mid);
						}
					}
					else {
						// どちらにも属さない（安全策：スキップ）
						// 想定外のインデックスには描画しない
					}
				}
			}
		}
		painter.end();

		// imageViewに描画済み画像をセット
		_ui->imageView->setFixedSize(img.size());
		_ui->imageView->setImage(img);
		_ui->imageView->update();
	}
}
