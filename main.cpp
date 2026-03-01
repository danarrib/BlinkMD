#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("BlinkMD");
    app.setApplicationVersion("1.0.0");
    app.setWindowIcon(QIcon(":/icon"));

    MainWindow window;
    if (argc > 1)
        window.openFile(QString::fromLocal8Bit(argv[1]));
    window.show();
    return app.exec();
}
