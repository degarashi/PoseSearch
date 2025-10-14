#include "poseinfodialog.h"
#include <QEvent>
#include <QImage>
#include <QImageReader>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPolygonF>
#include <QToolTip>
#include <iterator>
#include "aux_f_q/convert.hpp"
#include "aux_f_q/image.hpp"
#include "landmark_index.hpp"
#include "poseinfo.hpp"
#include "singleton/my_db.hpp"
#include "ui_poseinfodialog.h"

namespace {
	constexpr int MAX_IMAGE_WIDTH = 640;

	// BlazePoseのランドマーク接続定義
	constexpr std::pair<BlazePoseLandmarkIndex, BlazePoseLandmarkIndex> CONNECTIONS[] = {
		// 左腕
		{BlazePoseLandmarkIndex::LEFT_SHOULDER, BlazePoseLandmarkIndex::LEFT_ELBOW},
		{BlazePoseLandmarkIndex::LEFT_ELBOW, BlazePoseLandmarkIndex::LEFT_WRIST},
		// 右腕
		{BlazePoseLandmarkIndex::RIGHT_SHOULDER, BlazePoseLandmarkIndex::RIGHT_ELBOW},
		{BlazePoseLandmarkIndex::RIGHT_ELBOW, BlazePoseLandmarkIndex::RIGHT_WRIST},
		// 肩（左右跨ぎ）
		{BlazePoseLandmarkIndex::LEFT_SHOULDER, BlazePoseLandmarkIndex::RIGHT_SHOULDER},
		// 腰（左右跨ぎ）
		{BlazePoseLandmarkIndex::LEFT_HIP, BlazePoseLandmarkIndex::RIGHT_HIP},
		// 胴体（同側）
		{BlazePoseLandmarkIndex::LEFT_SHOULDER, BlazePoseLandmarkIndex::LEFT_HIP},
		{BlazePoseLandmarkIndex::RIGHT_SHOULDER, BlazePoseLandmarkIndex::RIGHT_HIP},
		// 左脚
		{BlazePoseLandmarkIndex::LEFT_HIP, BlazePoseLandmarkIndex::LEFT_KNEE},
		{BlazePoseLandmarkIndex::LEFT_KNEE, BlazePoseLandmarkIndex::LEFT_ANKLE},
		// 右脚
		{BlazePoseLandmarkIndex::RIGHT_HIP, BlazePoseLandmarkIndex::RIGHT_KNEE},
		{BlazePoseLandmarkIndex::RIGHT_KNEE, BlazePoseLandmarkIndex::RIGHT_ANKLE},
		// 左足
		{BlazePoseLandmarkIndex::LEFT_ANKLE, BlazePoseLandmarkIndex::LEFT_HEEL},
		{BlazePoseLandmarkIndex::LEFT_HEEL, BlazePoseLandmarkIndex::LEFT_FOOT_INDEX},
		// 右足
		{BlazePoseLandmarkIndex::RIGHT_ANKLE, BlazePoseLandmarkIndex::RIGHT_HEEL},
		{BlazePoseLandmarkIndex::RIGHT_HEEL, BlazePoseLandmarkIndex::RIGHT_FOOT_INDEX},
		// 左手（手首=15と付け根）
		{BlazePoseLandmarkIndex::LEFT_WRIST, BlazePoseLandmarkIndex::LEFT_THUMB},
		{BlazePoseLandmarkIndex::LEFT_WRIST, BlazePoseLandmarkIndex::LEFT_INDEX},
		{BlazePoseLandmarkIndex::LEFT_WRIST, BlazePoseLandmarkIndex::LEFT_PINKY},
		{BlazePoseLandmarkIndex::LEFT_INDEX, BlazePoseLandmarkIndex::LEFT_PINKY},
		// 右手（手首=16と付け根）
		{BlazePoseLandmarkIndex::RIGHT_WRIST, BlazePoseLandmarkIndex::RIGHT_THUMB},
		{BlazePoseLandmarkIndex::RIGHT_WRIST, BlazePoseLandmarkIndex::RIGHT_INDEX},
		{BlazePoseLandmarkIndex::RIGHT_WRIST, BlazePoseLandmarkIndex::RIGHT_PINKY},
		{BlazePoseLandmarkIndex::RIGHT_INDEX, BlazePoseLandmarkIndex::RIGHT_PINKY},
	};
	constexpr auto CONNECTION_COUNT = std::size(CONNECTIONS);

