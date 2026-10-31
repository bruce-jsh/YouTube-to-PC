<div align="center">

<img src="../res/logo.png" width="72" alt="YouTube to PC">

# 下載或安裝被攔截時

</div>

---

**這裡的警告都不代表程式有危險。**

Windows 依靠信譽來信任軟體，而信譽要靠購買簽署憑證取得。這個程式由一個人撰寫，
目前還沒有憑證，所以 Windows 把它當成陌生程式，並大聲提醒。

在下面找到你遇到的畫面。大多數情況點三下就好。

---

## 最快的辦法：換一個瀏覽器

嚴格的是 Microsoft Edge。**Chrome 和 Firefox 都會直接下載，不會攔。**

如果 Edge 擋住了你，用 Chrome 或 Firefox 開啟
[下載頁面](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest)
再按按鈕即可。這就是全部。

想繼續用 Edge 的話，請往下看。

---

## 1. Edge 攔截下載

會出現提示，說這個檔案**不常被下載**。

<img src="images/block-1-notice.png" width="380" alt="Edge 提示該檔案不常被下載">

**第 1 步.** 按 `Ctrl` + `J` 開啟**下載**清單，點選該列的 **…** 按鈕。

<img src="images/block-2-menu.png" width="520" alt="包含保留與回報此檔案安全的三點選單">

**第 2 步.** 選擇**保留**(Keep)。

**第 3 步.** 會跳出第二個視窗。很多人卡在這裡：看得見的按鈕只有**刪除**(Delete)。
點**刪除旁邊的小箭頭**，就會出現**仍要保留**(Keep anyway)。

<img src="images/block-3-keep.png" width="370" alt="點選刪除旁的箭頭會出現仍要保留">

之後下載就會正常完成。

> 順手點一下選單裡的**回報此檔案安全**(Report this file as safe) 也很有幫助。
> 回報的人越多，後來的人越不會看到這個警告。

---

## 2. 執行時出現「Windows 已保護您的電腦」

可能會出現一個藍色視窗，只給出**不要執行**按鈕。

1. 點選提示文字下方那個小小的**其他資訊**連結。它是純文字而不是按鈕，很容易漏看
2. 底部會新出現一個**仍要執行**按鈕
3. 按它

這個視窗通常只在某台電腦上第一次執行時出現。允許一次之後，同一台電腦不會再問。

---

## 3. 防毒軟體刪掉了檔案

下載完成後檔案卻不見了。

1. 從開始功能表開啟 **Windows 安全性**
2. 進入**病毒與威脅防護**
3. 在**保護歷程記錄**中找到 `YouTubeToPC-Setup` 這一項
4. 展開它，按**動作**，選擇**允許在裝置上**
5. 重新下載檔案

如果用的是其他防毒軟體（Avast、AVG、Norton 等），請為安裝程式或安裝資料夾新增例外。

```
C:\Program Files\YouTube to PC
```

也可以先暫停即時防護，裝完再開啟。

---

## 4. 安裝到一半停住

出現取不到 yt-dlp 或提示檢查網路連線的訊息，表示安裝程式沒能下載所需的工具。

安裝程式本身只有 1 MB 左右。安裝過程中它會從各自的官方頁面下載 yt-dlp、FFmpeg 與
QuickJS，合計約 110 MB，並在程式資料夾內保留自己的副本。

請依序嘗試。

**a. 關掉 VPN 再裝一次。** 這是最常見的原因。

**b. 在防火牆裡放行。** **Windows 安全性** → **防火牆與網路保護** →
**允許應用程式通過防火牆** → **變更設定** → **允許其他應用程式** → **瀏覽**選取
安裝程式 → **新增**，並同時勾選**私人**與**公用**。

**c. 換一個網路。** 公司、學校與部分公共網路會封鎖 GitHub，而工具正是從那裡下載的。
用家裡的網路或手機熱點通常就可以。

---

## 5. 完全沒有反應

- **按兩下沒有任何動靜。** 在檔案上按右鍵，選擇**以系統管理員身分執行**。
- **「這個應用程式無法在你的電腦上執行」。** 需要 64 位元的 Windows 10 或 11。
  可在**設定 → 系統 → 關於**中查看。
- **檔案是 0 位元組或打不開。** 下載被中斷了。刪掉重新下載。

---

## 檢查檔案是否為原版

每個發行頁面都會列出安裝程式的 SHA-256。在檔案所在的資料夾開啟 PowerShell，執行：

```powershell
Get-FileHash .\YouTubeToPC-Setup-1.3.1.exe -Algorithm SHA256
```

如果和[發行頁面](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest)上的
值一致，表示你拿到的正是公開發行的那個檔案，沒有被任何人改動過。

---

## 為什麼會這樣

簽署憑證每年都要花錢。沒有憑證，Windows 就無從得知這個程式是誰寫的。

作為替代，原始碼是公開的，每個發行版都由
[GitHub Actions](https://github.com/bruce-jsh/YouTube-to-PC/actions) 在公開環境中
建置，上面的雜湊值可以讓你親自確認拿到的就是那次建置。

申請簽署的工作正在進行。完成之後，這些警告就會消失。

---

## 還是不行

請寫信到 **[sunghunet@gmail.com](mailto:sunghunet@gmail.com)**，或者開一個
[issue](https://github.com/bruce-jsh/YouTube-to-PC/issues)。告訴我出現了什麼訊息、
在哪一步出現的。有截圖會非常有幫助。

<div align="center">

[返回主頁](README.zh-Hant.md)

</div>
