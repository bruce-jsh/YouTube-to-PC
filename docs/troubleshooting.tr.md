<div align="center">

<img src="../res/logo.png" width="72" alt="YouTube to PC">

# Windows indirmeye ya da kurmaya izin vermediğinde

</div>

---

**Buradaki uyarıların hiçbiri programın tehlikeli olduğu anlamına gelmez.**

Windows yazılıma itibarına göre güvenir ve o itibar bir imzalama sertifikasıyla
satın alınır. Bu programı tek bir kişi yazıyor ve henüz sertifikası yok; Windows da
onu yabancı sayıp yüksek sesle uyarıyor.

Karşılaştığınız uyarıyı aşağıdan bulun. Çoğu üç tıklamayla biter.

---

## En hızlısı: başka bir tarayıcı kullanmak

Katı olan Microsoft Edge. **Chrome ve Firefox bu dosyayı hiç ses çıkarmadan
indiriyor.**

Edge size zorluk çıkarıyorsa
[indirme sayfasını](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest)
Chrome veya Firefox ile açıp düğmeye orada basın. Hepsi bu.

Edge'de kalmak isterseniz okumaya devam edin.

---

## 1. Edge indirmeyi engelliyor

Dosyanın **yaygın olarak indirilmediğini** söyleyen bir uyarı çıkar.

<img src="images/block-1-notice.png" width="380" alt="Edge dosyanın yaygın olarak indirilmediğini bildiriyor">

**Adım 1.** `Ctrl` + `J` ile **İndirilenler**'i açın, o satırdaki **…** düğmesine
basın.

<img src="images/block-2-menu.png" width="520" alt="Sakla ve bu dosyayı güvenli olarak bildir seçeneklerini içeren üç nokta menüsü">

**Adım 2.** **Sakla** (Keep) seçin.

**Adım 3.** İkinci bir pencere açılır. İnsanların takıldığı yer burası: görünen tek
düğmede **Sil** (Delete) yazar. **Sil'in yanındaki küçük oka** basın ve
**Yine de sakla** (Keep anyway) seçeneğini seçin.

<img src="images/block-3-keep.png" width="370" alt="Sil'in yanındaki ok Yine de sakla seçeneğini ortaya çıkarır">

Ardından indirme normal şekilde tamamlanır.

> Hazır o menüdeyken **Bu dosyayı güvenli olarak bildir** (Report this file as safe)
> seçeneğine basmaya değer. Ne kadar çok kişi bildirirse bu uyarı herkes için o
> kadar çabuk ortadan kalkar.

---

## 2. Çalıştırırken "Windows protected your PC" çıkıyor

Mavi bir pencere çıkabilir ve yalnızca **Don't run** düğmesini gösterir.

1. Yazının hemen altındaki küçük **More info** bağlantısına tıklayın. Düğme değil,
   düz metindir; gözden kaçması çok kolaydır
2. Altta yeni bir düğme belirir: **Run anyway**
3. Ona basın

Bu pencere genelde bir bilgisayarda ilk çalıştırmada çıkar. Bir kez izin
verdiğinizde Windows o bilgisayarda bir daha sormaz.

---

## 3. Antivirüs dosyayı sildi

İndirme bitti ama dosya ortada yok.

1. Başlat menüsünden **Windows Security**'yi açın
2. **Virus & threat protection** bölümüne gidin
3. **Protection history** içinde `YouTubeToPC-Setup` kaydını bulun
4. Açın, **Actions**'a basın ve **Allow on device** seçin
5. Dosyayı yeniden indirin

Başka bir antivirüs kullanıyorsanız (Avast, AVG, Norton), kurulum dosyası ya da
kurulum klasörü için bir istisna ekleyin:

```
C:\Program Files\YouTube to PC
```

Ya da gerçek zamanlı korumayı bir dakikalığına durdurup kurduktan sonra geri açın.

---

## 4. Kurulum başlıyor ama yarıda duruyor

yt-dlp ile ilgili ya da internet bağlantınızı denetlemenizi isteyen iletiler,
kurulum programının ihtiyaç duyduğu araçları indiremediği anlamına gelir.

Kurulum dosyası yalnızca 1 MB kadardır. Kurulum sırasında yt-dlp, FFmpeg ve
QuickJS'i kendi resmi sayfalarından toplam 110 MB civarında indirir ve kendi
kopyalarını program klasörünün içinde tutar.

Sırayla deneyin.

**a. VPN'i kapatıp kurulumu yeniden çalıştırın.** En sık görülen neden budur.

**b. Güvenlik duvarından izin verin.** **Windows Security** →
**Firewall & network protection** → **Allow an app through firewall** →
**Change settings** → **Allow another app** → **Browse** ile kurulum dosyasını
seçin, **Add**, ardından hem **Private** hem **Public** kutularını işaretleyin.

**c. Başka bir ağ deneyin.** Şirket, okul ve bazı genel ağlar araçların indirildiği
yer olan GitHub'ı engeller. Ev bağlantısı veya telefonun mobil erişim noktası
genelde çalışır.

---

## 5. Hiçbir şey olmuyor

- **Çift tıklıyorum, hiçbir şey olmuyor.** Dosyaya sağ tıklayıp
  **Run as administrator** seçin.
- **"This app can't run on your PC".** 64 bit Windows 10 veya 11 gerekir.
  **Settings**, **System**, **About** bölümünden bakabilirsiniz.
- **Dosya 0 bayt ya da açılmıyor.** İndirme yarıda kesilmiş. Silip yeniden indirin.

---

## Dosyanın gerçek olduğunu doğrulamak

Her sürüm sayfasında kurulum dosyasının SHA-256 değeri yazar. Dosyanın bulunduğu
klasörde PowerShell açıp şunu çalıştırın:

```powershell
Get-FileHash .\YouTubeToPC-Setup-1.3.1.exe -Algorithm SHA256
```

Değer
[sürüm sayfasındakiyle](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest)
aynıysa elinizdeki tam olarak yayımlanan dosyadır ve kimse ona dokunmamıştır.

---

## Bu neden oluyor

İmzalama sertifikası her yıl para ister ve o olmadan Windows'un bu programı kimin
yazdığını anlamasının bir yolu yoktur.

Buna karşılık kaynak kodu açık, her sürüm
[GitHub Actions](https://github.com/bruce-jsh/YouTube-to-PC/actions) tarafından
herkesin gözü önünde derleniyor ve yukarıdaki özet değeri tam olarak o derlemeyi
aldığınızı doğrulamanızı sağlıyor.

İmza için çalışmalar sürüyor. Tamamlandığında bu uyarılar ortadan kalkacak.

---

## Yine de olmuyorsa

**[sunghunet@gmail.com](mailto:sunghunet@gmail.com)** adresine yazın ya da bir
[issue](https://github.com/bruce-jsh/YouTube-to-PC/issues) açın. İletide ne
yazdığını ve hangi adımda çıktığını anlatın. Ekran görüntüsü çok yardımcı olur.

<div align="center">

[Ana sayfaya dön](README.tr.md)

</div>
