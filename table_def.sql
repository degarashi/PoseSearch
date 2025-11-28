-- サムネイル定義 --
CREATE TABLE thumb.Thumbnail (
	fileId		INTEGER PRIMARY KEY,
	cacheName	TEXT NOT NULL
);

-- 姿勢解析が上手くいってないのをユーザーが手動でフラグ付けする --
-- ポーズIdではデータベースを作り直す毎に対象が変わる可能性があるのでHashを格納
--  = ファイル単位でブラックリスト登録
CREATE TABLE blacklist.Blacklist (
    hash        BLOB NOT NULL UNIQUE,       -- SHA2(512)
    CHECK(LENGTH(hash) == 64)
);