	// 頭部ランドマーク（BlazePoseの代表的な頭部周り）
	constexpr BlazePoseLandmarkIndex HEAD_LANDMARKS[] = {
		BlazePoseLandmarkIndex::NOSE,
		BlazePoseLandmarkIndex::LEFT_EYE_INNER,
		BlazePoseLandmarkIndex::LEFT_EYE,
		BlazePoseLandmarkIndex::LEFT_EYE_OUTER,
		BlazePoseLandmarkIndex::RIGHT_EYE_INNER,
		BlazePoseLandmarkIndex::RIGHT_EYE,
		BlazePoseLandmarkIndex::RIGHT_EYE_OUTER,
		BlazePoseLandmarkIndex::LEFT_EAR,
		BlazePoseLandmarkIndex::RIGHT_EAR,
		BlazePoseLandmarkIndex::MOUTH_LEFT,
		BlazePoseLandmarkIndex::MOUTH_RIGHT,
	};
	constexpr auto HEAD_LANDMARK_COUNT = std::size(HEAD_LANDMARKS);

	// ポリゴン判定用のオフセット値
	constexpr float TORSO_OFFSET = 5.0f;
	constexpr float THIGH_OFFSET = 10.0f;
	constexpr float SHIN_OFFSET = 7.0f;
	constexpr float UPPER_ARM_OFFSET = 15.0f;
	constexpr float FOREARM_OFFSET = 15.0f;

	bool IsLeftIndex(const BlazePoseLandmarkIndex idx) {
		return idx == BlazePoseLandmarkIndex::LEFT_SHOULDER || idx == BlazePoseLandmarkIndex::LEFT_ELBOW ||
			idx == BlazePoseLandmarkIndex::LEFT_WRIST || idx == BlazePoseLandmarkIndex::LEFT_HIP || idx == BlazePoseLandmarkIndex::LEFT_KNEE ||
			idx == BlazePoseLandmarkIndex::LEFT_ANKLE || idx == BlazePoseLandmarkIndex::LEFT_HEEL ||
			idx == BlazePoseLandmarkIndex::LEFT_FOOT_INDEX || idx == BlazePoseLandmarkIndex::LEFT_PINKY ||
			idx == BlazePoseLandmarkIndex::LEFT_INDEX || idx == BlazePoseLandmarkIndex::LEFT_THUMB;
	}

