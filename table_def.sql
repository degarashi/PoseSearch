--------------- PoseSearch -----------------
CREATE TABLE Thumbnail (
	fileId		INTEGER PRIMARY KEY,
	width		INTEGER NOT NULL CHECK(width > 0),
	height		INTEGER NOT NULL CHECK(height > 0),
	cacheName	TEXT NOT NULL -- 同一ファイルがあった場合、ダブるのでUNIQUEにはしない
);

-------------- PoseDB ----------------------
-- ファイル情報を格納するテーブル
CREATE TABLE File (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    path        TEXT NOT NULL UNIQUE,
    size        INTEGER NOT NULL CHECK(size > 0),
    timestamp   INTEGER NOT NULL CHECK(timestamp >= 0),
    hash        BLOB NOT NULL UNIQUE CHECK(LENGTH(hash) = 64)
);

-- 各ファイル内の人物のポーズ情報を格納するテーブル
CREATE TABLE Pose (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    fileId          INTEGER NOT NULL,
    personIndex     INTEGER NOT NULL CHECK(personIndex >= 0),
    FOREIGN KEY (fileId) REFERENCES File(id),
    UNIQUE(fileId, personIndex)
);

-- ランドマークの名前を格納するテーブル (デバッグ用)
CREATE TABLE LandmarkName (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    name            TEXT NOT NULL UNIQUE
);

-- 各ポーズに対するランドマークの座標情報を格納するテーブル
CREATE TABLE Landmark (
    poseId          INTEGER NOT NULL,
    landmarkIndex   INTEGER NOT NULL,
    presence        REAL NOT NULL CHECK(presence BETWEEN 0 AND 1),
    visibility      REAL NOT NULL CHECK(visibility BETWEEN 0 AND 1),
    x               REAL NOT NULL,
    y               REAL NOT NULL,
    z               REAL NOT NULL,
    FOREIGN KEY (poseId) REFERENCES Pose(id),
    FOREIGN KEY (landmarkIndex) REFERENCES LandmarkName(id),
    UNIQUE(poseId, landmarkIndex)
);

-- torsodir の計算方法と信頼度を格納するテーブル
CREATE TABLE MasseTorsoDir (
	poseId      INTEGER PRIMARY KEY REFERENCES Pose(id),
	x           REAL NOT NULL,
	y           REAL NOT NULL,
	z           REAL NOT NULL,
	method      TEXT NOT NULL,                                  -- for debug (どのメソッドを使って算出したか)
	score       REAL NOT NULL CHECK(score BETWEEN 0 AND 1),     -- どのくらい自信を持って言えるか
	embedded    BLOB NOT NULL CHECK(LENGTH(embedded) == 4*3),   -- (x,y,z)をバイト列にした物。後の計算用(不要かも)
	yaw_x       REAL NOT NULL,
	yaw_z       REAL NOT NULL,
	embedded_yaw BLOB NOT NULL CHECK(LENGTH(embedded_yaw) == 4*2),   -- (yaw_x,yaw_z)をバイト列にした物
	pitch       REAL NOT NULL CHECK(pitch BETWEEN -1.0 AND 1.0),
	CHECK((x*x + y*y + z*z) BETWEEN 0.995 AND 1.005)            -- should be unit vector
	CHECK((yaw_x*yaw_x + yaw_z*yaw_z) BETWEEN 0.995 AND 1.005)  -- should be unit vector
);

-- 各ポーズの信頼度情報を格納するテーブル
CREATE TABLE Reliability (
    poseId              INTEGER NOT NULL UNIQUE,
    torsoHalfMin        REAL NOT NULL CHECK(torsoHalfMin BETWEEN 0 AND 1),
    faceDetect          REAL NOT NULL CHECK(faceDetect BETWEEN 0 AND 1),
    FOREIGN KEY (poseId) REFERENCES Pose(id)
);

-- torsodir をベクトルとして扱うための仮想テーブル
CREATE VIRTUAL TABLE MasseTorsoVec USING vec0(
	poseId      INTEGER NOT NULL UNIQUE,
	dir         float[3],
	yaw         float[2],
	pitch		float[1]
);

CREATE TABLE TagInfo (
	id                  INTEGER PRIMARY KEY,
	name                TEXT NOT NULL UNIQUE
);

CREATE TABLE Tags (
	poseId              INTEGER NOT NULL REFERENCES Pose(id),
	tagId               INTEGER NOT NULL REFERENCES TagInfo(id),
	PRIMARY KEY (poseId, tagId)
);
