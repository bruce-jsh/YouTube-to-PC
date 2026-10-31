<div align="center">

<img src="res/logo.png" width="96" alt="YouTube to PC">

# YouTube to PC

**The simplest way to save YouTube videos and music on your PC**

Copy a link, press one button. That is all.

<br>

### [⬇️ Download YouTubeToPC-Setup-1.3.1.exe](https://github.com/bruce-jsh/YouTube-to-PC/releases/download/v1.3.1/YouTubeToPC-Setup-1.3.1.exe)

1.3 MB · Windows 10/11 (64-bit) · click the button, the file downloads straight away

Blocked by Windows or your browser? [Here is how to get past it](docs/troubleshooting.md).

<br>

[한국어](docs/README.ko.md) ·
[Türkçe](docs/README.tr.md) ·
[Монгол](docs/README.mn.md) ·
**English** ·
[简体中文](docs/README.zh-Hans.md) ·
[繁體中文](docs/README.zh-Hant.md) ·
[Español](docs/README.es.md) ·
[Français](docs/README.fr.md) ·
[Deutsch](docs/README.de.md) ·
[Português](docs/README.pt.md) ·
[Русский](docs/README.ru.md) ·
[Tiếng Việt](docs/README.vi.md) ·
[ไทย](docs/README.th.md) ·
[Bahasa Indonesia](docs/README.id.md) ·
[日本語](docs/README.ja.md)


</div>

---

## Nothing to worry about

**No extra runtimes to install.**
You do not need .NET Framework, Visual C++ redistributables, Python or anything else.
It is a plain Win32 program that uses only what Windows already has. Download it and run it.

**No ads. None at all.**
No banners, no pop-ups, no "upgrade now" nags. There is no paid version either.

**No personal data is collected.**
No sign-up, no login. Nothing about your usage is sent anywhere.
Your settings and download list stay on your own PC.

**No bundled extras.**
Setup does not slip other software in, and it does not touch your browser settings.

---

## What it does

- Press **Paste Link** and the download starts (`Ctrl+V` works too)
- Choose **video** (MP4, MKV) or **music** (MP3, M4A)
- Quality from **best available** down to 144p; audio from best down to 128 kbps
- Grab a **whole playlist or channel**, or tick only the videos you want
- Pick **subtitles and audio tracks**: about 85 languages, each shown in its own script
- 1 to 8 downloads at the same time
- **Light and dark themes**, or follow your Windows setting
- Available in **15 languages**
- Thumbnails, progress and time left
- Play the file or open its folder
- Extract audio from a saved video
- Keeps a download history

---

## Installing

1. Click the download button above. The installer is saved right away.
2. Run it.
3. Choose your language and install folder, then press **Install**.

Setup needs an internet connection: it downloads the tools the app uses
(yt-dlp and FFmpeg) and **keeps its own copies inside the program folder**.
This usually takes 10 to 30 seconds.

> **If Windows shows "Windows protected your PC"**
> Click **More info → Run anyway**.
> That warning appears because this is made by one person and has no paid code
> signing certificate. It is not a virus.

To remove it, use **Settings → Apps**, or run `Uninstall.exe` in the install folder.
Your downloaded videos and music are never deleted.

---

## Using it

1. Copy a video address from YouTube.
2. Press **Paste Link** in the app.
3. Done. It appears in the list and starts downloading.

If you want to choose quality or subtitles for each video, turn **Instant** off.
A dialog will then open every time you add a link.

Paste a playlist or channel address and you get the full list of videos,
with checkboxes so you can pick only what you want.

---

## Questions

**Downloads suddenly stopped working.**
When YouTube changes, the download tool has to change with it.
The app checks for a newer version once a day, but you can run
**Tools → Update yt-dlp** yourself whenever you need to.

**It does not work on my company network.**
Try entering a proxy address under **Settings → Connection**.

**Age-restricted videos fail.**
Under **Settings → Sign-in**, choose a browser where you are signed in to YouTube.

---

## Building it yourself

```
build.bat
```

All you need on Windows is the UCRT64 g++ from [MSYS2](https://www.msys2.org).
No Visual Studio, no .NET SDK.

---

## Tools it uses

| Name | Purpose | Licence |
|---|---|---|
| [yt-dlp](https://github.com/yt-dlp/yt-dlp) | Downloading from YouTube | Unlicense |
| [FFmpeg](https://ffmpeg.org) | Merging video and audio, converting | GPLv3 |
| [QuickJS-ng](https://github.com/quickjs-ng/quickjs) | Running YouTube's JavaScript checks | MIT |

Setup downloads the official build of each project.
Full notices are in [THIRD-PARTY-NOTICES](licenses/THIRD-PARTY-NOTICES.txt).

---

## Please note

Only download content you have the right to save.
This tool is meant for content you own, have permission to use, or that is free of copyright.
How you use it is your responsibility.

---

## Licence

[MIT](LICENSE). The source code is yours to read, change and reuse.

---

## Code signing policy

Free code signing provided by [SignPath.io](https://about.signpath.io), certificate by [SignPath Foundation](https://signpath.org).

| Role | Who |
|---|---|
| Committers and reviewers | [Bruce Jung](https://github.com/bruce-jsh) |
| Approvers | [Bruce Jung](https://github.com/bruce-jsh) |

**Privacy policy.** This program will not transfer any information to other networked systems unless specifically requested by the user or the person installing or operating it.

It contacts YouTube for the videos you ask for, i.ytimg.com for the thumbnails in the list, and GitHub to look for newer versions of yt-dlp and of this program. Those checks can be switched off in Settings. There is no account, no sign-in and no usage tracking.

---

<div align="center">

I quit my job a few days ago. Bored at home, I built this instead.<br>
Dreaming of the freelance life, from one small room in Mukdong, Seoul in Korea.
31 October 2026.

**Bruce Jung** · [sunghunet@gmail.com](mailto:sunghunet@gmail.com)

</div>
