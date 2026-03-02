#include <QApplication>
#include <QImage>
#include <QPixmap>
#include <QStyleHints>
#include "MainWindow.h"

static bool isDarkTheme()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();
    if (scheme != Qt::ColorScheme::Unknown)
        return scheme == Qt::ColorScheme::Dark;
#endif
    return qApp->palette().color(QPalette::Window).lightness() < 128;
}

static QIcon themeIcon()
{
    QImage img(":/icon");
    if (!isDarkTheme())
        img.invertPixels();
    return QIcon(QPixmap::fromImage(img));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("BlinkMD");
    app.setApplicationVersion("1.0.0");
    app.setWindowIcon(themeIcon());

    MainWindow window;
    if (argc > 1)
        window.openFile(QString::fromLocal8Bit(argv[1]));
    window.show();
    return app.exec();
}
