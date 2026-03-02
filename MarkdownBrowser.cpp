#include "MarkdownBrowser.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QTextDocument>
#include <QTimer>

MarkdownBrowser::MarkdownBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    m_nam = new QNetworkAccessManager(this);
    m_nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    m_redrawTimer = new QTimer(this);
    m_redrawTimer->setSingleShot(true);
    m_redrawTimer->setInterval(50);
    connect(m_redrawTimer, &QTimer::timeout, this, [this]() {
        emit logMessage("[timer] imagesUpdated fired → triggering re-render");
        emit imagesUpdated();
    });
}

QImage MarkdownBrowser::makePlaceholder(const QUrl &url) const
{
    const int W = 512, H = 512;
    QImage img(W, H, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);

    const QPalette &pal = palette();
    const bool isDark   = pal.color(QPalette::Window).lightness() < 128;
    const QColor bg     = pal.color(QPalette::AlternateBase);
    const QColor border = isDark ? QColor(192, 192, 192)   // silver
                                 : QColor(128, 128, 128);  // gray
    const QColor text   = pal.color(QPalette::Text);
    const QColor muted  = pal.color(QPalette::PlaceholderText);

    p.setBrush(bg);
    p.setPen(QPen(border, 2));
    p.drawRoundedRect(QRectF(1, 1, W - 2, H - 2), 8, 8);

    // Warning symbol — large, centered in the upper half
    QFont fIcon = font();
    fIcon.setPointSize(72);
    p.setFont(fIcon);
    p.setPen(text);
    p.drawText(QRect(0, 80, W, 160), Qt::AlignHCenter | Qt::AlignVCenter, "\u26a0");

    // "Image unavailable" label
    QFont fLabel = font();
    fLabel.setPointSize(18);
    fLabel.setBold(true);
    p.setFont(fLabel);
    p.setPen(text);
    p.drawText(QRect(20, 280, W - 40, 60), Qt::AlignHCenter | Qt::AlignVCenter,
               "Image unavailable");

    // URL — elided, muted, below the label
    QFont fUrl = font();
    fUrl.setPointSize(11);
    p.setFont(fUrl);
    p.setPen(muted);
    const QString elided = QFontMetrics(fUrl).elidedText(
        url.toString(), Qt::ElideMiddle, W - 60);
    p.drawText(QRect(30, 360, W - 60, 40), Qt::AlignHCenter | Qt::AlignVCenter, elided);

    p.end();
    return img;
}

QVariant MarkdownBrowser::loadResource(int type, const QUrl &url)
{
    if (type == QTextDocument::ImageResource &&
        (url.scheme() == QLatin1String("http") || url.scheme() == QLatin1String("https")))
    {
        // Cache hit
        if (m_imageCache.contains(url)) {
            const QImage &img = m_imageCache[url];
            if (img.isNull()) {
                // Failed fetch — return a generated placeholder (palette-aware,
                // so it's regenerated correctly after dark/light mode switches)
                return makePlaceholder(url);
            }
            emit logMessage(QString("[cache] hit %1x%2 → %3")
                .arg(img.width()).arg(img.height()).arg(url.toString()));
            return img;
        }

        // Kick off async fetch (dedup: ignore if already in flight)
        if (!m_fetching.contains(url)) {
            m_fetching.insert(url);
            emit logMessage("[fetch] starting: " + url.toString());
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::UserAgentHeader,
                              "Mozilla/5.0 BlinkMD/1.0");
            QNetworkReply *reply = m_nam->get(request);
            connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
                reply->deleteLater();
                m_fetching.remove(url);

                const QUrl finalUrl = reply->url();
                const int httpStatus =
                    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

                if (reply->error() != QNetworkReply::NoError) {
                    emit logMessage(QString("[fetch] FAILED HTTP %1: %2 — %3")
                                   .arg(httpStatus)
                                   .arg(reply->errorString())
                                   .arg(url.toString()));
                    m_imageCache.insert(url, QImage()); // null = failed, show placeholder
                    m_redrawTimer->start();             // re-render so placeholder appears
                    return;
                }

                const QByteArray data = reply->readAll();
                emit logMessage(QString("[fetch] received %1 bytes from %2")
                    .arg(data.size()).arg(finalUrl.toString()));

                QImage img;
                if (!img.loadFromData(data)) {
                    emit logMessage("[image] decode FAILED for: " + url.toString());
                    m_imageCache.insert(url, QImage());
                    m_redrawTimer->start();
                    return;
                }

                emit logMessage(QString("[image] decoded %1x%2 for: %3")
                    .arg(img.width()).arg(img.height()).arg(url.toString()));
                m_imageCache.insert(url, img);
                m_redrawTimer->start();
            });
        }

        return QVariant(); // placeholder while downloading
    }

    return QTextBrowser::loadResource(type, url);
}
