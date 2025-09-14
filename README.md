# PoseSearch

pose_make_dbで作成した姿勢データベースからGUIを通じて対応する画像を取り出すプログラム
自分用なので汎用性はありませんし未完成です。
ビルド、使用にはQt6ライブラリの他にQtのSQLiteプラグイン(qsqlite.dll)がエクステンションをロード出来るよう、ビルドしなおす必要があります。

This program retrieves corresponding images from a pose database created by `pose_make_db` via a GUI.
It is for personal use, lacks generality, and is unfinished.
To build and use this program, you need to rebuild the Qt6 library and the Qt SQLite plugin(qsqlite.dll) so that extensions can be loaded.
