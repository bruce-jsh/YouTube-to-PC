<div align="center">

<img src="../res/logo.png" width="72" alt="YouTube to PC">

# Kalau Windows tidak mengizinkan unduhan atau pemasangan

</div>

---

**Tidak satu pun peringatan di sini berarti programnya berbahaya.**

Windows memercayai perangkat lunak berdasarkan reputasi, dan reputasi itu dibeli
dengan sertifikat penandatanganan. Program ini ditulis satu orang dan belum punya
sertifikat, jadi Windows memperlakukannya sebagai orang asing dan memperingatkan
dengan keras.

Cari peringatan Anda di bawah. Sebagian besar selesai dengan tiga klik.

---

## Cara tercepat: pakai peramban lain

Yang ketat adalah Microsoft Edge. **Chrome dan Firefox mengunduh berkas ini tanpa
berkomentar.**

Kalau Edge menghalangi, buka
[halaman unduhan](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest) di
Chrome atau Firefox lalu tekan tombolnya di sana. Hanya itu.

Kalau tetap ingin memakai Edge, lanjutkan membaca.

---

## 1. Edge memblokir unduhan

Muncul pemberitahuan bahwa berkas ini **jarang diunduh**.

<img src="images/block-1-notice.png" width="380" alt="Edge memberitahu bahwa berkas ini jarang diunduh">

**Langkah 1.** Tekan `Ctrl` + `J` untuk membuka **Unduhan**, lalu tekan tombol
**…** di baris itu.

<img src="images/block-2-menu.png" width="520" alt="Menu tiga titik dengan Simpan dan Laporkan berkas ini aman">

**Langkah 2.** Pilih **Simpan** (Keep).

**Langkah 3.** Jendela kedua muncul. Di sinilah orang tersangkut: satu-satunya
tombol yang terlihat bertuliskan **Hapus** (Delete). Tekan **panah kecil di
sebelah Hapus**, lalu pilih **Tetap simpan** (Keep anyway).

<img src="images/block-3-keep.png" width="370" alt="Tekan panah di sebelah Hapus untuk memunculkan Tetap simpan">

Setelah itu unduhan selesai seperti biasa.

> Selagi di menu itu, **Laporkan berkas ini aman** (Report this file as safe) layak
> ditekan. Makin banyak yang melapor, makin cepat peringatan ini hilang untuk semua
> orang.

---

## 2. "Windows protected your PC" saat dijalankan

Jendela biru bisa muncul dan hanya menawarkan tombol **Don't run**.

1. Klik tautan kecil **More info** tepat di bawah pesannya. Itu teks biasa, bukan
   tombol, dan mudah terlewat
2. Tombol baru muncul di bawah: **Run anyway**
3. Tekan tombol itu

Jendela ini biasanya muncul saat pertama kali menjalankannya di sebuah PC. Setelah
Anda mengizinkannya sekali, Windows tidak akan bertanya lagi di PC tersebut.

---

## 3. Antivirus menghapus berkasnya

Unduhan selesai tetapi berkasnya lenyap.

1. Buka **Windows Security** dari menu Start
2. Masuk ke **Virus & threat protection**
3. Di **Protection history** cari entri `YouTubeToPC-Setup`
4. Buka, tekan **Actions**, lalu pilih **Allow on device**
5. Unduh berkasnya lagi

Untuk antivirus lain (Avast, AVG, Norton, Smadav), tambahkan pengecualian untuk
pemasang atau untuk folder pemasangannya:

```
C:\Program Files\YouTube to PC
```

Atau jeda perlindungan waktu nyata sebentar, pasang, lalu nyalakan lagi.

---

## 4. Pemasangan mulai lalu berhenti di tengah

Pesan tentang yt-dlp atau tentang memeriksa koneksi internet berarti pemasang tidak
bisa mengunduh perkakas yang dibutuhkannya.

Pemasangnya hanya sekitar 1 MB. Saat memasang, ia mengunduh yt-dlp, FFmpeg dan
QuickJS dari halaman resmi masing-masing, sekitar 110 MB seluruhnya, dan menyimpan
salinannya sendiri di dalam folder program.

Coba berurutan.

**a. Matikan VPN lalu jalankan pemasang lagi.** Ini penyebab paling umum.

**b. Izinkan lewat firewall.** **Windows Security** →
**Firewall & network protection** → **Allow an app through firewall** →
**Change settings** → **Allow another app** → **Browse**, pilih berkas pemasang,
**Add**, lalu centang **Private** dan **Public**.

**c. Coba jaringan lain.** Jaringan kantor, sekolah dan sebagian jaringan publik
memblokir GitHub, padahal dari sanalah perkakasnya diambil. Koneksi rumah atau
tethering ponsel biasanya berhasil.

---

## 5. Sama sekali tidak terjadi apa-apa

- **Klik dua kali tidak ada reaksi.** Klik kanan berkasnya lalu pilih
  **Run as administrator**.
- **"This app can't run on your PC".** Diperlukan Windows 10 atau 11 versi 64-bit.
  Periksa di **Settings**, **System**, **About**.
- **Berkasnya 0 byte atau tidak bisa dibuka.** Unduhan terputus. Hapus lalu unduh
  ulang.

---

## Memastikan berkasnya asli

Setiap halaman rilis mencantumkan SHA-256 pemasangnya. Buka PowerShell di folder
tempat berkas berada lalu jalankan:

```powershell
Get-FileHash .\YouTubeToPC-Setup-1.3.3.exe -Algorithm SHA256
```

Kalau cocok dengan nilai di
[halaman rilis](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest),
berkas Anda persis yang diterbitkan dan tidak ada yang mengubahnya.

---

## Kenapa ini terjadi

Sertifikat penandatanganan memakan biaya setiap tahun, dan tanpa itu Windows tidak
punya cara mengetahui siapa yang menulis program ini.

Sebagai gantinya kode sumbernya terbuka, setiap rilis dibangun secara terbuka oleh
[GitHub Actions](https://github.com/bruce-jsh/YouTube-to-PC/actions), dan hash di
atas memungkinkan Anda memastikan sendiri bahwa yang Anda terima adalah build itu.

Upaya mendapatkan tanda tangan sedang berjalan. Setelah selesai, peringatan ini
akan hilang.

---

## Masih tidak bisa

Kirim surel ke **[sunghunet@gmail.com](mailto:sunghunet@gmail.com)** atau buka
[issue](https://github.com/bruce-jsh/YouTube-to-PC/issues). Ceritakan bunyi
pesannya dan di langkah mana muncul. Tangkapan layar sangat membantu.

<div align="center">

[Kembali ke halaman utama](README.id.md)

</div>
