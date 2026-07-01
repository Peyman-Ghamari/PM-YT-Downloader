#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "AboutDialog.h"
#include "PlaylistPickerDialog.h"
#include "ToolPaths.h"
#include <QIcon>
#include <QSize>
#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <QTimer>
#include <QNetworkProxyFactory>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QUrl>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Icon For Window
    setWindowIcon(QIcon(":/assets/app.ico"));

    ui->btnLogoPM->setObjectName("LogoPM");

    // Testing Image Loading Process
    QPixmap px(":/assets/logo.png");
    qDebug() << "Logo loaded?" << !px.isNull();

    // Setting Logo Onto The Button
    ui->btnLogoPM->setText("");
    ui->btnLogoPM->setIcon(QIcon(px));
    ui->btnLogoPM->setIconSize(QSize(500, 200));   // برای دکمه 96x96 عالیه

    ui->qualityCombo->setCurrentText("720");
    ui->outputEdit->setText(QDir::homePath());
    ui->modeCombo->setCurrentIndex(0); // Single

    connect(ui->btnCookies, &QPushButton::clicked, this, &MainWindow::onBrowseCookies);
    connect(ui->btnOutput,  &QPushButton::clicked, this, &MainWindow::onBrowseOutput);
    connect(ui->btnStart,   &QPushButton::clicked, this, &MainWindow::onStart);
    connect(ui->btnStop,    &QPushButton::clicked, this, &MainWindow::onStop);
    connect(ui->btnLogoPM,  &QPushButton::clicked, this, &MainWindow::onLogoClicked);
    connect(ui->btnTestProxy, &QPushButton::clicked, this, &MainWindow::onTestProxy);
    connect(ui->btnUpdateEngine, &QPushButton::clicked, this, &MainWindow::onUpdateEngine);

    downloadProc.setProcessChannelMode(QProcess::SeparateChannels);
    connect(&downloadProc, &QProcess::readyReadStandardOutput, this, &MainWindow::onDownloadStdout);
    connect(&downloadProc, &QProcess::readyReadStandardError,  this, &MainWindow::onDownloadStderr);
    connect(&downloadProc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &MainWindow::onDownloadFinished);

    previewProc.setProcessChannelMode(QProcess::SeparateChannels);
    connect(&previewProc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &MainWindow::onPreviewFinished);

    ui->progressBar->setRange(0, 1);
    ui->progressBar->setValue(0);

    setUiRunning(false, "Ready.");
    // Automatically Loading Paths Or Constructing Professional Default Paths
    QSettings settings("PeymanProjects", "PM_YT_Downloader");

    QString savedCookies = settings.value("cookiesPath", "").toString();
    QString savedOutput = settings.value("outputPath", "").toString();

    if (!savedCookies.isEmpty()) {
        ui->cookiesEdit->setText(savedCookies);
    }

    // Automatically Detecting System Download Folder If No Prior Path Was Saved By User
    if (savedOutput.isEmpty()) {
        // پیدا کردن مسیر استاندارد C:\Users\Username\Downloads
        QString defaultDownloadsFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

        // Locating Standard Path C:\Users\Username\Downloads
        QDir dir(defaultDownloadsFolder);
        QString fullPath = dir.filePath("PM YT Downloader");


        if (!dir.exists("PM YT Downloader")) {
            dir.mkdir("PM YT Downloader");
        }

        ui->outputEdit->setText(fullPath);
        settings.setValue("outputPath", fullPath);
    } else {
        ui->outputEdit->setText(savedOutput);
    }


    localServer = new QTcpServer(this);


    if (!localServer->isListening()) {
        if (localServer->listen(QHostAddress::Any, 12345)) {
            logLine("System: Extension server is listening on port 12345.");
            connect(localServer, &QTcpServer::newConnection, this, &MainWindow::handleLocalConnection);
        } else {
            logLine("System Error: Port 12345 is blocked by another app!");
        }
    }


}

MainWindow::~MainWindow()
{
    if (previewProc.state() != QProcess::NotRunning) {
        previewProc.kill();
        previewProc.waitForFinished(800);
    }
    if (downloadProc.state() != QProcess::NotRunning) {
        downloadProc.kill();
        downloadProc.waitForFinished(800);
    }
    delete ui;
}

void MainWindow::onLogoClicked()
{
    AboutDialog dlg(this);
    dlg.exec();
}

