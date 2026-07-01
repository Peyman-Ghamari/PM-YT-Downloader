# 🌐 PM YT.Downloader

A high-performance, asynchronous YouTube Video and Playlist Downloader built with **C++ (Qt)**, **Deno**, **yt-dlp**, and **FFmpeg**. It features a stunning **dark glassmorphic UI** and seamless **Chrome Extension integration** for an optimized desktop workflow.

---
## 🚀 Quick Download

📥 **[Download PM YT.Downloader Installer v1.0.0 (Windows)](https://github.com/Peyman-Ghamari/PM-YT-Downloader/releases/download/v1.0.0/PM_YT_Downloader_v1.0.0_Setup.exe)**

> *Note: For automatic browser session syncing and bypassing age restrictions, make sure to install the bundled Chrome extension following the guide below.*

---
## ✨ Features

* **Glassmorphic UI:** A modern, beautiful dark-mode interface utilizing glassmorphism aesthetics with glowing neon focus states and floating layout elements.
* **Asynchronous Engine:** Fully non-blocking UI using decoupled `QProcess` tasks to handle active downloading operations flawlessly.
* **Chrome Extension Sync:** Built-in lightweight TCP local server socket to immediately capture and process incoming links shared from Google Chrome.
* **Smart Network Watchdog:** Automatic detection of active Windows system proxies/VPNs with a dedicated 5-second connection timeout fallback.
* **Granular Progress Monitoring:** Real-time extraction of downloading speed, remaining ETA, and file size routed directly into a custom-styled logging terminal.
* **Sandboxed Cookies Manager:** Automatically initializes and structures safe local browser cookie folders next to the executable to securely bypass age-restricted links.

---

## 🛠️ Project Structure

Based on the clean CMake tree architecture of the project:

* `MainWindow.ui / .cpp / .h` - Core GUI controller embedded with highly optimized QSS layers.
* `PlaylistPickerDialog.cpp / .h` - Interactive element allowing customized individual or batch track selection.
* `AboutDialog.cpp / .h` - Information module housing developer and core component licenses.
* `ToolPaths.h` - Central utility manager responsible for dynamic binary mapping (`yt-dlp`, `FFmpeg`, `Deno`).
* `assets/` & `resources/` - Dedicated nodes preserving core icons, media branding, and visual assets.
* `tools/` - Sandbox context environment where the download core components live.

---

## 🚀 Getting Started

### Prerequisites
* **Qt 5.15+** or **Qt 6.x** Framework
* **CMake** 3.16+
* Modern C++ Compiler (**MinGW** or **MSVC**)

### Compiling from Source
1. Clone the repository:
   ```bash
   git clone [https://github.com/YOUR_USERNAME/PM-YT-Downloader.git](https://github.com/YOUR_USERNAME/PM-YT-Downloader.git)
   cd PM-YT-Downloader

---

---

## 💻 Workflow & Advanced Feature Guide

### 1. Desktop Client Operations & Network Verification
* **Network & Proxy Validation:** Before initiating downloads, users can click the **Test Connection** button. The application's built-in network watchdog will automatically verify your connection stability, inspect active Windows system proxies/VPNs, and enforce a strict 5-second timeout fallback to ensure smooth communication with YouTube's servers.
* **Core Engine Maintenance:** Keep your environment up to date using the **Update Engine** feature, which dynamically checks, fetches, and overwrites the underlying `yt-dlp` core binary directly through the UI.

### 2. Multi-Mode Downloading Capabilities
* **Single Video Mode:** Standard YouTube URLs are parsed instantly. This mode natively supports extracting and downloading high-fidelity **YouTube Shorts** alongside regular videos.
* **Playlist Mode:** Activating this option passes the playlist structure to the `PlaylistPickerDialog`, giving you granular control to inspect, select, or batch-download specific tracks or full albums seamlessly.

### 3. Bypassing Restrictions (Cookie Management)
To safely bypass YouTube's age restrictions, bot-detection walls, or private video limitations, the application requires valid session cookies. You can implement this via two professional methods:
1. **The Extension Method (Recommended):** Install the companion Chrome extension (setup below). It automatically captures your browser's session state and securely pipes the updated cookie files directly into the program’s sandboxed `cookies/` directory.
2. **The Manual Method:** Use external tools like `Get cookies.txt` (Netscape format), export your browser cookies, and link the file path directly within the UI's **Cookies** field.

---

## 🔌 Chrome Extension Setup

To stream video links instantly from Google Chrome into your desktop application's asynchronous downloading queue:

1. Open Google Chrome and navigate to `chrome://extensions/`.
2. Toggle on **Developer mode** (top-right corner).
3. Click **Load unpacked** (top-left corner).
4. Select the `chrome-extension` folder bundled within this project's repository.
5. Open any YouTube video or Short, click the extension icon, and watch the local TCP server instantly route the payload into the active desktop log box without interrupting your browser workflow.
