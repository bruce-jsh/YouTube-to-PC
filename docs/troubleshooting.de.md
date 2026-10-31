<div align="center">

<img src="../res/logo.png" width="72" alt="YouTube to PC">

# Wenn Windows den Download oder die Installation verhindert

</div>

---

**Keine dieser Warnungen bedeutet, dass das Programm gefährlich ist.**

Windows vertraut Software nach ihrem Ruf, und dieser Ruf wird mit einem
Signaturzertifikat gekauft. Dieses Programm schreibt eine einzelne Person und hat
noch keines, also behandelt Windows es als Fremden und sagt das sehr deutlich.

Suchen Sie unten Ihre Meldung. Die meisten sind mit drei Klicks erledigt.

---

## Am schnellsten: einen anderen Browser nehmen

Streng ist Microsoft Edge. **Chrome und Firefox laden die Datei kommentarlos
herunter.**

Wenn Edge Ihnen Ärger macht, öffnen Sie die
[Download-Seite](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest) in
Chrome oder Firefox und drücken dort die Schaltfläche. Das ist alles.

Wenn Sie bei Edge bleiben möchten, lesen Sie weiter.

---

## 1. Edge blockiert den Download

Ein Hinweis erscheint, die Datei werde **nicht häufig heruntergeladen**.

<img src="images/block-1-notice.png" width="380" alt="Edge meldet, die Datei werde nicht häufig heruntergeladen">

**Schritt 1.** Drücken Sie `Strg` + `J`, um **Downloads** zu öffnen, und dann die
Schaltfläche **…** in dieser Zeile.

<img src="images/block-2-menu.png" width="520" alt="Das Drei-Punkte-Menü mit Beibehalten und Datei als sicher melden">

**Schritt 2.** Wählen Sie **Beibehalten** (Keep).

**Schritt 3.** Ein zweites Fenster erscheint. Hier bleiben die meisten hängen: die
einzige sichtbare Schaltfläche heißt **Löschen** (Delete). Drücken Sie den
**kleinen Pfeil neben Löschen** und wählen Sie **Trotzdem beibehalten**
(Keep anyway).

<img src="images/block-3-keep.png" width="370" alt="Der Pfeil neben Löschen bringt Trotzdem beibehalten zum Vorschein">

Danach läuft der Download normal durch.

> Wenn Sie schon im Menü sind: **Datei als sicher melden** (Report this file as
> safe) lohnt sich. Je mehr Leute das tun, desto eher verschwindet diese Warnung
> für alle.

---

## 2. "Der Computer wurde durch Windows geschützt" beim Start

Es kann ein blaues Fenster erscheinen, das nur die Schaltfläche
**Nicht ausführen** anbietet.

1. Klicken Sie auf den kleinen Link **Weitere Informationen** unter dem Text. Er
   ist einfacher Text, keine Schaltfläche, und wird leicht übersehen
2. Unten erscheint eine neue Schaltfläche: **Trotzdem ausführen**
3. Drücken Sie sie

Dieses Fenster erscheint meist beim ersten Start auf einem PC. Wenn Sie es einmal
erlaubt haben, fragt Windows auf diesem PC nicht wieder.

---

## 3. Das Antivirenprogramm hat die Datei gelöscht

Der Download ist fertig und die Datei ist verschwunden.

1. Öffnen Sie **Windows-Sicherheit** über das Startmenü
2. Gehen Sie zu **Viren- und Bedrohungsschutz**
3. Suchen Sie unter **Schutzverlauf** den Eintrag `YouTubeToPC-Setup`
4. Öffnen Sie ihn, drücken Sie **Aktionen** und wählen Sie **Auf Gerät zulassen**
5. Laden Sie die Datei erneut herunter

Bei anderen Programmen (Avast, AVG, Norton, G DATA) tragen Sie eine Ausnahme für
die Setup-Datei oder den Installationsordner ein:

```
C:\Program Files\YouTube to PC
```

Oder halten Sie den Echtzeitschutz kurz an, installieren und schalten ihn wieder
ein.

---

## 4. Das Setup startet und bleibt stehen

Meldungen über yt-dlp oder über die Internetverbindung bedeuten, dass das Setup die
benötigten Werkzeuge nicht laden konnte.

Das Setup ist nur rund 1 MB groß. Während der Installation lädt es yt-dlp, FFmpeg
und QuickJS von deren offiziellen Seiten, zusammen etwa 110 MB, und legt eigene
Kopien im Programmordner ab.

Versuchen Sie es in dieser Reihenfolge.

**a. VPN ausschalten und das Setup erneut starten.** Das ist die häufigste
Ursache.

**b. Im Firewall freigeben.** **Windows-Sicherheit** →
**Firewall- und Netzwerkschutz** → **App durch Firewall zulassen** →
**Einstellungen ändern** → **Andere App zulassen** → **Durchsuchen**, die
Setup-Datei wählen, **Hinzufügen**, und sowohl **Privat** als auch **Öffentlich**
ankreuzen.

**c. Ein anderes Netz probieren.** Firmen-, Schul- und manche öffentlichen Netze
sperren GitHub, und von dort kommen die Werkzeuge. Ein Anschluss zu Hause oder ein
Handy-Hotspot funktioniert meist.

---

## 5. Es passiert überhaupt nichts

- **Doppelklick bewirkt nichts.** Rechtsklick auf die Datei und
  **Als Administrator ausführen** wählen.
- **"Diese App kann auf dem PC nicht ausgeführt werden".** Sie brauchen Windows 10
  oder 11 in 64 Bit. Nachsehen unter **Einstellungen**, **System**, **Info**.
- **Die Datei hat 0 Byte oder lässt sich nicht öffnen.** Der Download wurde
  abgeschnitten. Löschen und neu laden.

---

## Prüfen, ob die Datei echt ist

Jede Release-Seite nennt den SHA-256 des Setups. Öffnen Sie PowerShell in dem
Ordner mit der Datei und führen Sie aus:

```powershell
Get-FileHash .\YouTubeToPC-Setup-1.3.1.exe -Algorithm SHA256
```

Stimmt der Wert mit dem auf der
[Release-Seite](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest)
überein, haben Sie genau die veröffentlichte Datei und niemand hat sie verändert.

---

## Warum das überhaupt passiert

Ein Signaturzertifikat kostet jedes Jahr Geld, und ohne eines hat Windows keine
Möglichkeit zu erkennen, wer dieses Programm geschrieben hat.

Dafür ist der Quelltext offen, jede Version wird öffentlich von
[GitHub Actions](https://github.com/bruce-jsh/YouTube-to-PC/actions) gebaut, und
mit dem Hash oben können Sie bestätigen, dass Sie genau diesen Build erhalten
haben.

An der Signatur wird gearbeitet. Sobald sie da ist, verschwinden diese Warnungen.

---

## Immer noch festgefahren

Schreiben Sie an **[sunghunet@gmail.com](mailto:sunghunet@gmail.com)** oder
eröffnen Sie ein [Issue](https://github.com/bruce-jsh/YouTube-to-PC/issues). Sagen
Sie mir, was die Meldung sagte und bei welchem Schritt sie kam. Ein Bildschirmfoto
hilft sehr.

<div align="center">

[Zurück zur Hauptseite](README.de.md)

</div>