MainWindow::Mode MainWindow::currentMode() const
{
    return (ui->modeCombo->currentText() == "Playlist") ? Mode::Playlist : Mode::Single;
}

void MainWindow::onBrowseCookies()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        "Select cookies.txt",
        QDir::homePath(),
        "Text files (*.txt);;All files (*.*)"
    );

    if (!path.isEmpty()) {
        ui->cookiesEdit->setText(path);

        // 🟢 Instantly And Automatically Saving Cookie File Path Into Windows Registry
        QSettings settings("PeymanProjects", "PM_YT_Downloader");
        settings.setValue("cookiesPath", path);
    }
}

void MainWindow::onBrowseOutput()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select output folder",
        QDir::homePath()
    );

    if (!dir.isEmpty()) {
        ui->outputEdit->setText(dir);

        // 🟢 Instantly And Automatically Saving Output Folder Path Into Windows Registry
        QSettings settings("PeymanProjects", "PM_YT_Downloader");
        settings.setValue("outputPath", dir);
    }
}

void MainWindow::setUiRunning(bool running, const QString& statusText)
{
    ui->statusLabel->setText(statusText);
    ui->btnStart->setEnabled(!running);
    ui->btnStop->setEnabled(running);
    ui->groupSettings->setEnabled(!running);

    if (running) ui->progressBar->setRange(0, 0);
    else { ui->progressBar->setRange(0, 1); ui->progressBar->setValue(0); }
}

void MainWindow::logLine(const QString& s)
{
    const QString t = s.trimmed();
    if (!t.isEmpty())
        ui->logBox->appendPlainText(t);
}

bool MainWindow::verifyTools(QString& errorText) const
{
    const QString yt = ytDlpPath();
    const QString dn = denoPath();
    const QString ff = ffmpegExe();

    if (!QFileInfo::exists(yt)) {
        errorText = "Missing tools/yt-dlp.exe";
        return false;
    }
    if (!QFileInfo::exists(dn)) {
        errorText = "Missing tools/deno.exe";
        return false;
    }
    if (!QFileInfo::exists(ff)) {
        errorText = "Missing tools/ffmpeg/bin/ffmpeg.exe";
        return false;
    }
    return true;
}

QStringList MainWindow::buildPreviewArgs(const QString& url) const
{
    const QString cookies = ui->cookiesEdit->text().trimmed();

    QStringList args;
    args << "--flat-playlist"
         << "--dump-single-json"
         << "--no-warnings";

    // 1. Validating Manual Cookie Setup
    if (!cookies.isEmpty()) {
        args << "--cookies" << cookies;
    }

    // 2. Injecting Configured Proxy From UI (If Present)
    QString proxyStr = getProxyUrl();
    if (!proxyStr.isEmpty()) {
        args << "--proxy" << proxyStr;
    }

    // 3. Injecting Shecan DNS To Concurrently Resolve Sanctions During Preview Loading
    args << "--downloader-args" << "aria2c:--dns-server=178.22.122.100,185.51.200.2";

    args << url;
    return args;
}

bool MainWindow::parsePlaylistPreview(const QByteArray& jsonBytes,
                                      QString& playlistTitle,
                                      QVector<PlaylistItem>& items,
                                      QString& errorText) const
{
    playlistTitle.clear();
    items.clear();
    errorText.clear();

    QJsonParseError err{};
    const auto doc = QJsonDocument::fromJson(jsonBytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        errorText = "Preview JSON parse failed (invalid JSON).";
        return false;
    }

    const QJsonObject root = doc.object();

    playlistTitle = root.value("title").toString();
    if (playlistTitle.isEmpty())
        playlistTitle = root.value("playlist_title").toString();
    if (playlistTitle.isEmpty())
        playlistTitle = "Playlist";

    const QJsonValue entriesV = root.value("entries");
    if (!entriesV.isArray()) {
        errorText = "No entries[] found in playlist preview.";
        return false;
    }

    const QJsonArray entries = entriesV.toArray();
    items.reserve(entries.size());

    int idx = 0;
    for (const auto& v : entries) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();

        PlaylistItem it;
        it.index = ++idx;
        it.id = o.value("id").toString();
        it.title = o.value("title").toString();
        it.uploader = o.value("uploader").toString();
        it.durationSec = o.value("duration").toInt(-1);

        QString url = o.value("webpage_url").toString();
        if (url.isEmpty())
            url = o.value("url").toString();
        if (url.isEmpty() && !it.id.isEmpty())
            url = "https://www.youtube.com/watch?v=" + it.id;

        it.url = url;
        it.checked = true;

        items.push_back(it);
    }

    if (items.isEmpty()) {
        errorText = "Playlist entries are empty.";
        return false;
    }

    return true;
}

