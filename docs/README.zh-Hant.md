<div align="center">

<img src="../res/app.ico" width="96" alt="YouTube to PC">

# YouTube to PC

**把 YouTube 影片和音樂儲存到電腦，最簡單的方式**

複製連結，按一下按鈕，就這麼簡單。

<br>

### [⬇️ 下載 YouTubeToPC-Setup-1.2.0.exe](https://github.com/bruce-jsh/YouTube-to-PC/releases/download/v1.2.0/YouTubeToPC-Setup-1.2.0.exe)

1.3 MB · Windows 10/11（64 位元）· 點選按鈕即可直接下載

<br>

[한국어](README.ko.md) ·
[Türkçe](README.tr.md) ·
[Монгол](README.mn.md) ·
[English](../README.md) ·
[简体中文](README.zh-Hans.md) ·
**繁體中文** ·
[Español](README.es.md) ·
[Français](README.fr.md) ·
[Deutsch](README.de.md) ·
[Português](README.pt.md) ·
[Русский](README.ru.md) ·
[Tiếng Việt](README.vi.md) ·
[ไทย](README.th.md) ·
[Bahasa Indonesia](README.id.md) ·
[日本語](README.ja.md)


</div>

---

## 可以安心使用

**不需要安裝任何額外元件。**
不必另外安裝 .NET Framework、Visual C++ 執行階段或 Python。
這是一個只使用 Windows 內建功能的 Win32 程式，下載後直接執行即可。

**完全沒有廣告。**
沒有橫幅、沒有彈出視窗，也沒有「立即升級」之類的提示。更沒有付費版本。

**不收集個人資訊。**
不需註冊，也不需登入。不會把你的使用紀錄傳送到任何地方。
設定與下載清單只保存在你自己的電腦上。

**不會綁其他軟體。**
安裝時不會夾帶別的程式，也不會更動瀏覽器設定。

---

## 功能

- 按一下 **貼上連結** 就開始下載（也可以用 `Ctrl+V`）
- 可選 **影片**（MP4、MKV）或 **音樂**（MP3、M4A、OPUS）
- 畫質從 **最佳畫質** 到 144p，音質從最佳到 128 kbps
- **整個播放清單或頻道** 一次下載，也可只勾選需要的影片
- 選擇 **字幕與音軌**: 約 85 種語言，皆以各自的文字顯示
- 同時下載 1 至 8 個
- **淺色與深色佈景主題**，也可跟隨系統設定
- 支援 **15 種語言**
- 顯示縮圖、進度與剩餘時間
- 完成後直接播放或開啟資料夾
- 從已儲存的影片中擷取音訊
- 保留下載紀錄

---

## 安裝方式

1. 點選上方的下載按鈕，安裝程式會立即儲存。
2. 執行該檔案。
3. 選擇語言與安裝位置，按下 **安裝**。

安裝時需要連線，因為程式會下載所使用的工具（yt-dlp、FFmpeg）並
**單獨保存在程式資料夾內**。通常 10 到 30 秒即可完成。

> **如果出現「Windows 已保護您的電腦」**
> 請點選 **其他資訊 → 仍要執行**。
> 這是因為本程式由個人開發，沒有付費的程式碼簽署憑證，並不是病毒。

解除安裝可透過 **設定 → 應用程式**，或執行安裝資料夾中的 `Uninstall.exe`。
已下載的影片和音樂不會被刪除。

---

## 使用方式

1. 在 YouTube 複製影片網址。
2. 按下程式中的 **貼上連結** 按鈕。
3. 完成。影片會出現在清單中並立即開始下載。

若想為每部影片個別選擇畫質或字幕，請關閉 **立即下載**。
之後每次加入連結都會跳出選項視窗。

輸入播放清單或頻道網址後，會列出全部影片，可只勾選需要的部分。

---

## 常見問題

**突然無法下載了。**
YouTube 一旦改版，下載工具也要隨之更新。
程式每天會自動檢查一次，著急時也可以手動執行
**工具 → 更新 yt-dlp**。

**在公司網路下無法使用。**
請在 **設定 → 連線** 中填入代理伺服器位址。

**年齡限制的影片下載失敗。**
請在 **設定 → 登入** 中選擇已登入 YouTube 的瀏覽器。

---

## 自行建置

```
build.bat
```

在 Windows 上只需要 [MSYS2](https://www.msys2.org) 的 UCRT64 g++。
不需要 Visual Studio，也不需要 .NET SDK。

---

## 使用的工具

| 名稱 | 用途 | 授權 |
|---|---|---|
| [yt-dlp](https://github.com/yt-dlp/yt-dlp) | 從 YouTube 下載 | Unlicense |
| [FFmpeg](https://ffmpeg.org) | 合併影片與音訊、格式轉換 | GPLv3 |
| [QuickJS-ng](https://github.com/quickjs-ng/quickjs) | 處理 YouTube 的 JavaScript 驗證 | MIT |

安裝時會下載各專案的官方建置版本。
詳細聲明見 [THIRD-PARTY-NOTICES](../licenses/THIRD-PARTY-NOTICES.txt)。

---

## 請注意

受著作權保護的內容，**僅在取得權利人許可的情況下** 才可下載。
本程式適用於你自己擁有、已獲授權或無著作權限制的內容。
使用所產生的責任由使用者承擔。

---

<div align="center">

幾天前我辭職了。在家閒得發慌，就寫了這個。<br>
夢想是當自由工作者，工作室是韓國首爾中浪區墨洞的一個小房間。
2026年10月31日。

**Bruce Jung** · [sunghunet@gmail.com](mailto:sunghunet@gmail.com)

</div>
