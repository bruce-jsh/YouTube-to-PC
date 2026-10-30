<div align="center">

<img src="../res/app.ico" width="96" alt="YouTube to PC">

# YouTube to PC

**La forma más sencilla de guardar vídeos y música de YouTube en tu PC**

Copia un enlace y pulsa un botón. Eso es todo.

<br>

### [⬇️ Descargar: ir al instalador](../download/)

[**Obtener YouTubeToPC-Setup-1.2.0.exe**](https://github.com/bruce-jsh/YouTube-to-PC/raw/main/download/YouTubeToPC-Setup-1.2.0.exe) · 1,2 MB · Windows 10/11 (64 bits)

<br>

[한국어](README.ko.md) ·
[Türkçe](README.tr.md) ·
[Монгол](README.mn.md) ·
[English](../README.md) ·
[简体中文](README.zh-Hans.md) ·
[繁體中文](README.zh-Hant.md) ·
**Español** ·
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

## Puedes usarlo con tranquilidad

**No hay que instalar nada más.**
No necesitas .NET Framework, los redistribuibles de Visual C++, Python ni nada parecido.
Es un programa Win32 que solo usa lo que Windows ya trae. Descárgalo y ejecútalo.

**Sin publicidad. Ninguna.**
Ni banners, ni ventanas emergentes, ni avisos de «mejora ahora». Tampoco hay versión de pago.

**No recoge datos personales.**
Sin registro ni inicio de sesión. No envía nada sobre tu uso a ninguna parte.
Tus ajustes y tu lista de descargas se quedan en tu propio PC.

**Sin programas añadidos.**
La instalación no cuela otro software ni toca la configuración de tu navegador.

---

## Qué hace

- Pulsa **Pegar enlace** y la descarga empieza (también funciona `Ctrl+V`)
- Elige **vídeo** (MP4, MKV) o **música** (MP3, M4A)
- Calidad desde **la mejor disponible** hasta 144p; audio desde la mejor hasta 128 kbps
- Descarga **listas o canales enteros**, o marca solo los vídeos que quieras
- Elige **subtítulos y pistas de audio**: unos 85 idiomas, cada uno en su propia escritura
- De 1 a 8 descargas a la vez
- **Tema claro y oscuro**, o según la configuración de Windows
- Disponible en **15 idiomas**
- Miniaturas, progreso y tiempo restante
- Reproduce el archivo o abre su carpeta
- Extrae el audio de un vídeo guardado
- Guarda un historial de descargas

---

## Instalación

1. Entra en **[Descargar](../download/)** y consigue `YouTubeToPC-Setup-1.2.0.exe`.
2. Ejecútalo.
3. Elige idioma y carpeta de instalación, y pulsa **Instalar**.

La instalación necesita conexión a internet: descarga las herramientas que usa el
programa (yt-dlp y FFmpeg) y **guarda sus propias copias dentro de la carpeta del
programa**. Suele tardar entre 10 y 30 segundos.

> **Si aparece «Windows protegió su PC»**
> Pulsa **Más información → Ejecutar de todas formas**.
> Ese aviso sale porque lo ha hecho una sola persona y no tiene un certificado
> de firma de código de pago. No es un virus.

Para desinstalarlo usa **Configuración → Aplicaciones**, o ejecuta `Uninstall.exe`
en la carpeta de instalación. Tus vídeos y tu música descargados nunca se borran.

---

## Cómo se usa

1. Copia la dirección de un vídeo en YouTube.
2. Pulsa **Pegar enlace** en el programa.
3. Listo. Aparece en la lista y empieza a descargarse.

Si quieres elegir calidad o subtítulos en cada vídeo, desactiva **Al instante**.
Entonces se abrirá una ventana de opciones cada vez que añadas un enlace.

Si pegas la dirección de una lista o un canal, verás todos los vídeos con casillas
para marcar solo los que te interesen.

---

## Preguntas

**De repente ya no descarga.**
Cuando YouTube cambia, la herramienta de descarga tiene que cambiar también.
El programa busca una versión nueva una vez al día, pero puedes ejecutar
**Herramientas → Actualizar yt-dlp** cuando quieras.

**No funciona en la red de mi empresa.**
Prueba a poner una dirección de proxy en **Configuración → Conexión**.

**Fallan los vídeos con restricción de edad.**
En **Configuración → Inicio de sesión**, elige un navegador donde hayas entrado en YouTube.

---

## Compilarlo tú mismo

```
build.bat
```

En Windows solo hace falta el g++ de UCRT64 de [MSYS2](https://www.msys2.org).
Ni Visual Studio ni .NET SDK.

---

## Herramientas que utiliza

| Nombre | Para qué | Licencia |
|---|---|---|
| [yt-dlp](https://github.com/yt-dlp/yt-dlp) | Descargar de YouTube | Unlicense |
| [FFmpeg](https://ffmpeg.org) | Unir vídeo y audio, convertir | GPLv3 |
| [QuickJS-ng](https://github.com/quickjs-ng/quickjs) | Ejecutar las comprobaciones JavaScript de YouTube | MIT |

La instalación descarga la versión oficial de cada proyecto.
Los avisos completos están en [THIRD-PARTY-NOTICES](../licenses/THIRD-PARTY-NOTICES.txt).

---

## Ten en cuenta

Descarga solo contenido que tengas derecho a guardar.
Esta herramienta está pensada para contenido propio, con permiso o libre de derechos.
El uso que hagas de ella es tu responsabilidad.

---

<div align="center">

Hace unos días dejé mi trabajo. Aburrido en casa, hice esto.<br>
Sueño con ser autónomo, desde un cuarto en Mukdong, Seúl (Corea).
31 de octubre de 2026.

**Bruce Jung** · [sunghunet@gmail.com](mailto:sunghunet@gmail.com)

</div>
