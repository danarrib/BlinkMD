#pragma once
#include <QMainWindow>
#include "MarkdownBrowser.h"

class QFileSystemWatcher;
class QLabel;
class DebugWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void openFile(const QString &filePath);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeEvent(QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onFileOpen();
    void onFileChanged(const QString &path);

private:
    MarkdownBrowser    *m_browser;
    QFileSystemWatcher *m_watcher;
    DebugWindow        *m_debugWindow;
    QLabel             *m_pathLabel;
    QLabel             *m_modifiedLabel;
    QLabel             *m_zoomLabel;
    QString             m_currentFilePath;
    QString             m_currentMarkdown;
    int                 m_basePointSize = 0;

    void setupMenu();
    void setupStatusBar();
    void reloadFile();
    void renderMarkdown();
    void applyDocumentStyle();
    void updateStatusBar();
    void updateZoomLabel();
    void updateTheme();
};