QStringList MainWindow::buildDownloadArgs(const QString& url,
                                         const QString& playlistItemsCsv) const
{
    const QString cookies = ui->cookiesEdit->text().trimmed();
    const QString outDir  = ui->outputEdit->text().trimmed();
    const int height      = ui->qualityCombo->currentText().toInt();

    const QString format = QString(
        "bv*[height<=%1][ext=mp4]+ba[ext=m4a]/"
        "b[height<=%1][ext=mp4]/"
        "bv*[height<=%1]+ba/"
        "b[height<=%1]"
    ).arg(height);

    QStringList args;
    QString outTemplate;

    if (currentMode() == Mode::Single) {
        args << "--no-playlist";

        // 🟢 If Single Video: Directly Saved By Its Own Name Without Extra Folder
        outTemplate = QDir(outDir).filePath("%(title)s.%(ext)s");
    } else {
        args << "--yes-playlist";
        if (!playlistItemsCsv.isEmpty())
            args << "--playlist-items" << playlistItemsCsv;

        // 🟢 If Playlist: Constructs An Elegant Folder Named After The Playlist And Places Videos Inside Sequentially With Index Numbers
        outTemplate = QDir(outDir).filePath("%(playlist_title)s/%(playlist_index)02d - %(title)s.%(ext)s");
    }

    if (ui->ignoreErrorsCheck->isChecked())
        args << "--ignore-errors" << "--no-abort-on-error";

    if (ui->resumeCheck->isChecked())
        args << "--continue" << "--no-overwrites";

    if (!ui->keepTempCheck->isChecked())
        args << "--no-keep-video";

    // 1. Cheking Coockies
    if (!cookies.isEmpty()) {
        args << "--cookies" << cookies;
    }

    // 2. Injecting Configured Proxy From UI (SOCKS5 Or HTTP)
    QString proxyStr = getProxyUrl();
    if (!proxyStr.isEmpty()) {
        args << "--proxy" << proxyStr;
    }

    // 3. Injecting Shecan DNS Into Downloader Core For Maximum Connection Stability
    args << "--downloader-args" << "aria2c:--dns-server=178.22.122.100,185.51.200.2";

    // Internal Project Tools
    args << "--js-runtimes" << QString("deno:%1").arg(denoPath());
    args << "--remote-components" << "ejs:github";
    args << "--ffmpeg-location" << ffmpegBinDir();

    args << "-f" << format;
    args << "--merge-output-format" << "mp4";
    args << "-o" << outTemplate;

    args << url;
    return args;
}

QString MainWindow::buildPlaylistItemsCsv(const QVector<PlaylistItem>& selected) const
{
    QStringList idx;
    for (const auto& it : selected) {
        if (it.checked)
            idx << QString::number(it.index);
    }
    return idx.join(",");
}

void MainWindow::onStart()
{
    ui->logBox->clear();

    QString toolErr;
    if (!verifyTools(toolErr)) {
        setUiRunning(false, "Tools missing.");
        logLine("ERROR: " + toolErr);
        return;
    }

    const QString url = ui->urlEdit->text().trimmed();
    if (url.isEmpty()) {
        setUiRunning(false, "Please paste a URL first.");
        return;
    }

    if (downloadProc.state() != QProcess::NotRunning || previewProc.state() != QProcess::NotRunning) {
        logLine("A process is already running.");
        return;
    }

    logLine("yt-dlp: " + ytDlpPath());
    logLine("deno : " + denoPath());
    logLine("ffmpeg: " + ffmpegExe());
    logLine("-----");

    if (currentMode() == Mode::Playlist) {
        setUiRunning(true, "Fetching playlist preview…");
        const auto args = buildPreviewArgs(url);
        logLine("Preview:");
        logLine(ytDlpPath() + " " + args.join(' '));
        logLine("-----");
        previewProc.start(ytDlpPath(), args);
        return;
    }

    const auto args = buildDownloadArgs(url);
    logLine("Running:");
    logLine(ytDlpPath() + " " + args.join(' '));
    logLine("-----");

    setUiRunning(true, "Downloading…");
    downloadProc.start(ytDlpPath(), args);
}

