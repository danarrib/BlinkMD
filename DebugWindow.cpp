#include "DebugWindow.h"

#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QFont>

DebugWindow::DebugWindow(QWidget *parent)
    : QDialog(parent, Qt::Window)
{
    setWindowTitle("Debug Log — BlinkMD");
    resize(700, 400);

    m_log = new QPlainTextEdit(this);
    m_log->setReadOnly(true);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(10);
    m_log->setFont(font);

    auto *clearBtn = new QPushButton(tr("Clear"), this);
    connect(clearBtn, &QPushButton::clicked, m_log, &QPlainTextEdit::clear);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(clearBtn);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_log);
    layout->addLayout(btnLayout);
}

void DebugWindow::appendMessage(const QString &msg)
{
    const QString ts = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    m_log->appendPlainText(ts + "  " + msg);
}
