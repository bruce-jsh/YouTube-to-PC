<div align="center">

<img src="../res/app.ico" width="96" alt="YouTube to PC">

# YouTube to PC

**Cara paling sederhana menyimpan video dan musik YouTube ke PC Anda**

Salin tautan, tekan satu tombol. Itu saja.

<br>

### [⬇️ Unduh YouTubeToPC-Setup-1.2.0.exe](https://github.com/bruce-jsh/YouTube-to-PC/releases/download/v1.2.0/YouTubeToPC-Setup-1.2.0.exe)

1,3 MB · Windows 10/11 (64-bit) · tekan tombolnya, berkas langsung terunduh

<br>

[한국어](README.ko.md) ·
[Türkçe](README.tr.md) ·
[Монгол](README.mn.md) ·
[English](../README.md) ·
[简体中文](README.zh-Hans.md) ·
[繁體中文](README.zh-Hant.md) ·
[Español](README.es.md) ·
[Français](README.fr.md) ·
[Deutsch](README.de.md) ·
[Português](README.pt.md) ·
[Русский](README.ru.md) ·
[Tiếng Việt](README.vi.md) ·
[ไทย](README.th.md) ·
**Bahasa Indonesia** ·
[日本語](README.ja.md)


</div>

---

## Aman dipakai tanpa khawatir

**Tidak perlu memasang apa pun lagi.**
Tidak butuh .NET Framework, Visual C++ Redistributable, Python, atau sejenisnya.
Ini program Win32 biasa yang hanya memakai apa yang sudah ada di Windows.
Unduh lalu jalankan.

**Tanpa iklan. Sama sekali.**
Tidak ada spanduk, tidak ada jendela sembul, tidak ada ajakan "tingkatkan sekarang".
Versi berbayar pun tidak ada.

**Tidak mengumpulkan data pribadi.**
Tanpa pendaftaran, tanpa login. Tidak ada data pemakaian yang dikirim ke mana pun.
Pengaturan dan daftar unduhan hanya tersimpan di PC Anda sendiri.

**Tanpa program bawaan tambahan.**
Pemasangan tidak menyelipkan perangkat lunak lain dan tidak mengubah pengaturan peramban.

---

## Yang bisa dilakukan

- Tekan **Tempel Tautan** dan unduhan langsung dimulai (`Ctrl+V` juga bisa)
- Pilih **video** (MP4, MKV) atau **musik** (MP3, M4A)
- Kualitas dari **terbaik yang tersedia** hingga 144p; audio dari terbaik hingga 128 kbps
- Unduh **seluruh playlist atau kanal**, atau centang hanya video yang Anda mau
- Pilih **subtitel dan trek audio**: sekitar 85 bahasa, masing-masing ditampilkan dengan aksaranya sendiri
- Mengunduh 1 sampai 8 berkas sekaligus
- **Tema terang dan gelap**, atau mengikuti pengaturan Windows
- Tersedia dalam **15 bahasa**
- Gambar mini, progres, dan sisa waktu
- Putar berkas atau buka foldernya
- Ekstrak audio dari video tersimpan
- Menyimpan riwayat unduhan

---

## Cara memasang

1. Tekan tombol unduh di atas. Pemasang langsung tersimpan.
2. Jalankan berkasnya.
3. Pilih bahasa dan folder pemasangan, lalu tekan **Pasang**.

Pemasangan memerlukan koneksi internet: ia mengunduh alat yang dipakai aplikasi
(yt-dlp dan FFmpeg) dan **menyimpan salinannya sendiri di dalam folder program**.
Biasanya memakan waktu 10 sampai 30 detik.

> **Jika muncul "Windows protected your PC"**
> Klik **More info → Run anyway**.
> Peringatan itu muncul karena program ini dibuat satu orang dan belum memiliki
> sertifikat penandatanganan kode berbayar. Ini bukan virus.

Untuk menghapusnya, gunakan **Pengaturan → Aplikasi**, atau jalankan `Uninstall.exe`
di folder pemasangan. Video dan musik yang sudah diunduh tidak akan terhapus.

---

## Cara memakai

1. Salin alamat video di YouTube.
2. Tekan tombol **Tempel Tautan** di aplikasi.
3. Selesai. Video muncul di daftar dan langsung diunduh.

Kalau ingin memilih kualitas atau subtitel untuk tiap video, matikan **Langsung**.
Setelah itu jendela pilihan akan terbuka setiap kali Anda menambah tautan.

Tempelkan alamat playlist atau kanal, dan Anda akan melihat daftar lengkap video
dengan kotak centang untuk memilih yang diperlukan saja.

---

## Pertanyaan

**Tiba-tiba tidak bisa mengunduh.**
Ketika YouTube berubah, alat pengunduh harus ikut diperbarui.
Aplikasi memeriksa sekali sehari, tetapi Anda bisa menjalankan
**Alat → Perbarui yt-dlp** kapan saja.

**Tidak jalan di jaringan kantor.**
Coba isi alamat proxy di **Pengaturan → Koneksi**.

**Video dengan batasan usia gagal diunduh.**
Di **Pengaturan → Masuk**, pilih peramban tempat Anda sudah masuk ke YouTube.

---

## Membangun sendiri

```
build.bat
```

Di Windows cukup g++ UCRT64 dari [MSYS2](https://www.msys2.org).
Tanpa Visual Studio, tanpa .NET SDK.

---

## Alat yang digunakan

| Nama | Fungsi | Lisensi |
|---|---|---|
| [yt-dlp](https://github.com/yt-dlp/yt-dlp) | Mengunduh dari YouTube | Unlicense |
| [FFmpeg](https://ffmpeg.org) | Menggabungkan video dan audio, mengonversi | GPLv3 |
| [QuickJS-ng](https://github.com/quickjs-ng/quickjs) | Menjalankan pemeriksaan JavaScript YouTube | MIT |

Pemasangan mengunduh versi resmi dari masing-masing proyek.
Pemberitahuan lengkap ada di [THIRD-PARTY-NOTICES](../licenses/THIRD-PARTY-NOTICES.txt).

---

## Harap diperhatikan

Unduhlah hanya konten yang memang berhak Anda simpan.
Alat ini ditujukan untuk konten milik Anda sendiri, konten yang sudah diizinkan,
atau konten bebas hak cipta. Tanggung jawab penggunaan ada pada Anda.

---

<div align="center">

Beberapa hari lalu saya berhenti kerja. Bosan di rumah, jadilah aplikasi ini.<br>
Saya bermimpi jadi pekerja lepas, dari satu kamar kecil di Mukdong, Seoul di Korea.
31 Oktober 2026.

**Bruce Jung** · [sunghunet@gmail.com](mailto:sunghunet@gmail.com)

</div>