void MainWindow::onStop()
{
    if (previewProc.state() != QProcess::NotRunning) {
        logLine("Stopping preview…");
        previewProc.kill();
        previewProc.waitForFinished(800);
        setUiRunning(false, "Preview stopped.");
        return;
    }

    if (downloadProc.state() != QProcess::NotRunning) {
        logLine("Stopping download…");
        downloadProc.kill();
        downloadProc.waitForFinished(1500);
        setUiRunning(false, "Download stopped.");
        return;
    }

    setUiRunning(false, "Ready.");
}

void MainWindow::onPreviewFinished(int exitCode, QProcess::ExitStatus)
{
    const QString url = ui->urlEdit->text().trimmed();
    const QByteArray out = previewProc.readAllStandardOutput();
    const QByteArray err = previewProc.readAllStandardError();

    if (!err.isEmpty())
        logLine(QString::fromUtf8(err));

    if (exitCode != 0) {
        setUiRunning(false, QString("Preview failed (exit %1)").arg(exitCode));
        return;
    }

    QString playlistTitle, parseErr;
    QVector<PlaylistItem> items;

    if (!parsePlaylistPreview(out, playlistTitle, items, parseErr)) {
        setUiRunning(false, "Preview parse failed.");
        logLine("ERROR: " + parseErr);
        return;
    }

    setUiRunning(false, "Select playlist items…");

    PlaylistPickerDialog dlg(playlistTitle, items, this);
    if (dlg.exec() != QDialog::Accepted) {
        setUiRunning(false, "Cancelled.");
        return;
    }

    const auto selected = dlg.selectedItems();
    const QString csv = buildPlaylistItemsCsv(selected);
    if (csv.isEmpty()) {
        setUiRunning(false, "No items selected.");
        return;
    }

    // Correcting Error Line: Returning To Standard 2-Argument Mode For Playlist Download With Manual Cookie File
    const auto args = buildDownloadArgs(url, csv);

    logLine("Selected playlist items: " + csv);
    logLine("Running:");
    logLine(ytDlpPath() + " " + args.join(' '));
    logLine("-----");

    setUiRunning(true, "Downloading playlist…");
    downloadProc.start(ytDlpPath(), args);
}



void MainWindow::onDownloadStdout()
{
    // 1. Reading Raw Data From Downloader Process
    QByteArray data = downloadProc.readAllStandardOutput();
    QString text = QString::fromLocal8Bit(data);

    // Printing Raw Log Inside Main Box For Archiving
    logLine(text.trimmed());

    // 2. Parsing Lines Regarding Playlist Items
    // Log Example: [download] Downloading item 5 of 26
    if (text.contains("[download]") && text.contains("item")) {
        static QRegularExpression playlistRegex(R"(item\s+(\d+)\s+of\s+(\d+))");
        auto match = playlistRegex.match(text);
        if (match.hasMatch()) {
            QString currentItem = match.captured(1);
            QString totalItems = match.captured(2);


            ui->statusLabel->setText(QString("Downloading: Video %1 of %2 🎬").arg(currentItem, totalItems));
        }
    }

    // 3. Intelligently Processing Percentage, Speed, Size, And ETA (Previous Code)
    if (text.contains("[download]") && text.contains("%")) {

        // Extracting Percentage And Updating ProgressBar
        static QRegularExpression percentRegex(R"(([0-9.]+)%)");
        auto percentMatch = percentRegex.match(text);
        if (percentMatch.hasMatch()) {
            double percent = percentMatch.captured(1).toDouble();
            ui->progressBar->setValue(static_cast<int>(percent));
        }

        // Extracting Download Speed
        static QRegularExpression speedRegex(R"(at\s+([0-9.]+[KM]iB/s))");
        auto speedMatch = speedRegex.match(text);
        if (speedMatch.hasMatch()) {
            ui->lblSpeed->setText("Speed: " + speedMatch.captured(1).replace("iB", "B"));
        }

        // Extracting Total File Size
        static QRegularExpression sizeRegex(R"(of\s+([0-9.]+[KM]iB))");
        auto sizeMatch = sizeRegex.match(text);
        if (sizeMatch.hasMatch()) {
            ui->lblSize->setText("Size: " + sizeMatch.captured(1).replace("iB", "B"));
        }

        // Extracting ETA (Estimated Time of Arrival)
        static QRegularExpression etaRegex(R"(ETA\s+([0-9:]+))");
        auto etaMatch = etaRegex.match(text);
        if (etaMatch.hasMatch()) {
            ui->lblEta->setText("ETA: " + etaMatch.captured(1));
        }
    }
}
void MainWindow::onDownloadStderr()
{
    const QString text = QString::fromUtf8(downloadProc.readAllStandardError());
    if (!text.isEmpty())
        logLine(text);
}

