#pragma once
#include <QDir>
#include <QCoreApplication>

inline QString appDir()
{
    return QCoreApplication::applicationDirPath();
}

inline QString toolPath(const QString& rel)
{
    return QDir(appDir()).filePath(rel);
}

inline QString ytDlpPath()
{
    return toolPath("tools/yt-dlp.exe");
}

inline QString denoPath()
{
    return toolPath("tools/deno.exe");
}

inline QString ffmpegBinDir()
{
    return toolPath("tools/ffmpeg/bin");
}

inline QString ffmpegExe()
{
    return QDir(ffmpegBinDir()).filePath("ffmpeg.exe");
}
