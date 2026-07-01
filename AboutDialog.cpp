#include "AboutDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>

static void openUrl(const QString& url) {
    QDesktopServices::openUrl(QUrl(url));
}

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("About Developer");
    setModal(true);
    setMinimumWidth(440);

    // Purple/Black theme (matches your main UI)
    setStyleSheet(R"QSS(
      QDialog { background: #0b0b12; color: #EAEAF2; }

      QLabel#Title { font-size: 16pt; font-weight: 800; color: #F3F1FF; }
      QLabel#Sub { color: #B8B8D6; }

      QLabel {
        color: #DADAF0;
      }

      QPushButton {
        border-radius: 12px;
        padding: 10px 14px;
        font-weight: 650;
        border: 1px solid rgba(170, 120, 255, 0.22);
        background: rgba(20, 20, 34, 0.9);
        color: #F2F1FF;
      }
      QPushButton:hover { border: 1px solid rgba(170, 120, 255, 0.55); }
      QPushButton:pressed { background: rgba(26, 26, 44, 0.95); }

      QPushButton#Primary {
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
          stop:0 rgba(124, 58, 237, 0.95),
          stop:1 rgba(168, 85, 247, 0.95));
        border: 1px solid rgba(200, 160, 255, 0.35);
      }
      QPushButton#Primary:hover {
        border: 1px solid rgba(220, 190, 255, 0.55);
      }

      QPushButton#CloseBtn {
        background: rgba(18, 18, 30, 0.95);
      }
    )QSS");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    auto *title = new QLabel("PM YT.Downloader");
    title->setObjectName("Title");

    auto *sub = new QLabel("Developer info");
    sub->setObjectName("Sub");

    auto *info = new QLabel(
        "Developer: <b>Peyman Ghamari</b><br>"
        "GitHub: <b>Peyman-Ghamari</b><br>"
        "Telegram: <b>P_Ghamari</b>"
    );
    info->setTextFormat(Qt::RichText);

    auto *btnGitHub = new QPushButton("Open GitHub");
    btnGitHub->setObjectName("Primary");
    QObject::connect(btnGitHub, &QPushButton::clicked, this, []{
        openUrl("https://github.com/Peyman-Ghamari");
    });

    auto *btnTelegram = new QPushButton("Open Telegram");
    btnTelegram->setObjectName("Primary");
    QObject::connect(btnTelegram, &QPushButton::clicked, this, []{
        openUrl("https://t.me/P_Ghamari");
    });

    auto *row = new QHBoxLayout();
    row->addWidget(btnGitHub);
    row->addWidget(btnTelegram);

    auto *closeBtn = new QPushButton("Close");
    closeBtn->setObjectName("CloseBtn");
    QObject::connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    root->addWidget(title);
    root->addWidget(sub);
    root->addWidget(info);
    root->addLayout(row);
    root->addStretch(1);
    root->addWidget(closeBtn);
}
