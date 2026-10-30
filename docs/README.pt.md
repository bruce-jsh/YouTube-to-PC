<div align="center">

<img src="../res/app.ico" width="96" alt="YouTube to PC">

# YouTube to PC

**A forma mais simples de guardar vídeos e músicas do YouTube no seu PC**

Copie um link, clique num botão. É só isso.

<br>

### [⬇️ Baixar: ir para o instalador](../download/)

[**Obter YouTubeToPC-Setup-1.2.0.exe**](https://github.com/bruce-jsh/YouTube-to-PC/raw/main/download/YouTubeToPC-Setup-1.2.0.exe) · 1,2 MB · Windows 10/11 (64 bits)

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
**Português** ·
[Русский](README.ru.md) ·
[Tiếng Việt](README.vi.md) ·
[ไทย](README.th.md) ·
[Bahasa Indonesia](README.id.md) ·
[日本語](README.ja.md)


</div>

---

## Pode usar sem preocupação

**Não precisa instalar mais nada.**
Nem .NET Framework, nem redistribuíveis do Visual C++, nem Python.
É um programa Win32 que usa apenas o que o Windows já tem. Baixe e execute.

**Sem anúncios. Nenhum.**
Sem banners, sem pop-ups, sem avisos de "faça o upgrade". Também não há versão paga.

**Não recolhe dados pessoais.**
Sem cadastro, sem login. Nada sobre o seu uso é enviado para lugar nenhum.
As configurações e a lista de downloads ficam apenas no seu PC.

**Sem programas embutidos.**
A instalação não enfia outros softwares nem mexe nas configurações do navegador.

---

## O que ele faz

- Clique em **Colar link** e o download começa (`Ctrl+V` também funciona)
- Escolha **vídeo** (MP4, MKV) ou **música** (MP3, M4A)
- Qualidade da **melhor disponível** até 144p; áudio da melhor até 128 kbps
- Baixe **playlists ou canais inteiros**, ou marque só os vídeos que quiser
- Escolha **legendas e faixas de áudio**: cerca de 85 idiomas, cada um na sua própria escrita
- De 1 a 8 downloads ao mesmo tempo
- **Tema claro e escuro**, ou conforme a configuração do Windows
- Disponível em **15 idiomas**
- Miniaturas, progresso e tempo restante
- Reproduz o arquivo ou abre a pasta
- Extrai o áudio de um vídeo salvo
- Mantém um histórico de downloads

---

## Instalação

1. Abra **[Baixar](../download/)** acima e pegue o `YouTubeToPC-Setup-1.2.0.exe`.
2. Execute o arquivo.
3. Escolha o idioma e a pasta de instalação e clique em **Instalar**.

A instalação precisa de internet: ela baixa as ferramentas que o programa usa
(yt-dlp e FFmpeg) e **guarda cópias próprias dentro da pasta do programa**.
Normalmente leva de 10 a 30 segundos.

> **Se aparecer "O Windows protegeu o seu PC"**
> Clique em **Mais informações → Executar assim mesmo**.
> Esse aviso surge porque o programa foi feito por uma pessoa só e não tem
> certificado pago de assinatura de código. Não é um vírus.

Para remover, use **Configurações → Aplicativos** ou execute o `Uninstall.exe`
na pasta de instalação. Os vídeos e músicas baixados nunca são apagados.

---

## Como usar

1. Copie o endereço de um vídeo no YouTube.
2. Clique em **Colar link** no programa.
3. Pronto. Ele aparece na lista e começa a baixar.

Se quiser escolher qualidade ou legendas em cada vídeo, desligue o **Instantâneo**.
Assim, uma janela de opções abre sempre que você adicionar um link.

Cole o endereço de uma playlist ou canal e você verá a lista completa de vídeos,
com caixas para marcar apenas o que interessa.

---

## Perguntas

**Os downloads pararam de funcionar de repente.**
Quando o YouTube muda, a ferramenta de download precisa mudar junto.
O programa verifica uma vez por dia, mas você pode executar
**Ferramentas → Atualizar o yt-dlp** quando quiser.

**Não funciona na rede da empresa.**
Tente informar um endereço de proxy em **Configurações → Conexão**.

**Vídeos com restrição de idade falham.**
Em **Configurações → Login**, escolha um navegador onde você esteja conectado ao YouTube.

---

## Compilar você mesmo

```
build.bat
```

No Windows basta o g++ do UCRT64 do [MSYS2](https://www.msys2.org).
Sem Visual Studio, sem .NET SDK.

---

## Ferramentas utilizadas

| Nome | Função | Licença |
|---|---|---|
| [yt-dlp](https://github.com/yt-dlp/yt-dlp) | Baixar do YouTube | Unlicense |
| [FFmpeg](https://ffmpeg.org) | Juntar vídeo e áudio, converter | GPLv3 |
| [QuickJS-ng](https://github.com/quickjs-ng/quickjs) | Executar as verificações JavaScript do YouTube | MIT |

A instalação baixa a versão oficial de cada projeto.
Os avisos completos estão em [THIRD-PARTY-NOTICES](../licenses/THIRD-PARTY-NOTICES.txt).

---

## Atenção

Baixe apenas conteúdo que você tem o direito de guardar.
Esta ferramenta serve para conteúdo próprio, autorizado ou livre de direitos autorais.
O uso é de sua responsabilidade.

---

<div align="center">

Pedi demissão há poucos dias. Entediado em casa, acabei fazendo isto.<br>
Sonho em ser freelancer, de um quartinho em Mukdong, Seul na Coreia.
31 de outubro de 2026.

**Bruce Jung** · [sunghunet@gmail.com](mailto:sunghunet@gmail.com)

</div>
