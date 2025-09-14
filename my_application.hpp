#include <QApplication>

#define myApp (static_cast<MyApplication *>(QCoreApplication::instance()))
#define myApp_c (static_cast<const MyApplication *>(QCoreApplication::instance()))

class MyApplication : public QApplication {
		Q_OBJECT

	public:
		MyApplication(int &argc, char **argv);
};
