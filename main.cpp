#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QColorDialog>
#include <QFileDialog>
#include <QPainter>
#include <qrencode.h>

class QRDisplay : public QWidget {
public:
    void setData(const QString &data, const QColor &fg, const QColor &bg,
                const QString &text, const QColor &textColor, const QImage &bgImg) {
        m_data = data;
        m_fg = fg;
        m_bg = bg;
        m_text = text;
        m_textColor = textColor;
        m_bgImg = bgImg;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);

        // Original QR code drawing - COMPLETELY UNCHANGED
        p.fillRect(rect(), m_bg);

        if (!m_data.isEmpty()) {
            if (!m_text.isEmpty()) {
                p.setPen(m_textColor);
                QFont font = p.font();
                font.setPixelSize(24);
                p.setFont(font);
                p.drawText(rect().adjusted(0, 20, 0, 0), Qt::AlignTop | Qt::AlignHCenter, m_text);
            }

            QRcode *qr = QRcode_encodeString(m_data.toUtf8(), 0, QR_ECLEVEL_H, QR_MODE_8, 1);
            if (qr) {
                const int qrSize = 300;
                const int margin = (width() - qrSize) / 2;
                const double scale = qrSize / (double)qr->width;
                const int qrTop = 70;

                if (!m_bgImg.isNull()) {
                    QRect qrRect(margin, qrTop, qrSize, qrSize);
                    p.drawImage(qrRect, m_bgImg.scaled(qrSize, qrSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                }

                p.setPen(Qt::NoPen);
                p.setBrush(m_fg);

                for (int y = 0; y < qr->width; y++) {
                    for (int x = 0; x < qr->width; x++) {
                        if (qr->data[y*qr->width + x] & 1) {
                            p.drawRect(QRectF(
                                margin + x*scale,
                                qrTop + y*scale,
                                scale, scale
                            ));
                        }
                    }
                }
                QRcode_free(qr);
            }
        }
    }

private:
    QString m_data;
    QColor m_fg = Qt::black;
    QColor m_bg = Qt::white;
    QString m_text;
    QColor m_textColor = Qt::black;
    QImage m_bgImg;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow() {
        // ONLY UI COLOR CHANGES:
        QPalette palette;
        palette.setColor(QPalette::Window, Qt::black);
        palette.setColor(QPalette::WindowText, QColor(0, 255, 255));
        palette.setColor(QPalette::Text, QColor(0, 255, 255));
        palette.setColor(QPalette::Button, Qt::black);
        palette.setColor(QPalette::ButtonText, QColor(0, 255, 255));
        palette.setColor(QPalette::Base, Qt::black);
        this->setPalette(palette);

        QWidget *central = new QWidget;
        central->setAutoFillBackground(true);
        central->setPalette(palette);
        QVBoxLayout *layout = new QVBoxLayout(central);

        // Original type selection
        QHBoxLayout *typeLayout = new QHBoxLayout;
        QLabel *typeLabel = new QLabel("Type:");
        typeLabel->setPalette(palette);
        typeLayout->addWidget(typeLabel);
        m_typeCombo = new QComboBox;
        m_typeCombo->setPalette(palette);
        m_typeCombo->addItems({"URL", "Phone", "Email", "App ID"});
        typeLayout->addWidget(m_typeCombo);
        layout->addLayout(typeLayout);

        // ONLY CHANGE: Added hint text
        m_contentInput = new QLineEdit;
        m_contentInput->setPalette(palette);
        m_contentInput->setPlaceholderText("Enter content here");
        layout->addWidget(m_contentInput);

        // ONLY CHANGE: Added hint text
        m_textInput = new QLineEdit;
        m_textInput->setPalette(palette);
        m_textInput->setPlaceholderText("Enter display text here");
        layout->addWidget(m_textInput);

        // Original color buttons
        QHBoxLayout *colorLayout = new QHBoxLayout;
        m_fgBtn = new QPushButton("QR Color");
        m_fgBtn->setPalette(palette);
        m_bgBtn = new QPushButton("Background");
        m_bgBtn->setPalette(palette);
        m_textBtn = new QPushButton("Text Color");
        m_textBtn->setPalette(palette);
        colorLayout->addWidget(m_fgBtn);
        colorLayout->addWidget(m_bgBtn);
        colorLayout->addWidget(m_textBtn);
        layout->addLayout(colorLayout);

        // Original image button
        m_bgImgBtn = new QPushButton("Set Background Image");
        m_bgImgBtn->setPalette(palette);
        layout->addWidget(m_bgImgBtn);

        // Original QR display - COMPLETELY UNCHANGED
        m_display = new QRDisplay;
        m_display->setMinimumSize(400, 400);
        layout->addWidget(m_display);

        // Original action buttons
        QHBoxLayout *btnLayout = new QHBoxLayout;
        m_genBtn = new QPushButton("Generate");
        m_genBtn->setPalette(palette);
        m_saveBtn = new QPushButton("Save");
        m_saveBtn->setPalette(palette);
        btnLayout->addWidget(m_genBtn);
        btnLayout->addWidget(m_saveBtn);
        layout->addLayout(btnLayout);

        // Original signal connections - COMPLETELY UNCHANGED
        connect(m_fgBtn, &QPushButton::clicked, this, [this](){ m_fg = QColorDialog::getColor(m_fg, this); });
        connect(m_bgBtn, &QPushButton::clicked, this, [this](){ m_bg = QColorDialog::getColor(m_bg, this); });
        connect(m_textBtn, &QPushButton::clicked, this, [this](){ m_textColor = QColorDialog::getColor(m_textColor, this); });
        connect(m_bgImgBtn, &QPushButton::clicked, this, &MainWindow::setBgImage);
        connect(m_genBtn, &QPushButton::clicked, this, &MainWindow::generate);
        connect(m_saveBtn, &QPushButton::clicked, this, &MainWindow::save);

        setCentralWidget(central);
    }

private slots:
    // Original slots - COMPLETELY UNCHANGED
    void generate() {
        QString content = m_contentInput->text();
        switch (m_typeCombo->currentIndex()) {
            case 0: if (!content.contains("://")) content.prepend("https://"); break;
            case 1: content.prepend("tel:"); break;
            case 2: content.prepend("mailto:"); break;
        }
        m_display->setData(content, m_fg, m_bg, m_textInput->text(), m_textColor, m_bgImg);
    }

    void save() {
        QString file = QFileDialog::getSaveFileName(this, "", "", "PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp)");
        if (!file.isEmpty()) {
            QImage img(400, 500, QImage::Format_ARGB32);
            img.fill(m_bg);

            QPainter p(&img);

            if (!m_textInput->text().isEmpty()) {
                p.setPen(m_textColor);
                QFont font = p.font();
                font.setPixelSize(24);
                p.setFont(font);
                p.drawText(QRect(0, 20, 400, 50), Qt::AlignCenter, m_textInput->text());
            }

            if (!m_contentInput->text().isEmpty()) {
                QRcode *qr = QRcode_encodeString(m_contentInput->text().toUtf8(), 0, QR_ECLEVEL_H, QR_MODE_8, 1);
                if (qr) {
                    const int qrSize = 300;
                    const int margin = (400 - qrSize) / 2;
                    const double scale = qrSize / (double)qr->width;
                    const int qrTop = 100;

                    if (!m_bgImg.isNull()) {
                        QRect qrRect(margin, qrTop, qrSize, qrSize);
                        p.drawImage(qrRect, m_bgImg.scaled(qrSize, qrSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                    }

                    p.setPen(Qt::NoPen);
                    p.setBrush(m_fg);
                    for (int y = 0; y < qr->width; y++) {
                        for (int x = 0; x < qr->width; x++) {
                            if (qr->data[y*qr->width + x] & 1) {
                                p.drawRect(QRectF(
                                    margin + x*scale,
                                    qrTop + y*scale,
                                    scale, scale
                                ));
                            }
                        }
                    }
                    QRcode_free(qr);
                }
            }

            img.save(file);
        }
    }

    void setBgImage() {
        QString file = QFileDialog::getOpenFileName(this, "", "", "Images (*.png *.jpg *.jpeg *.bmp)");
        if (!file.isEmpty()) m_bgImg.load(file);
    }

private:
    QComboBox *m_typeCombo;
    QLineEdit *m_contentInput;
    QLineEdit *m_textInput;
    QPushButton *m_fgBtn;
    QPushButton *m_bgBtn;
    QPushButton *m_textBtn;
    QPushButton *m_bgImgBtn;
    QPushButton *m_genBtn;
    QPushButton *m_saveBtn;
    QRDisplay *m_display;
    QColor m_fg = Qt::black;
    QColor m_bg = Qt::white;
    QColor m_textColor = Qt::black;
    QImage m_bgImg;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set dark palette for the entire application
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, Qt::black);
    darkPalette.setColor(QPalette::WindowText, QColor(0, 255, 255));
    darkPalette.setColor(QPalette::Base, Qt::black);
    darkPalette.setColor(QPalette::AlternateBase, Qt::black);
    darkPalette.setColor(QPalette::ToolTipBase, Qt::black);
    darkPalette.setColor(QPalette::ToolTipText, QColor(0, 255, 255));
    darkPalette.setColor(QPalette::Text, QColor(0, 255, 255));
    darkPalette.setColor(QPalette::Button, Qt::black);
    darkPalette.setColor(QPalette::ButtonText, QColor(0, 255, 255));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    app.setPalette(darkPalette);
    
    MainWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"
