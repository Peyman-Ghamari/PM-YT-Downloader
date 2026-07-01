#include "PlaylistPickerDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QItemSelectionModel>

QString PlaylistPickerDialog::formatDuration(int sec)
{
    if (sec < 0) return "-";
    int h = sec / 3600; sec %= 3600;
    int m = sec / 60;   sec %= 60;
    if (h > 0) return QString("%1:%2:%3").arg(h).arg(m,2,10,QChar('0')).arg(sec,2,10,QChar('0'));
    return QString("%1:%2").arg(m).arg(sec,2,10,QChar('0'));
}

PlaylistPickerDialog::PlaylistPickerDialog(const QString& playlistTitle,
                                           const QVector<PlaylistItem>& items,
                                           QWidget *parent)
    : QDialog(parent),
      m_playlistTitle(playlistTitle),
      m_items(items)
{
    setModal(true);
    setWindowTitle("Select playlist items");
    setMinimumSize(920, 560);

    buildUi(playlistTitle);
    populateTable();
    applyTheme();

    if (m_table->rowCount() > 0) {
        m_table->selectRow(0);
        onSelectionChanged();
    }
}

void PlaylistPickerDialog::buildUi(const QString& playlistTitle)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16);
    root->setSpacing(10);

    auto* title = new QLabel(QString("Playlist: <b>%1</b> — Select items to download").arg(playlistTitle));
    title->setTextFormat(Qt::RichText);

    m_search = new QLineEdit();
    m_search->setPlaceholderText("Search title…");
    connect(m_search, &QLineEdit::textChanged, this, &PlaylistPickerDialog::onSearchChanged);

    auto* topRow = new QHBoxLayout();
    auto* btnAll = new QPushButton("Check All");
    auto* btnNone = new QPushButton("Uncheck All");
    auto* btnInvert = new QPushButton("Invert");

    connect(btnAll, &QPushButton::clicked, this, &PlaylistPickerDialog::onCheckAll);
    connect(btnNone, &QPushButton::clicked, this, &PlaylistPickerDialog::onUncheckAll);
    connect(btnInvert, &QPushButton::clicked, this, &PlaylistPickerDialog::onInvert);

    topRow->addWidget(btnAll);
    topRow->addWidget(btnNone);
    topRow->addWidget(btnInvert);
    topRow->addStretch(1);
    topRow->addWidget(m_search);

    m_table = new QTableWidget();
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"#", "Title", "Duration", "Uploader", "ID"});
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);

    connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PlaylistPickerDialog::onSelectionChanged);

    m_details = new QLabel("Select an item to see details…");
    m_details->setWordWrap(true);

    auto* bottomRow = new QHBoxLayout();
    auto* btnOk = new QPushButton("Download Selected");
    auto* btnCancel = new QPushButton("Cancel");
    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    bottomRow->addWidget(m_details, 1);
    bottomRow->addStretch(1);
    bottomRow->addWidget(btnOk);
    bottomRow->addWidget(btnCancel);

    root->addWidget(title);
    root->addLayout(topRow);
    root->addWidget(m_table);
    root->addLayout(bottomRow);
}

void PlaylistPickerDialog::populateTable()
{
    m_table->setRowCount(m_items.size());

    for (int r = 0; r < m_items.size(); ++r) {
        const auto& it = m_items[r];

        auto* idx = new QTableWidgetItem(QString::number(it.index));
        idx->setCheckState(it.checked ? Qt::Checked : Qt::Unchecked);

        auto* title = new QTableWidgetItem(it.title);
        auto* dur = new QTableWidgetItem(formatDuration(it.durationSec));
        auto* up = new QTableWidgetItem(it.uploader.isEmpty() ? "-" : it.uploader);
        auto* id = new QTableWidgetItem(it.id);

        m_table->setItem(r, 0, idx);
        m_table->setItem(r, 1, title);
        m_table->setItem(r, 2, dur);
        m_table->setItem(r, 3, up);
        m_table->setItem(r, 4, id);
    }
}

void PlaylistPickerDialog::applyTheme()
{
    setStyleSheet(R"QSS(
      QDialog { background: #0b0b12; color: #EAEAF2; }
      QLabel { color: #DADAF0; }

      QLineEdit {
        background: rgba(10, 10, 18, 0.9);
        border: 1px solid rgba(170, 120, 255, 0.22);
        border-radius: 12px;
        padding: 8px 10px;
        color: #F2F1FF;
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

      QTableWidget {
        background: rgba(10,10,18,0.95);
        border: 1px solid rgba(170, 120, 255, 0.22);
        border-radius: 12px;
        gridline-color: rgba(170, 120, 255, 0.10);
        color: #F2F1FF;
        alternate-background-color: rgba(18, 18, 30, 0.55);
      }
      QHeaderView::section {
        background: rgba(18, 18, 30, 0.9);
        color: #CBB6FF;
        border: none;
        padding: 8px;
        font-weight: 700;
      }
      QTableWidget::item:selected { background: rgba(170, 120, 255, 0.18); }
    )QSS");
}

void PlaylistPickerDialog::onCheckAll()
{
    for (int r = 0; r < m_table->rowCount(); ++r)
        m_table->item(r, 0)->setCheckState(Qt::Checked);
}

void PlaylistPickerDialog::onUncheckAll()
{
    for (int r = 0; r < m_table->rowCount(); ++r)
        m_table->item(r, 0)->setCheckState(Qt::Unchecked);
}

void PlaylistPickerDialog::onInvert()
{
    for (int r = 0; r < m_table->rowCount(); ++r) {
        auto* it = m_table->item(r, 0);
        it->setCheckState(it->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
    }
}

void PlaylistPickerDialog::onSearchChanged(const QString& text)
{
    const QString q = text.trimmed();
    for (int r = 0; r < m_table->rowCount(); ++r) {
        const QString t = m_table->item(r, 1)->text();
        const bool match = q.isEmpty() || t.contains(q, Qt::CaseInsensitive);
        m_table->setRowHidden(r, !match);
    }
}

void PlaylistPickerDialog::onSelectionChanged()
{
    const auto ranges = m_table->selectedRanges();
    if (ranges.isEmpty()) return;

    const int r = ranges.first().topRow();
    if (r < 0 || r >= m_items.size()) return;

    const auto& it = m_items[r];
    const QString detail = QString(
        "<b>#%1</b> — %2<br>"
        "Uploader: %3 — Duration: %4<br>"
        "URL: %5"
    ).arg(it.index)
     .arg(it.title.toHtmlEscaped())
     .arg(it.uploader.isEmpty() ? "-" : it.uploader.toHtmlEscaped())
     .arg(formatDuration(it.durationSec))
     .arg(it.url.toHtmlEscaped());

    m_details->setText(detail);
    m_details->setTextFormat(Qt::RichText);
}

QVector<PlaylistItem> PlaylistPickerDialog::selectedItems() const
{
    QVector<PlaylistItem> out = m_items;
    for (int r = 0; r < out.size(); ++r) {
        out[r].checked = (m_table->item(r, 0)->checkState() == Qt::Checked);
    }
    return out;
}
