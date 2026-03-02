#pragma once
#include <QDialog>

class QPlainTextEdit;

class DebugWindow : public QDialog
{
    Q_OBJECT
public:
    explicit DebugWindow(QWidget *parent = nullptr);

public slots:
    void appendMessage(const QString &msg);

private:
    QPlainTextEdit *m_log;
};
