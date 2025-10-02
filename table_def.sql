-- サムネイル定義 --
CREATE TABLE thumb.Thumbnail (
	fileId		INTEGER PRIMARY KEY,
	cacheName	TEXT NOT NULL UNIQUE
);

-- 姿勢解析が上手くいってないのをユーザーが手動でフラグ付けする --
CREATE TABLE blacklist.Blacklist (
	poseId		INTEGER PRIMARY KEY
);