	bool IsRightIndex(const BlazePoseLandmarkIndex idx) {
		return idx == BlazePoseLandmarkIndex::RIGHT_SHOULDER || idx == BlazePoseLandmarkIndex::RIGHT_ELBOW ||
			idx == BlazePoseLandmarkIndex::RIGHT_WRIST || idx == BlazePoseLandmarkIndex::RIGHT_HIP || idx == BlazePoseLandmarkIndex::RIGHT_KNEE ||
			idx == BlazePoseLandmarkIndex::RIGHT_ANKLE || idx == BlazePoseLandmarkIndex::RIGHT_HEEL ||
			idx == BlazePoseLandmarkIndex::RIGHT_FOOT_INDEX || idx == BlazePoseLandmarkIndex::RIGHT_PINKY ||
			idx == BlazePoseLandmarkIndex::RIGHT_INDEX || idx == BlazePoseLandmarkIndex::RIGHT_THUMB;
	}
	void DrawConnection(QPainter &painter, const QPointF &p1, const QPointF &p2, const bool left1, const bool right1,
						const bool left2, const bool right2, const QPen &leftPen, const QPen &rightPen) {
		if (left1 && left2) {
			painter.setPen(leftPen);
			painter.drawLine(p1, p2);
		}
		else if (right1 && right2) {
			painter.setPen(rightPen);
			painter.drawLine(p1, p2);
		}
		else {
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

	class PolygonToolTipFilter final : public QObject {
		public:
			PolygonToolTipFilter(const QPolygonF &poly, const QString &text, QObject *const parent = nullptr) :
				QObject(parent), _poly(poly), _text(text) {
			}

		protected:
			bool eventFilter(QObject *const obj, QEvent *const event) override {
				auto *const w = qobject_cast<QWidget *>(obj);
				if (!w)
					return QObject::eventFilter(obj, event);

				switch (event->type()) {
					case QEvent::MouseMove: {
						const auto *const me = static_cast<QMouseEvent *>(event);
						const QPointF p = me->position();
						const bool inside = !_poly.isEmpty() && _poly.containsPoint(p, Qt::OddEvenFill);
						if (inside && !_visible) {
							QToolTip::showText(w->mapToGlobal(me->pos()), _text, w);
							_visible = true;
						}
						else if (!inside && _visible) {
							QToolTip::hideText();
							_visible = false;
						}
						return false;
					}
					case QEvent::Leave:
						if (_visible) {
							QToolTip::hideText();
							_visible = false;
						}
						return false;
					default:
						return QObject::eventFilter(obj, event);
				}
			}

		private:
			QPolygonF _poly;
			QString _text;
			bool _visible = false; // 現在表示中かどうか
	};

	// ポリゴン作成ヘルパー
	QPolygonF MakeQuadPoly(const QPointF &p1, const QPointF &p2, float offset = 5.f) {
		return QPolygonF() << p1 << p2 << QPointF(p2.x() + offset, p2.y() + offset)
						   << QPointF(p1.x() + offset, p1.y() + offset);
	}

	// イベントフィルタ登録ヘルパー
	void InstallPolyFilter(QWidget *view, const QPolygonF &poly, const QString &text) {
		auto *filter = new PolygonToolTipFilter(poly, text, view);
		view->installEventFilter(filter);
	}

	void DrawPoseRect(QPainter &p, const QRectF &rect, const int w, const int h) {
		const QRectF qRect(rect.x() * w, rect.y() * h, rect.width() * w, rect.height() * h);
		QPolygonF poly;
		poly << qRect.topLeft() << qRect.topRight() << qRect.bottomRight() << qRect.bottomLeft();

		p.setRenderHint(QPainter::Antialiasing, true);
		const QPen rectPen(QColor(255, 165, 0), 2); // Orange のペン
		p.setPen(rectPen);
		p.setBrush(Qt::NoBrush);
		p.drawPolygon(poly);
	}

	// 指定されたランドマークインデックスのペアからポリゴンを作成し、イベントフィルタをインストールするヘルパー関数
	void CreateAndInstallSegmentPolyFilter(QWidget *const view, const PoseInfo &info, const int w, const int h,
										   const std::pair<BlazePoseLandmarkIndex, BlazePoseLandmarkIndex> &segment, const float offset,
										   const QString &segmentName) {
		const BlazePoseLandmarkIndex idx1 = segment.first;
		const BlazePoseLandmarkIndex idx2 = segment.second;

		if (static_cast<int>(idx1) < static_cast<int>(info.landmarks.size()) &&
			static_cast<int>(idx2) < static_cast<int>(info.landmarks.size())) {

			const auto &lm1 = info.landmarks[static_cast<int>(idx1)];
			const auto &lm2 = info.landmarks[static_cast<int>(idx2)];

			const QPointF p1F(lm1.x() * w, lm1.y() * h);
			const QPointF p2F(lm2.x() * w, lm2.y() * h);

			QString text = segmentName;
			if (IsLeftIndex(idx1)) {
				text = "(L)";
			}
			else if (IsRightIndex(idx1)) {
				text = "(R)";
			}
			text += segmentName;

			InstallPolyFilter(view, MakeQuadPoly(p1F, p2F, offset), text);
		}
	}
	// 骨格描画処理をまとめたメソッド
	void DrawSkeleton(QPainter &painter, const PoseInfo &info, const int w, const int h) {
		painter.setRenderHint(QPainter::Antialiasing, true);
		const QPen leftPen(QColor(0, 200, 0), 2);
		const QPen rightPen(QColor(220, 0, 0), 2);

		for (const auto &connection : CONNECTIONS) {
			const BlazePoseLandmarkIndex idx1 = connection.first;
			const BlazePoseLandmarkIndex idx2 = connection.second;

			if (static_cast<int>(idx1) >= static_cast<int>(info.landmarks.size()) ||
				static_cast<int>(idx2) >= static_cast<int>(info.landmarks.size()))
				continue;

			const auto &lm1 = info.landmarks[static_cast<int>(idx1)];
			const auto &lm2 = info.landmarks[static_cast<int>(idx2)];
			const QPointF p1F(lm1.x() * w, lm1.y() * h);
			const QPointF p2F(lm2.x() * w, lm2.y() * h);

			DrawConnection(painter, p1F, p2F, IsLeftIndex(idx1), IsRightIndex(idx1), IsLeftIndex(idx2),
						   IsRightIndex(idx2), leftPen, rightPen);
		}

		// 頭部ランドマークの描画（点）
		{
			const QPen headPen(QColor(30, 144, 255), 2); // DodgerBlue
			const QBrush headBrush(QColor(30, 144, 255, 180));
			painter.setPen(headPen);
			painter.setBrush(headBrush);
			const float r = 3.5f;

			for (size_t i = 0; i < HEAD_LANDMARK_COUNT; ++i) {
				const BlazePoseLandmarkIndex idx = HEAD_LANDMARKS[i];
				if (static_cast<int>(idx) >= static_cast<int>(info.landmarks.size()))
					continue;

				const auto &lm = info.landmarks[static_cast<int>(idx)];
				const QPointF p(lm.x() * w, lm.y() * h);
				painter.drawEllipse(QRectF(p.x() - r, p.y() - r, r * 2.0, r * 2.0));
			}
		}
	}
	void SetupPosePolygons(Ui::PoseInfoDialog *const ui, const PoseInfo &info, const int w, const int h) {
		// 胴体ポリゴン
		const BlazePoseLandmarkIndex torsoIdx[] = {BlazePoseLandmarkIndex::LEFT_SHOULDER, BlazePoseLandmarkIndex::RIGHT_SHOULDER,
										  BlazePoseLandmarkIndex::RIGHT_HIP, BlazePoseLandmarkIndex::LEFT_HIP};
		bool torsoOk = true;
		for (const BlazePoseLandmarkIndex idx : torsoIdx) {
			if (static_cast<int>(idx) >= static_cast<int>(info.landmarks.size())) {
				torsoOk = false;
				break;
			}
		}
		if (torsoOk) {
			const auto &lShoulder = info.landmarks[static_cast<int>(BlazePoseLandmarkIndex::LEFT_SHOULDER)];
			const auto &rShoulder = info.landmarks[static_cast<int>(BlazePoseLandmarkIndex::RIGHT_SHOULDER)];
			const auto &rHip = info.landmarks[static_cast<int>(BlazePoseLandmarkIndex::RIGHT_HIP)];
			const auto &lHip = info.landmarks[static_cast<int>(BlazePoseLandmarkIndex::LEFT_HIP)];
			QPolygonF torsoPoly;
			torsoPoly << QPointF(lShoulder.x() * w, lShoulder.y() * h) << QPointF(rShoulder.x() * w, rShoulder.y() * h)
					  << QPointF(rHip.x() * w, rHip.y() * h) << QPointF(lHip.x() * w, lHip.y() * h);
			InstallPolyFilter(
				ui->imageView,
				torsoPoly.subtracted(torsoPoly.translated(-TORSO_OFFSET, -TORSO_OFFSET))
					.united(torsoPoly.translated(TORSO_OFFSET, TORSO_OFFSET)),
				QStringLiteral("TORSO\n%1\n%2").arg(dg::VecToString(info.torsoDir)).arg(info.torsoMethod));
			ui->imageView->setMouseTracking(true);
		}

		// 上腕ポリゴン
		const std::pair<BlazePoseLandmarkIndex, BlazePoseLandmarkIndex> upperArmSegments[] = {
			{BlazePoseLandmarkIndex::LEFT_SHOULDER, BlazePoseLandmarkIndex::LEFT_ELBOW},
			{BlazePoseLandmarkIndex::RIGHT_SHOULDER, BlazePoseLandmarkIndex::RIGHT_ELBOW}};
		for (const auto &segment : upperArmSegments) {
			CreateAndInstallSegmentPolyFilter(ui->imageView, info, w, h, segment, UPPER_ARM_OFFSET,
											  QStringLiteral("UPPER ARM"));
		}

		// 前腕ポリゴン
		const std::pair<BlazePoseLandmarkIndex, BlazePoseLandmarkIndex> forearmSegments[] = {
			{BlazePoseLandmarkIndex::LEFT_ELBOW, BlazePoseLandmarkIndex::LEFT_WRIST},
			{BlazePoseLandmarkIndex::RIGHT_ELBOW, BlazePoseLandmarkIndex::RIGHT_WRIST}};
		for (const auto &segment : forearmSegments) {
			CreateAndInstallSegmentPolyFilter(ui->imageView, info, w, h, segment, FOREARM_OFFSET,
											  QStringLiteral("FOREARM"));
		}

		struct Tmp {
				BlazePoseLandmarkIndex lm0, lm1;
				int index;
		};
		// 大腿ポリゴン
		const Tmp thighSegments[] = {{BlazePoseLandmarkIndex::LEFT_HIP, BlazePoseLandmarkIndex::LEFT_KNEE, 0},
									 {BlazePoseLandmarkIndex::RIGHT_HIP, BlazePoseLandmarkIndex::RIGHT_KNEE, 1}};
		for (const auto &def : thighSegments) {
			CreateAndInstallSegmentPolyFilter(ui->imageView, info, w, h, {def.lm0, def.lm1}, THIGH_OFFSET,
											  QStringLiteral("THIGH\n%1\n%2")
												  .arg(dg::VecToString(info.thighDir[def.index]))
												  .arg(info.thighFlex[def.index].toString()));
		}

		// 下腿ポリゴン
		const Tmp shinSegments[] = {{BlazePoseLandmarkIndex::LEFT_KNEE, BlazePoseLandmarkIndex::LEFT_ANKLE, 0},
									{BlazePoseLandmarkIndex::RIGHT_KNEE, BlazePoseLandmarkIndex::RIGHT_ANKLE, 1}};
		for (const auto &def : shinSegments) {
			CreateAndInstallSegmentPolyFilter(ui->imageView, info, w, h, {def.lm0, def.lm1}, SHIN_OFFSET,
											  QStringLiteral("CRUS\n%1\n%2")
												  .arg(dg::VecToString(info.crusDir[def.index]))
												  .arg(info.crusFlex[def.index].toString()));
		}
	}
} // namespace

PoseInfoDialog::PoseInfoDialog(const PoseId poseId, QWidget *const parent) :
	QDialog(parent), _ui(new Ui::PoseInfoDialog) {
	_ui->setupUi(this);

	// 画像読み込み（Exifを考慮して回転）
	const QString filePath = myDb_c.getFilePath(myDb_c.getFileId(poseId));
	QImageReader reader(filePath);
	reader.setAutoTransform(false); // 自動回転はオフ
	QImage img = reader.read();
	if (img.isNull())
		return;

	// まず縮小
	if (img.width() > MAX_IMAGE_WIDTH)
		img = img.scaled(MAX_IMAGE_WIDTH, MAX_IMAGE_WIDTH, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	img = dg::RotateByExif(reader.transformation(), img);

	_ui->imageView->setFixedSize(img.size());
	adjustSize();

	// ポーズ情報取得
	const auto info = myDb_c.getPoseInfo(poseId);
	if (info.landmarks.empty())
		return;

	const int w = img.width();
	const int h = img.height();
	{
		QPainter painter(&img);
		DrawSkeleton(painter, info, w, h);

		// 姿勢推定で使われた領域を描画
		DrawPoseRect(painter, info.rect, w, h);
		painter.end();

		_ui->imageView->setFixedSize(img.size());
		_ui->imageView->setImage(img);
		_ui->imageView->update();
	}

	// 切り出したメソッドを呼び出し
	SetupPosePolygons(_ui.get(), info, w, h);
}
