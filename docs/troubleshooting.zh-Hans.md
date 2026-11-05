<div align="center">

<img src="../res/logo.png" width="72" alt="YouTube to PC">

# 下载或安装被拦截时

</div>

---

**这里的警告都不代表程序有危险。**

Windows 依靠信誉来信任软件，而信誉要靠购买签名证书获得。这个程序由一个人编写，
目前还没有证书，所以 Windows 把它当作陌生程序，并大声提醒。

在下面找到你遇到的画面。大多数情况点三下就好。

---

## 最快的办法：换一个浏览器

严格的是 Microsoft Edge。**Chrome 和 Firefox 都会直接下载，不会拦。**

如果 Edge 拦住了你，用 Chrome 或 Firefox 打开
[下载页面](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest)
再点按钮即可。这就是全部。

想继续用 Edge 的话，请往下看。

---

## 1. Edge 拦截下载

会出现提示，说这个文件**不常被下载**。

<img src="images/block-1-notice.png" width="380" alt="Edge 提示该文件不常被下载">

**第 1 步.** 按 `Ctrl` + `J` 打开**下载**列表，点击该行的 **…** 按钮。

<img src="images/block-2-menu.png" width="520" alt="包含保留和报告此文件安全的三点菜单">

**第 2 步.** 选择**保留**(Keep)。

**第 3 步.** 会弹出第二个窗口。很多人卡在这里：看得见的按钮只有**删除**(Delete)。
点**删除旁边的小箭头**，就会出现**仍然保留**(Keep anyway)。

<img src="images/block-3-keep.png" width="370" alt="点击删除旁的箭头会出现仍然保留">

之后下载就会正常完成。

> 顺手点一下菜单里的**报告此文件安全**(Report this file as safe) 也很有帮助。
> 报告的人越多，后来的人越不会看到这个警告。

---

## 2. 运行时出现“Windows 已保护你的电脑”

可能会出现一个蓝色窗口，只给出**不运行**按钮。

1. 点击提示文字下方那个小小的**更多信息**链接。它是纯文字而不是按钮，很容易漏看
2. 底部会新出现一个**仍要运行**按钮
3. 点它

这个窗口通常只在某台电脑上第一次运行时出现。允许一次之后，同一台电脑不会再问。

---

## 3. 杀毒软件删掉了文件

下载完成后文件却不见了。

1. 从开始菜单打开 **Windows 安全中心**
2. 进入**病毒和威胁防护**
3. 在**保护历史记录**中找到 `YouTubeToPC-Setup` 这一项
4. 展开它，点**操作**，选择**允许在设备上**
5. 重新下载文件

如果用的是其他杀毒软件（360、火绒、Avast、AVG、Norton 等），请为安装程序或安装
文件夹添加例外。

```
C:\Program Files\YouTube to PC
```

也可以先暂停实时防护，装完再打开。

---

## 4. 安装到一半停住

出现取不到 yt-dlp 或提示检查网络连接的消息，说明安装程序没能下载所需的工具。

安装程序本身只有 1 MB 左右。安装过程中它会从各自的官方页面下载 yt-dlp、FFmpeg 和
QuickJS，合计约 110 MB，并在程序文件夹内保留自己的副本。

请按顺序尝试。

**a. 关掉 VPN 再装一次。** 这是最常见的原因。

**b. 在防火墙里放行。** **Windows 安全中心** → **防火墙和网络保护** →
**允许应用通过防火墙** → **更改设置** → **允许其他应用** → **浏览**选中安装程序 →
**添加**，并同时勾选**专用**和**公用**。

**c. 换一个网络。** 公司、学校和部分公共网络会屏蔽 GitHub，而工具正是从那里下载的。
用家里的宽带或手机热点通常就可以。

---

## 5. 完全没有反应

- **双击没有任何动静。** 右键点击文件，选择**以管理员身份运行**。
- **“这个应用无法在你的电脑上运行”。** 需要 64 位的 Windows 10 或 11。可在
  **设置 → 系统 → 关于**中查看。
- **文件是 0 字节或打不开。** 下载被中断了。删掉重新下载。

---

## 检查文件是否为原版

每个发布页面都会列出安装程序的 SHA-256。在文件所在的文件夹打开 PowerShell，运行：

```powershell
Get-FileHash .\YouTubeToPC-Setup-1.3.3.exe -Algorithm SHA256
```

如果和[发布页面](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest)上的
值一致，说明你拿到的正是公开发布的那个文件，没有被任何人改动过。

---

## 为什么会这样

签名证书每年都要花钱。没有证书，Windows 就无从知道这个程序是谁写的。

作为替代，源代码是公开的，每个发布版都由
[GitHub Actions](https://github.com/bruce-jsh/YouTube-to-PC/actions) 在公开环境中
构建，上面的哈希值可以让你亲自确认拿到的就是那次构建。

申请签名的工作正在进行。完成之后，这些警告就会消失。

---

## 还是不行

请写信到 **[sunghunet@gmail.com](mailto:sunghunet@gmail.com)**，或者提一个
[issue](https://github.com/bruce-jsh/YouTube-to-PC/issues)。告诉我出现了什么消息、
在哪一步出现的。有截图会非常有帮助。

<div align="center">

[返回主页](README.zh-Hans.md)

</div>
