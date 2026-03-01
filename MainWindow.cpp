#include "MainWindow.h"

#include <QTextBrowser>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QStatusBar>
#include <QLabel>
#include <QFileSystemWatcher>
#include <QTextDocument>
#include <QTextTable>
#include <QTextFrame>
#include <QTextCursor>
#include <QTextBlock>
#include <QDateTime>

// Recursively walks all frames and applies consistent table styling:
// 1px solid border, no cell spacing, and breathing room above/below.
static void styleFrameTables(QTextFrame *frame, const QColor &borderColor)
{
    for (auto it = frame->begin(); it != frame->end(); ++it) {
        QTextFrame *child = it.currentFrame();
        if (!child)
            continue;
        if (auto *table = qobject_cast<QTextTable *>(child)) {
            QTextTableFormat fmt = table->format();
            fmt.setBorder(1);
            fmt.setBorderBrush(borderColor);
            fmt.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
            fmt.setCellSpacing(0);
            fmt.setCellPadding(4);
            fmt.setTopMargin(12);
            fmt.setBottomMargin(12);
            table->setFormat(fmt);
        }
        styleFrameTables(child, borderColor);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("BlinkMD");
    setMinimumSize(600, 400);
    resize(900, 700);
    setAcceptDrops(true);

    m_browser = new QTextBrowser(this);
    m_browser->setOpenLinks(false);
    m_browser->viewport()->installEventFilter(this);
    setCentralWidget(m_browser);

    m_basePointSize = m_browser->font().pointSize();
    if (m_basePointSize <= 0)
        m_basePointSize = 10;

    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, &MainWindow::onFileChanged);

    setupMenu();
    setupStatusBar();
}

void MainWindow::setupMenu()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAction = fileMenu->addAction(tr("&Open..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onFileOpen);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::setupStatusBar()
{
    m_pathLabel = new QLabel(this);
    m_pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_modifiedLabel = new QLabel(this);
    m_zoomLabel = new QLabel("Zoom: 100%", this);

    // Path stretches to fill available space; modified and zoom are pinned right
    statusBar()->addWidget(m_pathLabel, 1);
    statusBar()->addPermanentWidget(m_modifiedLabel);
    statusBar()->addPermanentWidget(m_zoomLabel);
    statusBar()->setSizeGripEnabled(false);
}

void MainWindow::onFileOpen()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Open Markdown File"),
        QString(),
        tr("Markdown Files (*.md *.markdown);;All Files (*)")
    );
    if (!path.isEmpty())
        openFile(path);
}

void MainWindow::openFile(const QString &filePath)
{
    // Update file system watcher
    if (!m_currentFilePath.isEmpty())
        m_watcher->removePath(m_currentFilePath);
    m_currentFilePath = filePath;
    m_watcher->addPath(m_currentFilePath);

    reloadFile();

    const QString fileName = QFileInfo(filePath).fileName();
    setWindowTitle(fileName + " \u2014 BlinkMD");
}

void MainWindow::reloadFile()
{
    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    m_currentMarkdown = in.readAll();

    renderMarkdown();
    updateStatusBar();
}

void MainWindow::onFileChanged(const QString &path)
{
    // Editors that do atomic saves (vim, etc.) delete and recreate the file,
    // which removes it from the watcher. Re-add it if that happened.
    if (!m_watcher->files().contains(path))
        m_watcher->addPath(path);

    if (QFile::exists(path))
        reloadFile();
}

void MainWindow::renderMarkdown()
{
    m_browser->setMarkdown(m_currentMarkdown);
    applyDocumentStyle();
}

void MainWindow::applyDocumentStyle()
{
    QTextDocument *doc = m_browser->document();

    // --- Table style ---
    const QColor borderColor = m_browser->palette().color(QPalette::Mid);
    styleFrameTables(doc->rootFrame(), borderColor);

    // --- Heading margins ---
    QTextCursor cursor(doc);
    for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
        const int level = block.blockFormat().headingLevel();
        if (level == 0)
            continue;
        const qreal topMargin    = (level == 1) ? 20.0 : (level == 2) ? 16.0 : 12.0;
        const qreal bottomMargin = 6.0;
        QTextBlockFormat fmt = block.blockFormat();
        fmt.setTopMargin(topMargin);
        fmt.setBottomMargin(bottomMargin);
        cursor.setPosition(block.position());
        cursor.setBlockFormat(fmt);
    }
}

void MainWindow::updateStatusBar()
{
    if (m_currentFilePath.isEmpty()) {
        m_pathLabel->clear();
        m_modifiedLabel->clear();
        return;
    }

    m_pathLabel->setText(m_currentFilePath);

    const QDateTime modified = QFileInfo(m_currentFilePath).lastModified();
    m_modifiedLabel->setText("Modified: " + modified.toString("yyyy-MM-dd  HH:mm:ss") + "  ");
}

void MainWindow::updateZoomLabel()
{
    const int current = m_browser->font().pointSize();
    const int percent = (current > 0)
        ? qRound(100.0 * current / m_basePointSize)
        : 100;
    m_zoomLabel->setText(QString("Zoom: %1%  ").arg(percent));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_browser->viewport() && event->type() == QEvent::Wheel) {
        const auto *we = static_cast<QWheelEvent *>(event);
        if (we->modifiers() & Qt::ControlModifier)
            QTimer::singleShot(0, this, &MainWindow::updateZoomLabel);
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::ApplicationPaletteChange && !m_currentMarkdown.isEmpty())
        renderMarkdown();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        const auto urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().isLocalFile())
            event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        const QString path = urls.first().toLocalFile();
        if (!path.isEmpty())
            openFile(path);
    }
}
