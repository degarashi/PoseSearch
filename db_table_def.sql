-- 入力ファイル情報 (パス・サイズ・ハッシュなど)
CREATE TABLE File (
    id          INTEGER PRIMARY KEY,
    path        TEXT NOT NULL UNIQUE,
    size        INTEGER NOT NULL,
    timestamp   INTEGER NOT NULL,           -- UnixTime
    hash        BLOB NOT NULL UNIQUE,       -- SHA2(512)
    CHECK(size > 0),
    CHECK(timestamp >= 0),
    CHECK(LENGTH(hash) == 64)
);

-- ランドマークの名前 (デバッグ用)
CREATE TABLE LandmarkName (
    id              INTEGER PRIMARY KEY,
    name            TEXT NOT NULL UNIQUE
);

-- 各ポーズに対するランドマーク座標 (3D/2D, 信頼度付き)
CREATE TABLE Landmark (
    poseId          INTEGER NOT NULL REFERENCES Pose(id),
    landmarkIndex   INTEGER NOT NULL REFERENCES LandmarkName(id),
    presence        REAL NOT NULL,
    visibility      REAL NOT NULL,
    x               REAL NOT NULL,
    y               REAL NOT NULL,
    z               REAL NOT NULL,
    td_x            REAL NOT NULL,
    td_y            REAL NOT NULL,
    CHECK (presence BETWEEN 0 AND 1),
    CHECK (visibility BETWEEN 0 AND 1),
    PRIMARY KEY(poseId, landmarkIndex)
);

-- 下腿の方向ベクトル (左右別、単位ベクトル制約あり)
CREATE TABLE MasseCrusDir (
    poseId      INTEGER REFERENCES Pose(id),
    is_right    INTEGER NOT NULL CHECK(is_right IN (0,1)),  -- 0 = L, 1 = R
    x           REAL NOT NULL,
    y           REAL NOT NULL,
    z           REAL NOT NULL,
    PRIMARY KEY(poseId, is_right),
    CHECK((x*x + y*y + z*z) BETWEEN 0.995 AND 1.005)
);

-- 脊柱の方向ベクトル (単位ベクトル制約あり)
CREATE TABLE MasseSpineDir (
    poseId      INTEGER PRIMARY KEY REFERENCES Pose(id),
    x           REAL NOT NULL,
    y           REAL NOT NULL,
    z           REAL NOT NULL,
    CHECK((x*x + y*y + z*z) BETWEEN 0.995 AND 1.005)
);

-- 大腿の方向ベクトル (左右別、単位ベクトル制約あり)
CREATE TABLE MasseThighDir (
    poseId      INTEGER REFERENCES Pose(id),
    is_right    INTEGER NOT NULL CHECK(is_right IN (0,1)),
    x           REAL NOT NULL,
    y           REAL NOT NULL,
    z           REAL NOT NULL,
    PRIMARY KEY(poseId, is_right),
    CHECK((x*x + y*y + z*z) BETWEEN 0.995 AND 1.005)
);

-- 胴体の方向ベクトル (算出方法・信頼度・ヨー/ピッチ付き)
CREATE TABLE MasseTorsoDir (
    poseId      INTEGER PRIMARY KEY REFERENCES Pose(id),
    x           REAL NOT NULL,
    y           REAL NOT NULL,
    z           REAL NOT NULL,
    method      TEXT NOT NULL,                                  -- 算出方法
    score       REAL NOT NULL CHECK(score BETWEEN 0 AND 1),     -- 信頼度
    yaw_x       REAL NOT NULL,
    yaw_z       REAL NOT NULL,
    pitch       REAL NOT NULL CHECK(pitch BETWEEN -1.0 AND 1.0),
    CHECK((x*x + y*y + z*z) BETWEEN 0.995 AND 1.005),
    CHECK((yaw_x*yaw_x + yaw_z*yaw_z) BETWEEN 0.995 AND 1.005)
);

-- 胴体ベクトルの高速検索用 (ベクトル型)
CREATE VIRTUAL TABLE MasseTorsoVec USING vec0(
    poseId      INTEGER NOT NULL UNIQUE,
    dir         float[3],
    yaw         float[2],
    pitch       float[1]
);

-- 脊柱ベクトルの高速検索用 (ベクトル型)
CREATE VIRTUAL TABLE MasseSpineVec USING vec0(
    poseId      INTEGER NOT NULL UNIQUE,
    dir         float[3]
);

-- 全身のバウンディングボックス (正規化座標)
CREATE TABLE PoseRect (
    poseId          INTEGER PRIMARY KEY,
    x0              REAL NOT NULL,
    x1              REAL NOT NULL,
    y0              REAL NOT NULL,
    y1              REAL NOT NULL,
    CHECK (x0 <= x1),
    CHECK (y0 <= y1),
    CHECK (x0 >= 0.0 AND x1 <= 1.0),
    CHECK (y0 >= 0.0 AND y1 <= 1.0)
);

-- ファイル内の人物ごとのポーズID
CREATE TABLE Pose (
    id              INTEGER PRIMARY KEY,
    fileId          INTEGER NOT NULL REFERENCES File(id),
    personIndex     INTEGER NOT NULL,
    CHECK(personIndex >= 0),
    UNIQUE(fileId, personIndex)
);

-- ポーズの信頼性指標 (半身・顔検出率)
CREATE TABLE Reliability (
    poseId          INTEGER NOT NULL UNIQUE REFERENCES Pose(id),
    torsoHalfMin    REAL NOT NULL CHECK(torsoHalfMin BETWEEN 0 AND 1),
    faceDetect      REAL NOT NULL CHECK(faceDetect BETWEEN 0 AND 1)
);

-- タグ情報 (タグ定義)
CREATE TABLE TagInfo (
    id          INTEGER PRIMARY KEY,
    name        TEXT NOT NULL UNIQUE
);

-- ポーズとタグの対応付け
CREATE TABLE Tags (
    poseId      INTEGER NOT NULL REFERENCES Pose(id),
    tagId       INTEGER NOT NULL REFERENCES TagInfo(id),
    PRIMARY KEY (poseId, tagId)
);

-- 大腿の屈曲解析 (体幹・脊柱との関係も保持)
CREATE TABLE ThighFlexion (
    poseId      INTEGER REFERENCES Pose(id),
    is_right    INTEGER NOT NULL CHECK(is_right IN (0,1)),
    dotBody     REAL CHECK(dotBody BETWEEN -1.0 AND 1.0),
    angleRad    REAL CHECK(angleRad BETWEEN -3.141592653589793 AND 3.141592653589793),
    dotSpine    REAL CHECK(dotSpine BETWEEN -1.0 AND 1.0),
    PRIMARY KEY(poseId, is_right)
);

-- 下腿の屈曲角度 (伸展〜屈曲をラジアンで表現)
CREATE TABLE CrusFlexion (
    poseId      INTEGER REFERENCES Pose(id),
    is_right    INTEGER NOT NULL CHECK(is_right IN (0,1)),
    angleRad    REAL CHECK(angleRad BETWEEN 0.0 AND 3.141592653589793),
    PRIMARY KEY(poseId, is_right)
);
