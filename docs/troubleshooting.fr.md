<div align="center">

<img src="../res/logo.png" width="72" alt="YouTube to PC">

# Si Windows refuse le téléchargement ou l'installation

</div>

---

**Aucun de ces avertissements ne signifie que le programme est dangereux.**

Windows accorde sa confiance aux logiciels selon leur réputation, et cette
réputation s'achète avec un certificat de signature. Ce programme est écrit par une
seule personne et n'en a pas encore, alors Windows le traite comme un inconnu et le
fait savoir bruyamment.

Trouvez votre avertissement ci-dessous. La plupart se règlent en trois clics.

---

## Le plus rapide : changer de navigateur

C'est Microsoft Edge qui est strict. **Chrome et Firefox téléchargent ce fichier
sans rien dire.**

Si Edge vous bloque, ouvrez la
[page de téléchargement](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest)
dans Chrome ou Firefox et appuyez sur le bouton. C'est tout.

Si vous préférez rester sur Edge, lisez la suite.

---

## 1. Edge bloque le téléchargement

Un message indique que le fichier **n'est pas téléchargé couramment**.

<img src="images/block-1-notice.png" width="380" alt="Edge indique que le fichier n'est pas téléchargé couramment">

**Étape 1.** Appuyez sur `Ctrl` + `J` pour ouvrir **Téléchargements**, puis sur le
bouton **…** de cette ligne.

<img src="images/block-2-menu.png" width="520" alt="Le menu à trois points avec Conserver et Signaler ce fichier comme sûr">

**Étape 2.** Choisissez **Conserver** (Keep).

**Étape 3.** Une seconde fenêtre apparaît. C'est là que tout le monde bute : le seul
bouton visible dit **Supprimer** (Delete). Appuyez sur la **petite flèche à côté de
Supprimer**, puis choisissez **Conserver quand même** (Keep anyway).

<img src="images/block-3-keep.png" width="370" alt="Appuyez sur la flèche à côté de Supprimer pour faire apparaître Conserver quand même">

Le téléchargement se termine alors normalement.

> Tant que vous y êtes, **Signaler ce fichier comme sûr** (Report this file as safe)
> vaut le coup. Plus il y a de signalements, plus vite cet avertissement disparaît
> pour tout le monde.

---

## 2. « Windows a protégé votre ordinateur » au lancement

Une fenêtre bleue peut apparaître, n'offrant que le bouton **Ne pas exécuter**.

1. Cliquez sur le petit lien **Informations complémentaires** sous le message.
   C'est du texte simple, pas un bouton, et il passe facilement inaperçu
2. Un nouveau bouton apparaît en bas : **Exécuter quand même**
3. Appuyez dessus

Cette fenêtre apparaît en général la première fois que vous lancez le fichier sur
un PC. Une fois que vous l'autorisez, Windows ne redemande plus sur ce PC.

---

## 3. L'antivirus a supprimé le fichier

Le téléchargement se termine et le fichier a disparu.

1. Ouvrez **Sécurité Windows** depuis le menu Démarrer
2. Allez dans **Protection contre les virus et menaces**
3. Dans **Historique de protection**, trouvez l'entrée `YouTubeToPC-Setup`
4. Ouvrez-la, appuyez sur **Actions** et choisissez **Autoriser sur l'appareil**
5. Téléchargez le fichier à nouveau

Avec un autre antivirus (Avast, AVG, Norton, McAfee), ajoutez une exception pour
l'installateur ou pour le dossier d'installation :

```
C:\Program Files\YouTube to PC
```

Ou mettez la protection en temps réel en pause une minute, installez, puis
réactivez-la.

---

## 4. L'installation démarre puis s'arrête

Les messages au sujet de yt-dlp ou de la connexion internet signifient que
l'installateur n'a pas pu récupérer les outils dont il a besoin.

L'installateur ne pèse qu'environ 1 Mo. Pendant l'installation, il télécharge
yt-dlp, FFmpeg et QuickJS depuis leurs pages officielles, environ 110 Mo au total,
et en garde ses propres copies dans le dossier du programme.

Essayez dans cet ordre.

**a. Coupez le VPN et relancez l'installation.** C'est la cause la plus fréquente.

**b. Autorisez-le dans le pare-feu.** **Sécurité Windows** →
**Pare-feu et protection du réseau** → **Autoriser une application via le pare-feu**
→ **Modifier les paramètres** → **Autoriser une autre application** → **Parcourir**,
choisissez l'installateur, **Ajouter**, et cochez **Privé** et **Public**.

**c. Essayez un autre réseau.** Les réseaux d'entreprise, d'école et certains
réseaux publics bloquent GitHub, d'où viennent les outils. Une connexion à domicile
ou un partage de connexion mobile fonctionne en général.

---

## 5. Rien ne se passe du tout

- **Un double-clic ne fait rien.** Clic droit sur le fichier, puis
  **Exécuter en tant qu'administrateur**.
- **« Cette application ne peut pas s'exécuter sur votre PC ».** Il faut Windows 10
  ou 11 en 64 bits. Vérifiez dans **Paramètres**, **Système**, **Informations
  système**.
- **Le fichier fait 0 octet ou ne s'ouvre pas.** Le téléchargement a été coupé.
  Supprimez-le et recommencez.

---

## Vérifier que le fichier est authentique

Chaque page de version indique le SHA-256 de l'installateur. Ouvrez PowerShell dans
le dossier contenant le fichier et lancez :

```powershell
Get-FileHash .\YouTubeToPC-Setup-1.3.2.exe -Algorithm SHA256
```

Si la valeur correspond à celle de la
[page de version](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest), vous
avez exactement le fichier publié et personne n'y a touché.

---

## Pourquoi cela arrive

Un certificat de signature coûte de l'argent chaque année et, sans lui, Windows n'a
aucun moyen de savoir qui a écrit ce programme.

En contrepartie, le code source est ouvert, chaque version est compilée en public
par [GitHub Actions](https://github.com/bruce-jsh/YouTube-to-PC/actions), et
l'empreinte ci-dessus permet de confirmer que vous avez bien reçu cette
compilation-là.

Des démarches sont en cours pour faire signer le programme. Une fois abouties, ces
avertissements disparaîtront.

---

## Toujours bloqué

Écrivez à **[sunghunet@gmail.com](mailto:sunghunet@gmail.com)** ou ouvrez un
[ticket](https://github.com/bruce-jsh/YouTube-to-PC/issues). Dites-moi ce que disait
le message et à quelle étape il est apparu. Une capture d'écran aide beaucoup.

<div align="center">

[Retour à la page principale](README.fr.md)

</div>