void MainWindow::onDownloadFinished(int exitCode, QProcess::ExitStatus)
{
    // 1. Changing UI State Based On Success Or Failure
    if (exitCode == 0) {
        setUiRunning(false, "Done ✅");
    } else {
        setUiRunning(false, QString("Finished with errors (exit %1)").arg(exitCode));
    }

    // 2. 🟢 Resetting Download Status Elements To Prepare For Next Download
    ui->progressBar->setValue(0);
    ui->lblSpeed->setText("Speed: 0 KB/s");
    ui->lblSize->setText("Size: 0 MB");
    ui->lblEta->setText("ETA: --:--");
}

void MainWindow::onTestProxy()
{
    ui->statusLabel->setText("Checking connection to YouTube...");
    ui->btnTestProxy->setEnabled(false);

    auto* manager = new QNetworkAccessManager(this);

    // Setting Temporary Proxy For Network Testing
    if (ui->proxyTypeCombo->currentText() != "No Proxy") {
        QNetworkProxy proxy;
        if (ui->proxyTypeCombo->currentText() == "SOCKS5") {
            proxy.setType(QNetworkProxy::Socks5Proxy);
        } else {
            proxy.setType(QNetworkProxy::HttpProxy);
        }

        QString host = ui->proxyAddressEdit->text().trimmed();
        if (host.isEmpty()) host = "127.0.0.1";
        int port = ui->proxyPortEdit->text().trimmed().toInt();

        proxy.setHostName(host);
        proxy.setPort(port);
        manager->setProxy(proxy);
    } else {
        // Enabling Direct Reading of Proxy/VPN Settings Directly From Windows
        QNetworkProxyFactory::setUseSystemConfiguration(true);

        // Locating Active System Proxies For YouTube Address
        QList<QNetworkProxy> proxies = QNetworkProxyFactory::proxyForQuery(QNetworkProxyQuery(QUrl("https://www.youtube.com")));
        if (!proxies.isEmpty()) {
            manager->setProxy(proxies.first()); // Using System Proxy (Like A VPN Tunnel)
        } else {
            manager->setProxy(QNetworkProxy::NoProxy); // If VPN Is Completely Turned Off
        }
    }

    // Sending A Lightweight Request To YouTube
    QNetworkRequest request(QUrl("https://www.youtube.com"));
    QNetworkReply* reply = manager->get(request);

    // 🟢 Manually Managing 5-Second Timeout Using QTimer (Bulletproof & Fully Compatible With All Versions)
    // Aborts The Request If The Process Has Not Completed After 5000 Milliseconds
    QTimer::singleShot(5000, reply, [reply]() {
        if (reply && reply->isRunning()) {
            reply->abort(); // سقط کردن درخواست در صورت طولانی شدن
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
            if (reply->error() == QNetworkReply::NoError) {
                ui->statusLabel->setText("Connected to YouTube successfully! 🟢");

                if (ui->logBox->toPlainText().isEmpty() || !ui->logBox->toPlainText().endsWith("Network: Connection to YouTube is active.\n")) {
                    logLine("Network: Connection to YouTube is active.");
                }
            } else {
                ui->statusLabel->setText("Connection failed! 🔴");
                logLine("Network Error: " + reply->errorString());
            }
            ui->btnTestProxy->setEnabled(true);
            reply->deleteLater();
            manager->deleteLater();
        });
}


QString MainWindow::getProxyUrl() const
{
    if (ui->proxyTypeCombo->currentText() == "No Proxy") return "";

    QString type = ui->proxyTypeCombo->currentText().toLower();
    QString host = ui->proxyAddressEdit->text().trimmed().isEmpty() ? "127.0.0.1" : ui->proxyAddressEdit->text().trimmed();
    QString port = ui->proxyPortEdit->text().trimmed().isEmpty() ? "1080" : ui->proxyPortEdit->text().trimmed();

    return QString("%1://%2:%3").arg(type, host, port);
}
void MainWindow::onUpdateEngine()
{
    ui->logBox->clear();
    logLine("Checking for yt-dlp updates...");
    ui->statusLabel->setText("Updating engine...");
    ui->btnUpdateEngine->setEnabled(false);

    auto* updateProc = new QProcess(this);

    // Locating Exact Executable Path From Tools Class
    // (Assuming ytDlpPath() Or Similar Exists; Otherwise, The Direct Path Below Is Perfectly Correct)
    QString ytDlpPath = QDir(QCoreApplication::applicationDirPath()).filePath("tools/yt-dlp.exe");

    updateProc->setProgram(ytDlpPath);
    updateProc->setArguments(QStringList() << "--update");

    // Automatically Redirecting Update Output To Program Log Box
    connect(updateProc, &QProcess::readyReadStandardOutput, this, [this, updateProc]() {
        logLine(QString::fromLocal8Bit(updateProc->readAllStandardOutput()).trimmed());
    });
    connect(updateProc, &QProcess::readyReadStandardError, this, [this, updateProc]() {
        logLine("Error: " + QString::fromLocal8Bit(updateProc->readAllStandardError()).trimmed());
    });

    // Managing Update Process Termination
    connect(updateProc, &QProcess::finished, this, [this, updateProc](int exitCode, QProcess::ExitStatus status) {
        if (exitCode == 0) {
            ui->statusLabel->setText("Engine update process finished! 🟢");
            logLine("System: yt-dlp core check completed successfully.");
        } else {
            ui->statusLabel->setText("Update failed or canceled! 🔴");
            logLine("System: Update process returned exit code " + QString::number(exitCode));
        }
        ui->btnUpdateEngine->setEnabled(true);
        updateProc->deleteLater();
    });

    updateProc->start();
}


void MainWindow::handleLocalConnection()
{
    QTcpSocket* socket = localServer->nextPendingConnection();

    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        QByteArray requestData = socket->readAll();
        QString requestStr = QString::fromUtf8(requestData);

        if (requestStr.startsWith("POST")) {
            int bodyStartIndex = requestStr.indexOf("\r\n\r\n");
            if (bodyStartIndex != -1) {
                QString body = requestStr.mid(bodyStartIndex + 4).trimmed();

                QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8());
                if (!doc.isNull() && doc.isObject()) {
                    QJsonObject obj = doc.object();
                    QString url = obj["url"].toString();
                    QString cookiesContent = obj["cookies"].toString();

                    if (!url.isEmpty()) {
                        ui->urlEdit->setText(url);

                        // 🟢 1. Managing And Creating Dedicated cookies Folder Next To Program Executable
                        QString appDir = QCoreApplication::applicationDirPath();
                        QDir cookiesDir(QDir(appDir).filePath("cookies"));

                        if (!cookiesDir.exists()) {
                            cookiesDir.mkpath(".");
                        }

                        // 🟢 2. Specifying New File Path Inside cookies Folder
                        QString cookiesFilePath = cookiesDir.filePath("ext_cookies.txt");

                        QFile file(cookiesFilePath);
                        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            QTextStream out(&file);
                            out << cookiesContent;
                            file.close();

                            ui->cookiesEdit->setText(cookiesFilePath);

                            QSettings settings("PeymanProjects", "PM_YT_Downloader");
                            settings.setValue("cookiesPath", cookiesFilePath);
                        }

                        ui->statusLabel->setText("Link & Cookies Injected! 🌐");
                        logLine("Extension: URL captured. Cookies saved cleanly in /cookies directory.");

                        this->raise();
                        this->activateWindow();
                    }
                }
            }

            // Sending Response To Chrome
            QString response = "HTTP/1.1 200 OK\r\n"
                               "Access-Control-Allow-Origin: *\r\n"
                               "Content-Type: text/plain\r\n"
                               "Content-Length: 2\r\n"
                               "Connection: close\r\n\r\n"
                               "OK";
            socket->write(response.toUtf8());
            socket->flush();
            socket->disconnectFromHost();
        }
    });
}