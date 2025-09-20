#include "poseinfodialog.h"
#include <QEvent>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPolygonF>
#include <QToolTip>
#include "aux_f/convert.hpp"
#include "landmark_index.hpp"
#include "singleton/my_db.hpp"
#include "ui_poseinfodialog.h"

namespace {
	constexpr int MAX_IMAGE_WIDTH = 640;

	// BlazePoseのランドマーク接続定義
	constexpr std::pair<LandmarkIndex, LandmarkIndex> CONNECTIONS[] = {
		// 左腕
		{LandmarkIndex::LEFT_SHOULDER, LandmarkIndex::LEFT_ELBOW},
		{LandmarkIndex::LEFT_ELBOW, LandmarkIndex::LEFT_WRIST},
		// 右腕
		{LandmarkIndex::RIGHT_SHOULDER, LandmarkIndex::RIGHT_ELBOW},
		{LandmarkIndex::RIGHT_ELBOW, LandmarkIndex::RIGHT_WRIST},
		// 肩（左右跨ぎ）
		{LandmarkIndex::LEFT_SHOULDER, LandmarkIndex::RIGHT_SHOULDER},
		// 腰（左右跨ぎ）
		{LandmarkIndex::LEFT_HIP, LandmarkIndex::RIGHT_HIP},
		// 胴体（同側）
		{LandmarkIndex::LEFT_SHOULDER, LandmarkIndex::LEFT_HIP},
		{LandmarkIndex::RIGHT_SHOULDER, LandmarkIndex::RIGHT_HIP},
		// 左脚
		{LandmarkIndex::LEFT_HIP, LandmarkIndex::LEFT_KNEE},
		{LandmarkIndex::LEFT_KNEE, LandmarkIndex::LEFT_ANKLE},
		// 右脚
		{LandmarkIndex::RIGHT_HIP, LandmarkIndex::RIGHT_KNEE},
		{LandmarkIndex::RIGHT_KNEE, LandmarkIndex::RIGHT_ANKLE},
		// 左足
		{LandmarkIndex::LEFT_ANKLE, LandmarkIndex::LEFT_HEEL},
		{LandmarkIndex::LEFT_HEEL, LandmarkIndex::LEFT_FOOT_INDEX},
		// 右足
		{LandmarkIndex::RIGHT_ANKLE, LandmarkIndex::RIGHT_HEEL},
		{LandmarkIndex::RIGHT_HEEL, LandmarkIndex::RIGHT_FOOT_INDEX},
		// 左手（手首=15と付け根）
		{LandmarkIndex::LEFT_WRIST, LandmarkIndex::LEFT_THUMB},
		{LandmarkIndex::LEFT_WRIST, LandmarkIndex::LEFT_INDEX},
		{LandmarkIndex::LEFT_WRIST, LandmarkIndex::LEFT_PINKY},
		// 右手（手首=16と付け根）
		{LandmarkIndex::RIGHT_WRIST, LandmarkIndex::RIGHT_THUMB},
		{LandmarkIndex::RIGHT_WRIST, LandmarkIndex::RIGHT_INDEX},
		{LandmarkIndex::RIGHT_WRIST, LandmarkIndex::RIGHT_PINKY},
	};
	constexpr int CONNECTION_COUNT = sizeof(CONNECTIONS) / sizeof(CONNECTIONS[0]);

	// 頭部ランドマーク（BlazePoseの代表的な頭部周り）
	constexpr LandmarkIndex HEAD_LANDMARKS[] = {
		LandmarkIndex::NOSE,
		LandmarkIndex::LEFT_EYE_INNER,
		LandmarkIndex::LEFT_EYE,
		LandmarkIndex::LEFT_EYE_OUTER,
		LandmarkIndex::RIGHT_EYE_INNER,
		LandmarkIndex::RIGHT_EYE,
		LandmarkIndex::RIGHT_EYE_OUTER,
		LandmarkIndex::LEFT_EAR,
		LandmarkIndex::RIGHT_EAR,
		LandmarkIndex::MOUTH_LEFT,
		LandmarkIndex::MOUTH_RIGHT,
	};
	constexpr int HEAD_LANDMARK_COUNT = sizeof(HEAD_LANDMARKS) / sizeof(HEAD_LANDMARKS[0]);

