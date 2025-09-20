#include "poseinfodialog.h"
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include "singleton/my_db.hpp"
#include "ui_poseinfodialog.h"

namespace {
	constexpr int MAX_IMAGE_WIDTH = 640;

	// BlazePoseのランドマーク接続定義（例: MediaPipe Poseの一部）
	constexpr int CONNECTIONS[][2] = {
		{11, 13}, {13, 15}, // 左腕
		{12, 14}, {14, 16}, // 右腕
		{11, 12}, // 肩
		{23, 24}, // 腰
		{11, 23}, {12, 24}, // 胴体
		{23, 25}, {25, 27}, // 左脚
		{24, 26}, {26, 28} // 右脚
	};
	constexpr int CONNECTION_COUNT = sizeof(CONNECTIONS) / sizeof(CONNECTIONS[0]);
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
		painter.setPen(QPen(Qt::red, 2));

		// landmarksは正規化座標(0〜1)と仮定
		const int w = img.width();
		const int h = img.height();

		for (int i = 0; i < CONNECTION_COUNT; ++i) {
			const int idx1 = CONNECTIONS[i][0];
			const int idx2 = CONNECTIONS[i][1];
			if (idx1 < static_cast<int>(info.landmarks.size()) && idx2 < static_cast<int>(info.landmarks.size())) {
				const auto &lm1 = info.landmarks[idx1];
				const auto &lm2 = info.landmarks[idx2];
				const QPoint p1(static_cast<int>(lm1.x() * w), static_cast<int>(lm1.y() * h));
				const QPoint p2(static_cast<int>(lm2.x() * w), static_cast<int>(lm2.y() * h));
				painter.drawLine(p1, p2);
			}
		}
		painter.end();

		// imageViewに描画済み画像をセット
		_ui->imageView->setFixedSize(img.size());
		_ui->imageView->update();
		_ui->imageView->setImage(img);
	}
}
