<div align="center">

<img src="../res/logo.png" width="96" alt="YouTube to PC">

# YouTube to PC

**把 YouTube 视频和音乐保存到电脑，最简单的方式**

复制链接，按一下按钮，就这么简单。

<br>

### [⬇️ 下载 YouTubeToPC-Setup-1.3.1.exe](https://github.com/bruce-jsh/YouTube-to-PC/releases/download/v1.3.1/YouTubeToPC-Setup-1.3.1.exe)

1.3 MB · Windows 10/11（64 位）· 点击按钮即可直接下载

被 Windows 或浏览器拦住了？[这里有解决办法](troubleshooting.zh-Hans.md)。


<img src="images/app-main.png" width="620" alt="YouTube to PC">

*粘贴链接即可开始下载。*

<img src="images/app-menu.png" width="620" alt="">

*从列表移除和连同文件删除是两个独立选项。*

<img src="images/app-languages.png" width="620" alt="">

*15 种语言，可在设置中选择。*

<br>

[한국어](README.ko.md) ·
[Türkçe](README.tr.md) ·
[Монгол](README.mn.md) ·
[English](../README.md) ·
**简体中文** ·
[繁體中文](README.zh-Hant.md) ·
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

## 可以放心使用

**无需安装任何额外组件。**
不用另外安装 .NET Framework、Visual C++ 运行库或 Python。
这是一个只使用 Windows 自带功能的 Win32 程序，下载后直接运行即可。

**完全没有广告。**
没有横幅、没有弹窗，也没有"立即升级"之类的提示。更没有付费版本。

**不收集个人信息。**
无需注册，无需登录。不会把你的使用记录发送到任何地方。
设置和下载列表只保存在你自己的电脑上。

**不捆绑其他软件。**
安装时不会夹带别的程序，也不会修改浏览器设置。

---

## 功能

- 按一下 **粘贴链接** 就开始下载（也可以用 `Ctrl+V`）
- 可选 **视频**（MP4、MKV）或 **音乐**（MP3、M4A）
- 画质从 **最佳画质** 到 144p，音质从最佳到 128 kbps
- **整个播放列表或频道** 一次下载，也可只勾选需要的视频
- 选择 **字幕和音轨**: 约 85 种语言，均以各自的文字显示
- 同时下载 1 至 8 个
- **浅色与深色主题**，也可跟随系统设置
- 支持 **15 种语言**
- 显示缩略图、进度和剩余时间
- 完成后直接播放或打开文件夹
- 从已保存的视频中提取音频
- 保留下载记录

---

## 安装方法

1. 点击上方的下载按钮，安装程序会立即保存。
2. 运行该文件。
3. 选择语言和安装位置，点击 **安装**。

安装时需要联网，因为程序会下载所用的工具（yt-dlp、FFmpeg）并
**单独保存在程序文件夹内**。通常 10 到 30 秒即可完成。

> **如果出现"Windows 已保护你的电脑"**
> 请点击 **更多信息 → 仍要运行**。
> 这是因为本程序由个人开发，没有付费的代码签名证书，并不是病毒。

卸载可通过 **设置 → 应用**，或运行安装文件夹中的 `Uninstall.exe`。
已下载的视频和音乐不会被删除。

---

## 使用方法

1. 在 YouTube 复制视频地址。
2. 点击程序中的 **粘贴链接** 按钮。
3. 完成。视频会出现在列表中并立即开始下载。

如果想为每个视频单独选择画质或字幕，请关闭 **立即下载**。
之后每次添加链接都会弹出选项窗口。

输入播放列表或频道地址后，会列出全部视频，可只勾选需要的部分。

---

## 常见问题

**突然下载不了了。**
YouTube 一旦改版，下载工具也要随之更新。
程序每天会自动检查一次，着急时也可以手动执行
**工具 → 更新 yt-dlp**。

**在公司网络下用不了。**
请在 **设置 → 连接** 中填写代理地址。

**年龄限制的视频下载失败。**
请在 **设置 → 登录** 中选择已登录 YouTube 的浏览器。

---

## 自行构建

```
build.bat
```

在 Windows 上只需要 [MSYS2](https://www.msys2.org) 的 UCRT64 g++。
不需要 Visual Studio，也不需要 .NET SDK。

---

## 使用的工具

| 名称 | 用途 | 许可证 |
|---|---|---|
| [yt-dlp](https://github.com/yt-dlp/yt-dlp) | 从 YouTube 下载 | Unlicense |
| [FFmpeg](https://ffmpeg.org) | 合并视频与音频、格式转换 | GPLv3 |
| [QuickJS-ng](https://github.com/quickjs-ng/quickjs) | 处理 YouTube 的 JavaScript 验证 | MIT |

安装时会下载各项目的官方构建版本。
详细声明见 [THIRD-PARTY-NOTICES](../licenses/THIRD-PARTY-NOTICES.txt)。

---

## 请注意

受版权保护的内容，**仅在获得权利人许可的情况下** 才可下载。
本程序面向你自己拥有、已获授权或无版权限制的内容。
使用所产生的责任由使用者承担。

---

## 许可证

[MIT](../LICENSE)。源代码可自由阅读、修改和再利用。

---

## 代码签名政策 (Code signing policy)

Free code signing provided by [SignPath.io](https://about.signpath.io), certificate by [SignPath Foundation](https://signpath.org).

| 角色 | 负责人 |
|---|---|
| 提交与审查 | [Bruce Jung](https://github.com/bruce-jsh) |
| 签名批准 | [Bruce Jung](https://github.com/bruce-jsh) |

**隐私政策。** 除非用户或安装者明确要求，本程序不会向任何外部系统发送信息。

它会连接 YouTube 以获取你要的视频，连接 i.ytimg.com 以显示列表缩略图，连接 GitHub 以检查 yt-dlp 和本程序的新版本。版本检查可在设置中关闭。没有账号、无需登录，也不记录使用情况。

---

<div align="center">

几天前我辞职了。在家闲得发慌，就写了这个。<br>
梦想是做自由职业者，工作室是韩国首尔中浪区墨洞的一个小房间。
2026年10月31日。

**Bruce Jung** · [sunghunet@gmail.com](mailto:sunghunet@gmail.com)

</div>
