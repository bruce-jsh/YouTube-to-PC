<div align="center">

<img src="../res/logo.png" width="72" alt="YouTube to PC">

# If Windows will not let you download or install

</div>

---

**None of these warnings mean the program is dangerous.**

Windows trusts software by reputation, and reputation is bought with a signing
certificate. This program is written by one person and does not have one yet,
so Windows treats it as a stranger and says so loudly.

Find your warning below. Most of them take three clicks.

---

## The quickest fix: use a different browser

Microsoft Edge is the strict one. Chrome and Firefox download this file without
a word.

If Edge is giving you trouble, open the
[download page](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest) in
Chrome or Firefox and press the button there. That is the whole fix.

If you would rather stay in Edge, read on.

---

## 1. Edge blocks the download

A note appears saying the file **isn't commonly downloaded**.

<img src="images/block-1-notice.png" width="380" alt="Edge says the file is not commonly downloaded">

**Step 1.** Press `Ctrl` + `J` to open **Downloads**, then press the **…**
button on that row.

<img src="images/block-2-menu.png" width="520" alt="The three dots menu with Keep and Report this file as safe">

**Step 2.** Choose **Keep**.

**Step 3.** A second window appears. This is where people get stuck: the only
obvious button says **Delete**. Press the **small arrow next to Delete**, then
choose **Keep anyway**.

<img src="images/block-3-keep.png" width="370" alt="Press the arrow beside Delete to reveal Keep anyway">

The download then completes normally.

> While you are in that menu, **Report this file as safe** is worth pressing.
> The more people who do, the sooner this warning stops appearing for everyone.

---

## 2. "Windows protected your PC" when you run it

A blue window may appear saying Windows protected your PC, offering only a
**Don't run** button.

1. Click the small **More info** link under the message. It is plain text, not
   a button, and it is easy to miss
2. A new button appears at the bottom: **Run anyway**
3. Press it

This window usually appears the first time you run the file on a PC. Once you
allow it, Windows will not ask again on that PC.

---

## 3. The antivirus deleted the file

The download finishes and the file is gone.

1. Open **Windows Security** from the Start menu
2. Go to **Virus & threat protection**
3. Under **Protection history** find the `YouTubeToPC-Setup` entry
4. Open it, press **Actions**, choose **Allow on device**
5. Download the file again

For other antivirus software (AhnLab V3, ALYac, Avast, AVG, Norton), add an
exception for the installer or for the folder it installs into:

```
C:\Program Files\YouTube to PC
```

Or pause real-time protection for a minute, install, then switch it back on.

---

## 4. Setup starts but stops partway

Messages about yt-dlp or about checking your internet connection mean the
installer could not fetch the tools it needs.

The installer is only about 1 MB. While installing it downloads yt-dlp, FFmpeg
and QuickJS from their official pages, roughly 110 MB in total, and keeps its
own copies inside the program folder.

Try these in order.

**a. Turn off your VPN and run setup again.** This is the most common cause.

**b. Let setup through the firewall.** Open **Windows Security**,
**Firewall & network protection**, **Allow an app through firewall**,
**Change settings**, **Allow another app**, **Browse**, pick the setup file,
**Add**, then tick both **Private** and **Public**.

**c. Try another network.** Company, school and some public networks block
GitHub, which is where the tools come from. A home connection or a phone
hotspot usually works.

---

## 5. Nothing happens at all

- **Double-clicking does nothing.** Right-click the file and choose
  **Run as administrator**.
- **"This app can't run on your PC".** You need 64-bit Windows 10 or 11.
  Check in **Settings**, **System**, **About**.
- **The file is 0 bytes or will not open.** The download was cut short. Delete
  it and download again.

---

## Checking the file is genuine

Every release page lists the SHA-256 of the installer. Open PowerShell in the
folder holding the file and run:

```powershell
Get-FileHash .\YouTubeToPC-Setup-1.3.0.exe -Algorithm SHA256
```

If it matches the value on the
[release page](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest), you
have exactly the file that was published and nothing has altered it.

---

## Why this happens

A signing certificate costs money every year, and without one Windows has no
way to tell who wrote this program. In the meantime the source code is open,
every release is built in public by
[GitHub Actions](https://github.com/bruce-jsh/YouTube-to-PC/actions), and the
hash above lets you confirm you received that exact build.

Work to get the program signed is under way. These warnings will go away once
it is done.

---

## Still stuck

Write to **[sunghunet@gmail.com](mailto:sunghunet@gmail.com)** or open an
[issue](https://github.com/bruce-jsh/YouTube-to-PC/issues). Tell me what the
message said and which step it appeared at. A screenshot helps a lot.

<div align="center">

[Back to the main page](../README.md)

</div>
