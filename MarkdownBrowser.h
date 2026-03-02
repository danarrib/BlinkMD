#pragma once
#include <QTextBrowser>
#include <QHash>
#include <QImage>
#include <QSet>
#include <QUrl>

class QNetworkAccessManager;
class QTimer;

class MarkdownBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit MarkdownBrowser(QWidget *parent = nullptr);

signals:
    void imagesUpdated();
    void logMessage(const QString &msg);

protected:
    QVariant loadResource(int type, const QUrl &url) override;

private:
    QImage makePlaceholder(const QUrl &url) const;

    QNetworkAccessManager *m_nam;
    QTimer                *m_redrawTimer;
    QHash<QUrl, QImage>    m_imageCache;
    QSet<QUrl>             m_fetching;
};
