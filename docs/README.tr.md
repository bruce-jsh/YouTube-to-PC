<div align="center">

<img src="../res/logo.png" width="96" alt="YouTube to PC">

# YouTube to PC

**YouTube videolarını ve müziklerini bilgisayarınıza kaydetmenin en basit yolu**

Bir bağlantı kopyalayın, tek bir düğmeye basın. Hepsi bu.

<br>

### [⬇️ YouTubeToPC-Setup-1.3.2.exe indir](https://github.com/bruce-jsh/YouTube-to-PC/releases/download/v1.3.2/YouTubeToPC-Setup-1.3.2.exe)

1,3 MB · Windows 10/11 (64 bit) · düğmeye basın, dosya hemen insin

Windows ya da tarayıcınız engelliyor mu? [Nasıl aşılacağı burada](troubleshooting.tr.md).


<img src="images/app-main.png" width="620" alt="YouTube to PC">

*Bağlantıyı yapıştırın, indirme başlasın.*

<img src="images/app-menu.png" width="620" alt="">

*Listeden kaldırmak ile dosyayı silmek ayrı seçeneklerdir.*

<img src="images/app-languages.png" width="620" alt="">

*On beş dil, ayarlardan seçilir.*

<br>

[한국어](README.ko.md) ·
**Türkçe** ·
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
[Bahasa Indonesia](README.id.md) ·
[日本語](README.ja.md)


</div>

---

## İçiniz rahat olsun

**Ayrıca kurmanız gereken hiçbir şey yok.**
.NET Framework, Visual C++ yeniden dağıtılabilir paketleri, Python ya da benzeri
bir şeye gerek yok. Bu, yalnızca Windows'ta hâlihazırda bulunanları kullanan sade
bir Win32 programıdır. İndirin ve çalıştırın.

**Reklam yok. Hiç yok.**
Afiş yok, açılır pencere yok, "hemen yükseltin" uyarısı yok. Ücretli bir sürümü de yok.

**Kişisel veri toplanmıyor.**
Kayıt yok, oturum açma yok. Kullanımınıza dair hiçbir şey hiçbir yere gönderilmez.
Ayarlarınız ve indirme listeniz yalnızca kendi bilgisayarınızda kalır.

**Yanında başka program gelmiyor.**
Kurulum, araya başka yazılım sıkıştırmaz ve tarayıcı ayarlarınıza dokunmaz.

---

## Neler yapar

- **Bağlantı Yapıştır** düğmesine basın, indirme başlasın (`Ctrl+V` de çalışır)
- **Video** (MP4, MKV) ya da **müzik** (MP3, M4A) seçin
- Kalite **en iyisinden** 144p'ye kadar; ses en iyisinden 128 kbps'ye kadar
- **Oynatma listesinin ya da kanalın tamamını** indirin veya yalnızca istediğiniz videoları işaretleyin
- **Altyazı ve ses parçası** seçin: yaklaşık 85 dil, her biri kendi yazısıyla gösterilir
- Aynı anda 1 ile 8 arası indirme
- **Açık ve koyu tema** ya da Windows ayarınızı izleyin
- **15 dilde** kullanılabilir
- Küçük resimler, ilerleme ve kalan süre
- Dosyayı oynatın ya da klasörünü açın
- Kaydedilmiş videodan sesi çıkarın
- İndirme geçmişini saklar

---

## Kurulum

1. Yukarıdaki indirme düğmesine basın. Kurulum dosyası hemen kaydedilir.
2. Dosyayı çalıştırın.
3. Dilinizi ve kurulum klasörünü seçip **Kur** düğmesine basın.

Kurulum için internet bağlantısı gerekir: uygulamanın kullandığı araçları
(yt-dlp ve FFmpeg) indirir ve **kendi kopyalarını program klasöründe tutar**.
Bu genellikle 10 ila 30 saniye sürer.

> **Windows "Bilgisayarınız korundu" uyarısı gösterirse**
> **Ek bilgi → Yine de çalıştır** seçeneğine tıklayın.
> Bu uyarı, programın tek bir kişi tarafından yapılmış olması ve ücretli bir kod
> imzalama sertifikası bulunmaması nedeniyle çıkar. Virüs değildir.

