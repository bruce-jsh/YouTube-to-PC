<div align="center">

<img src="../res/app.ico" width="96" alt="YouTube to PC">

# YouTube to PC

**Der einfachste Weg, YouTube-Videos und -Musik auf dem PC zu speichern**

Link kopieren, eine Schaltfläche drücken. Mehr nicht.

<br>

### [⬇️ Herunterladen: zum Installationsprogramm](../download/)

[**YouTubeToPC-Setup-1.2.0.exe holen**](https://github.com/bruce-jsh/YouTube-to-PC/raw/main/download/YouTubeToPC-Setup-1.2.0.exe) · 1,2 MB · Windows 10/11 (64 Bit)

<br>

[한국어](README.ko.md) ·
[Türkçe](README.tr.md) ·
[Монгол](README.mn.md) ·
[English](../README.md) ·
[简体中文](README.zh-Hans.md) ·
[繁體中文](README.zh-Hant.md) ·
[Español](README.es.md) ·
[Français](README.fr.md) ·
**Deutsch** ·
[Português](README.pt.md) ·
[Русский](README.ru.md) ·
[Tiếng Việt](README.vi.md) ·
[ไทย](README.th.md) ·
[Bahasa Indonesia](README.id.md) ·
[日本語](README.ja.md)


</div>

---

## Sie können es bedenkenlos nutzen

**Es muss nichts zusätzlich installiert werden.**
Weder .NET Framework noch Visual C++ Redistributables, Python oder Ähnliches.
Es ist ein schlichtes Win32-Programm, das nur nutzt, was Windows ohnehin mitbringt.
Herunterladen und starten.

**Keine Werbung. Gar keine.**
Keine Banner, keine Pop-ups, keine „Jetzt upgraden“-Hinweise. Es gibt auch keine Bezahlversion.

**Es werden keine persönlichen Daten erfasst.**
Keine Registrierung, keine Anmeldung. Nichts über Ihre Nutzung wird irgendwohin gesendet.
Einstellungen und Download-Liste bleiben auf Ihrem eigenen PC.

**Keine mitgelieferte Zusatzsoftware.**
Die Installation schiebt keine anderen Programme unter und verändert keine Browsereinstellungen.

---

## Was es kann

- **Link einfügen** drücken und der Download startet (`Strg+V` geht auch)
- **Video** (MP4, MKV) oder **Musik** (MP3, M4A) wählen
- Qualität von **bestmöglich** bis 144p, Ton von bestmöglich bis 128 kbit/s
- **Ganze Playlists oder Kanäle** laden oder nur ausgewählte Videos ankreuzen
- **Untertitel und Tonspuren** wählen – rund 85 Sprachen, jeweils in eigener Schrift
- 1 bis 8 Downloads gleichzeitig
- **Helles und dunkles Design**, oder nach Windows-Einstellung
- In **15 Sprachen** verfügbar
- Vorschaubilder, Fortschritt, Restzeit
- Datei abspielen oder Ordner öffnen
- Ton aus gespeichertem Video extrahieren
- Behält einen Download-Verlauf

---

## Installation

1. Öffnen Sie oben **[Herunterladen](../download/)** und holen Sie `YouTubeToPC-Setup-1.2.0.exe`.
2. Starten Sie die Datei.
3. Sprache und Zielordner wählen, dann **Installieren** drücken.

Für die Installation wird eine Internetverbindung benötigt: Sie lädt die verwendeten
Werkzeuge (yt-dlp und FFmpeg) herunter und **bewahrt eigene Kopien im Programmordner
auf**. Das dauert meist 10 bis 30 Sekunden.

> **Wenn „Der Computer wurde durch Windows geschützt“ erscheint**
> Klicken Sie auf **Weitere Informationen → Trotzdem ausführen**.
> Der Hinweis erscheint, weil das Programm von einer Einzelperson stammt und kein
> kostenpflichtiges Signaturzertifikat besitzt. Es ist kein Virus.

Zum Entfernen nutzen Sie **Einstellungen → Apps** oder starten `Uninstall.exe` im
Installationsordner. Ihre heruntergeladenen Videos und Musikdateien bleiben erhalten.

---

## Bedienung

1. Kopieren Sie bei YouTube die Adresse eines Videos.
2. Drücken Sie im Programm auf **Link einfügen**.
3. Fertig. Es erscheint in der Liste und der Download beginnt.

Wenn Sie Qualität oder Untertitel je Video wählen möchten, schalten Sie **Sofort laden** aus.
Dann öffnet sich bei jedem Link ein Auswahlfenster.

Fügen Sie die Adresse einer Playlist oder eines Kanals ein, erhalten Sie die vollständige
Videoliste mit Kästchen, um nur das Gewünschte auszuwählen.

---

## Fragen

**Downloads funktionieren plötzlich nicht mehr.**
Wenn sich YouTube ändert, muss sich das Download-Werkzeug mitändern.
Das Programm prüft einmal täglich, Sie können aber jederzeit
**Extras → yt-dlp aktualisieren** ausführen.

**Im Firmennetz geht es nicht.**
Tragen Sie unter **Einstellungen → Verbindung** eine Proxy-Adresse ein.

**Altersbeschränkte Videos schlagen fehl.**
Wählen Sie unter **Einstellungen → Anmeldung** einen Browser, in dem Sie bei YouTube angemeldet sind.

---

## Selbst kompilieren

```
build.bat
```

Unter Windows genügt das UCRT64-g++ aus [MSYS2](https://www.msys2.org).
Kein Visual Studio, kein .NET SDK.

---

## Verwendete Werkzeuge

| Name | Zweck | Lizenz |
|---|---|---|
| [yt-dlp](https://github.com/yt-dlp/yt-dlp) | Herunterladen von YouTube | Unlicense |
| [FFmpeg](https://ffmpeg.org) | Video und Ton zusammenführen, umwandeln | GPLv3 |
| [QuickJS-ng](https://github.com/quickjs-ng/quickjs) | YouTubes JavaScript-Prüfungen ausführen | MIT |

Die Installation lädt jeweils den offiziellen Build des Projekts.
Die vollständigen Hinweise stehen in [THIRD-PARTY-NOTICES](../licenses/THIRD-PARTY-NOTICES.txt).

---

## Bitte beachten

Laden Sie nur Inhalte herunter, zu deren Speicherung Sie berechtigt sind.
Dieses Werkzeug ist für eigene Inhalte, für Inhalte mit Erlaubnis oder für
gemeinfreie Inhalte gedacht. Die Verantwortung für die Nutzung liegt bei Ihnen.

---

<div align="center">

Vor ein paar Tagen habe ich gekündigt. Aus Langeweile zu Hause ist das hier entstanden.<br>
Mein Traum: freiberuflich arbeiten, aus einem kleinen Zimmer in Mukdong, Seoul in Korea.
31. Oktober 2026.

**Bruce Jung** · [sunghunet@gmail.com](mailto:sunghunet@gmail.com)

</div>
