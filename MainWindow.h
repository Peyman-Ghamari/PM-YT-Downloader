#pragma once

#include <QMainWindow>
#include <QProcess>
#include <QVector>
#include <QTcpServer>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 🟢 Forward Declaration
struct PlaylistItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // Button Slots And User Events
    void onBrowseCookies();
    void onBrowseOutput();
    void onLogoClicked();
    void onStart();
    void onStop();
    void onTestProxy();
    void onUpdateEngine();

    // 🌐 Dedicated Slot For Managing Chrome Extension Requests
    void handleLocalConnection();

    // Slots Related To Managing Download And Preview Processes
    void onDownloadStdout();
    void onDownloadStderr();
    void onDownloadFinished(int exitCode, QProcess::ExitStatus status);
    void onPreviewFinished(int exitCode, QProcess::ExitStatus status);

private:
    enum class Mode { Single, Playlist };
    Mode currentMode() const;

    bool verifyTools(QString& errorText) const;
    QStringList buildPreviewArgs(const QString& url) const;

    bool parsePlaylistPreview(const QByteArray& jsonBytes,
                              QString& playlistTitle,
                              QVector<PlaylistItem>& items,
                              QString& errorText) const;

    QString buildPlaylistItemsCsv(const QVector<PlaylistItem>& selected) const;
    QStringList buildDownloadArgs(const QString& url, const QString& playlistItemsCsv = "") const;

    QString getProxyUrl() const;

    void logLine(const QString& s);
    void setUiRunning(bool running, const QString& statusText);

private:
    // Class Member Variables Definition Section
    Ui::MainWindow *ui;

    QProcess downloadProc;
    QProcess previewProc;

    // 🟢 Local Server Variable For Communication With Chrome Extension (In Its Correct Position)
    QTcpServer* localServer;
};