#pragma once
#include <QDialog>
#include <QVector>

QT_BEGIN_NAMESPACE
class QTableWidget;
class QLineEdit;
class QLabel;
QT_END_NAMESPACE

struct PlaylistItem {
    int index = 0;          // 1-based
    QString title;
    QString id;
    QString url;
    int durationSec = -1;
    QString uploader;
    bool checked = true;
};

class PlaylistPickerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PlaylistPickerDialog(const QString& playlistTitle,
                                  const QVector<PlaylistItem>& items,
                                  QWidget *parent = nullptr);

    QVector<PlaylistItem> selectedItems() const;

private slots:
    void onCheckAll();
    void onUncheckAll();
    void onInvert();
    void onSearchChanged(const QString& text);
    void onSelectionChanged();

private:
    void buildUi(const QString& playlistTitle);
    void populateTable();
    void applyTheme();
    static QString formatDuration(int sec);

private:
    QString m_playlistTitle;
    QVector<PlaylistItem> m_items;

    QLineEdit* m_search = nullptr;
    QTableWidget* m_table = nullptr;
    QLabel* m_details = nullptr;
};
