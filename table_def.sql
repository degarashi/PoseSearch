-- サムネイル定義 --
CREATE TABLE thumb.Thumbnail (
	fileId		INTEGER PRIMARY KEY,
	cacheName	TEXT NOT NULL UNIQUE
);