Kaldırmak için **Ayarlar → Uygulamalar** bölümünü kullanın ya da kurulum
klasöründeki `Uninstall.exe` dosyasını çalıştırın. İndirdiğiniz videolar ve
müzikler asla silinmez.

---

## Kullanımı

1. YouTube'da bir video adresini kopyalayın.
2. Uygulamada **Bağlantı Yapıştır** düğmesine basın.
3. Tamam. Listede belirir ve indirilmeye başlar.

Her video için kalite ya da altyazı seçmek isterseniz **Anında** seçeneğini kapatın.
O zaman her bağlantı eklediğinizde bir seçenek penceresi açılır.

Bir oynatma listesi ya da kanal adresi yapıştırdığınızda videoların tam listesini,
yalnızca istediklerinizi işaretleyebileceğiniz kutucuklarla birlikte görürsünüz.

---

## Sorular

**İndirmeler aniden çalışmaz oldu.**
YouTube değiştiğinde indirme aracının da onunla birlikte değişmesi gerekir.
Uygulama günde bir kez yeni sürüm denetler, ama dilediğiniz zaman
**Araçlar → yt-dlp güncelle** komutunu kendiniz de çalıştırabilirsiniz.

**Şirket ağımda çalışmıyor.**
**Ayarlar → Bağlantı** bölümüne bir proxy adresi girmeyi deneyin.

**Yaş sınırlı videolar başarısız oluyor.**
**Ayarlar → Oturum açma** bölümünde, YouTube'da oturum açtığınız tarayıcıyı seçin.

---

## Kendiniz derlemek

```
build.bat
```

Windows'ta tek gereken [MSYS2](https://www.msys2.org) içindeki UCRT64 g++ derleyicisidir.
Visual Studio da .NET SDK de gerekmez.

---

## Kullandığı araçlar

| Ad | Görevi | Lisans |
|---|---|---|
| [yt-dlp](https://github.com/yt-dlp/yt-dlp) | YouTube'dan indirme | Unlicense |
| [FFmpeg](https://ffmpeg.org) | Video ve sesi birleştirme, dönüştürme | GPLv3 |
| [QuickJS-ng](https://github.com/quickjs-ng/quickjs) | YouTube'un JavaScript denetimlerini çalıştırma | MIT |

Kurulum, her projenin resmî sürümünü indirir.
Bildirimlerin tamamı [THIRD-PARTY-NOTICES](../licenses/THIRD-PARTY-NOTICES.txt) dosyasındadır.

---

## Lütfen dikkat

Yalnızca kaydetme hakkına sahip olduğunuz içerikleri indirin.
Bu araç; size ait olan, izin aldığınız ya da telif hakkı bulunmayan içerikler içindir.
Nasıl kullandığınız sizin sorumluluğunuzdadır.

---

## Lisans

[MIT](../LICENSE). Kaynak kod okunabilir, değiştirilebilir ve yeniden kullanılabilir.

---

## Kod imzalama politikası (Code signing policy)

Free code signing provided by [SignPath.io](https://about.signpath.io), certificate by [SignPath Foundation](https://signpath.org).

| Rol | Kim |
|---|---|
| Geliştirme ve inceleme | [Bruce Jung](https://github.com/bruce-jsh) |
| İmza onayı | [Bruce Jung](https://github.com/bruce-jsh) |

**Gizlilik politikası.** Bu program, kullanıcı ya da kurulumu yapan kişi özellikle istemedikçe hiçbir bilgiyi başka sistemlere göndermez.

İstediğiniz videolar için YouTube'a, listedeki küçük resimler için i.ytimg.com'a, yt-dlp ve bu programın yeni sürümleri için GitHub'a bağlanır. Sürüm denetimleri ayarlardan kapatılabilir. Hesap, oturum açma ve kullanım takibi yoktur.

---

<div align="center">

Birkaç gün önce işimden ayrıldım. Evde canım sıkıldı, bunun yerine bunu yaptım.<br>
Hayalim serbest çalışmak; Kore'nin Seul kentindeki Mukdong'da küçük bir odadan.<br>
31 Ekim 2026.

**Bruce Jung** · [sunghunet@gmail.com](mailto:sunghunet@gmail.com)

</div>