	// ポリゴン判定用のオフセット値
	constexpr float TORSO_OFFSET = 5.0f;
	constexpr float THIGH_OFFSET = 10.0f;
	constexpr float SHIN_OFFSET = 7.0f;
	constexpr float UPPER_ARM_OFFSET = 15.0f;
	constexpr float FOREARM_OFFSET = 15.0f;

	bool IsLeftIndex(const LandmarkIndex idx) {
		return idx == LandmarkIndex::LEFT_SHOULDER || idx == LandmarkIndex::LEFT_ELBOW ||
			idx == LandmarkIndex::LEFT_WRIST || idx == LandmarkIndex::LEFT_HIP || idx == LandmarkIndex::LEFT_KNEE ||
			idx == LandmarkIndex::LEFT_ANKLE || idx == LandmarkIndex::LEFT_HEEL ||
			idx == LandmarkIndex::LEFT_FOOT_INDEX || idx == LandmarkIndex::LEFT_PINKY ||
			idx == LandmarkIndex::LEFT_INDEX || idx == LandmarkIndex::LEFT_THUMB;
	}

	bool IsRightIndex(const LandmarkIndex idx) {
		return idx == LandmarkIndex::RIGHT_SHOULDER || idx == LandmarkIndex::RIGHT_ELBOW ||
			idx == LandmarkIndex::RIGHT_WRIST || idx == LandmarkIndex::RIGHT_HIP || idx == LandmarkIndex::RIGHT_KNEE ||
			idx == LandmarkIndex::RIGHT_ANKLE || idx == LandmarkIndex::RIGHT_HEEL ||
			idx == LandmarkIndex::RIGHT_FOOT_INDEX || idx == LandmarkIndex::RIGHT_PINKY ||
			idx == LandmarkIndex::RIGHT_INDEX || idx == LandmarkIndex::RIGHT_THUMB;
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

	// 指定されたランドマークインデックスのペアからポリゴンを作成し、イベントフィルタをインストールするヘルパー関数
	void CreateAndInstallSegmentPolyFilter(QWidget *const view, const MyDatabase::PoseInfo &info, const int w,
										   const int h, const std::pair<LandmarkIndex, LandmarkIndex> &segment,
										   const float offset, const QString &segmentName) {
		const LandmarkIndex idx1 = segment.first;
		const LandmarkIndex idx2 = segment.second;

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
	void DrawSkeleton(QPainter &painter, const MyDatabase::PoseInfo &info, const int w, const int h) {
		painter.setRenderHint(QPainter::Antialiasing, true);
		const QPen leftPen(QColor(0, 200, 0), 2);
		const QPen rightPen(QColor(220, 0, 0), 2);

		for (const auto &connection : CONNECTIONS) {
			const LandmarkIndex idx1 = connection.first;
			const LandmarkIndex idx2 = connection.second;

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

			for (int i = 0; i < HEAD_LANDMARK_COUNT; ++i) {
				const LandmarkIndex idx = HEAD_LANDMARKS[i];
				if (static_cast<int>(idx) >= static_cast<int>(info.landmarks.size()))
					continue;

				const auto &lm = info.landmarks[static_cast<int>(idx)];
				const QPointF p(lm.x() * w, lm.y() * h);
				painter.drawEllipse(QRectF(p.x() - r, p.y() - r, r * 2.0, r * 2.0));
			}
		}
	}
	void SetupPosePolygons(Ui::PoseInfoDialog *const ui, const MyDatabase::PoseInfo &info, const int w, const int h) {
		// 胴体ポリゴン
		const LandmarkIndex torsoIdx[] = {LandmarkIndex::LEFT_SHOULDER, LandmarkIndex::RIGHT_SHOULDER,
										  LandmarkIndex::RIGHT_HIP, LandmarkIndex::LEFT_HIP};
		bool torsoOk = true;
		for (const LandmarkIndex idx : torsoIdx) {
			if (static_cast<int>(idx) >= static_cast<int>(info.landmarks.size())) {
				torsoOk = false;
				break;
			}
		}
		if (torsoOk) {
			const auto &lShoulder = info.landmarks[static_cast<int>(LandmarkIndex::LEFT_SHOULDER)];
			const auto &rShoulder = info.landmarks[static_cast<int>(LandmarkIndex::RIGHT_SHOULDER)];
			const auto &rHip = info.landmarks[static_cast<int>(LandmarkIndex::RIGHT_HIP)];
			const auto &lHip = info.landmarks[static_cast<int>(LandmarkIndex::LEFT_HIP)];
			QPolygonF torsoPoly;
			torsoPoly << QPointF(lShoulder.x() * w, lShoulder.y() * h) << QPointF(rShoulder.x() * w, rShoulder.y() * h)
					  << QPointF(rHip.x() * w, rHip.y() * h) << QPointF(lHip.x() * w, lHip.y() * h);
			InstallPolyFilter(ui->imageView,
							  torsoPoly.subtracted(torsoPoly.translated(-TORSO_OFFSET, -TORSO_OFFSET))
								  .united(torsoPoly.translated(TORSO_OFFSET, TORSO_OFFSET)),
							  QStringLiteral("TORSO\n") + dg::VecToString(info.torsoDir));
			ui->imageView->setMouseTracking(true);
		}

		// 上腕ポリゴン
		const std::pair<LandmarkIndex, LandmarkIndex> upperArmSegments[] = {
			{LandmarkIndex::LEFT_SHOULDER, LandmarkIndex::LEFT_ELBOW},
			{LandmarkIndex::RIGHT_SHOULDER, LandmarkIndex::RIGHT_ELBOW}};
		for (const auto &segment : upperArmSegments) {
			CreateAndInstallSegmentPolyFilter(ui->imageView, info, w, h, segment, UPPER_ARM_OFFSET,
											  QStringLiteral("UPPER ARM"));
		}

		// 前腕ポリゴン
		const std::pair<LandmarkIndex, LandmarkIndex> forearmSegments[] = {
			{LandmarkIndex::LEFT_ELBOW, LandmarkIndex::LEFT_WRIST},
			{LandmarkIndex::RIGHT_ELBOW, LandmarkIndex::RIGHT_WRIST}};
		for (const auto &segment : forearmSegments) {
			CreateAndInstallSegmentPolyFilter(ui->imageView, info, w, h, segment, FOREARM_OFFSET,
											  QStringLiteral("FOREARM"));
		}

		struct Tmp {
				LandmarkIndex lm0, lm1;
				bool isRight;
		};
		// 大腿ポリゴン
		const Tmp thighSegments[] = {{LandmarkIndex::LEFT_HIP, LandmarkIndex::LEFT_KNEE, false},
									 {LandmarkIndex::RIGHT_HIP, LandmarkIndex::RIGHT_KNEE, true}};
		for (const auto &def : thighSegments) {
			CreateAndInstallSegmentPolyFilter(ui->imageView, info, w, h, {def.lm0, def.lm1}, THIGH_OFFSET,
											  QStringLiteral("THIGH\n") +
												  dg::VecToString(info.thighDir[static_cast<int>(def.isRight)]));
		}

		// 下腿ポリゴン
		const Tmp shinSegments[] = {{LandmarkIndex::LEFT_KNEE, LandmarkIndex::LEFT_ANKLE, false},
									{LandmarkIndex::RIGHT_KNEE, LandmarkIndex::RIGHT_ANKLE, true}};
		for (const auto &def : shinSegments) {
			CreateAndInstallSegmentPolyFilter(ui->imageView, info, w, h, {def.lm0, def.lm1}, SHIN_OFFSET,
											  QStringLiteral("CRUS\n") +
												  dg::VecToString(info.crusDir[static_cast<int>(def.isRight)]));
		}
	}
} // namespace

PoseInfoDialog::PoseInfoDialog(const int poseId, QWidget *const parent) : QDialog(parent), _ui(new Ui::PoseInfoDialog) {
	_ui->setupUi(this);

	// 画像読み込み
	const QString filePath = myDb_c.getFilePath(myDb_c.getFileId(poseId));
	QPixmap pix(filePath);
	if (pix.isNull())
		return;
	if (pix.width() > MAX_IMAGE_WIDTH) {
		pix = pix.scaled(MAX_IMAGE_WIDTH, MAX_IMAGE_WIDTH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	_ui->imageView->setFixedSize(pix.size());
	adjustSize();

	// ポーズ情報取得
	const auto info = myDb_c.getPoseInfo(poseId);
	if (info.landmarks.empty())
		return;

	QImage img = pix.toImage();
	const int w = img.width();
	const int h = img.height();
	{
		QPainter painter(&img);
		DrawSkeleton(painter, info, w, h);
		painter.end();

		_ui->imageView->setFixedSize(img.size());
		_ui->imageView->setImage(img);
		_ui->imageView->update();
	}

	// 切り出したメソッドを呼び出し
	SetupPosePolygons(_ui.get(), info, w, h);
}
