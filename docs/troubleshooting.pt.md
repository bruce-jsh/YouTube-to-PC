<div align="center">

<img src="../res/logo.png" width="72" alt="YouTube to PC">

# Se o Windows não deixa baixar ou instalar

</div>

---

**Nenhum destes avisos significa que o programa seja perigoso.**

O Windows confia em programas pela reputação, e essa reputação se compra com um
certificado de assinatura. Este programa é escrito por uma pessoa só e ainda não
tem um, então o Windows o trata como estranho e avisa em alto e bom som.

Procure o seu aviso abaixo. A maioria se resolve em três cliques.

---

## O mais rápido: use outro navegador

O rigoroso é o Microsoft Edge. **Chrome e Firefox baixam este arquivo sem dizer
nada.**

Se o Edge estiver atrapalhando, abra a
[página de download](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest)
no Chrome ou no Firefox e clique no botão por lá. É só isso.

Se preferir continuar no Edge, siga lendo.

---

## 1. O Edge bloqueia o download

Aparece um aviso dizendo que o arquivo **não é baixado com frequência**.

<img src="images/block-1-notice.png" width="380" alt="O Edge avisa que o arquivo não é baixado com frequência">

**Passo 1.** Pressione `Ctrl` + `J` para abrir **Downloads** e clique no botão
**…** daquela linha.

<img src="images/block-2-menu.png" width="520" alt="O menu de três pontos com Manter e Relatar que o arquivo é seguro">

**Passo 2.** Escolha **Manter** (Keep).

**Passo 3.** Abre-se uma segunda janela. É aqui que as pessoas travam: o único
botão visível diz **Excluir** (Delete). Clique na **setinha ao lado de Excluir** e
escolha **Manter mesmo assim** (Keep anyway).

<img src="images/block-3-keep.png" width="370" alt="Clique na seta ao lado de Excluir para revelar Manter mesmo assim">

O download termina normalmente.

> Já que está nesse menu, vale clicar em **Relatar que o arquivo é seguro** (Report
> this file as safe). Quanto mais gente fizer isso, mais cedo esse aviso some para
> todo mundo.

---

## 2. "O Windows protegeu o seu PC" ao executar

Pode aparecer uma janela azul dizendo que o Windows protegeu o seu PC, oferecendo
apenas o botão **Não executar**.

1. Clique no pequeno link **Mais informações** abaixo da mensagem. É texto simples,
   não um botão, e passa despercebido com facilidade
2. Um botão novo aparece embaixo: **Executar assim mesmo**
3. Clique nele

Essa janela costuma aparecer na primeira vez que você executa o arquivo num PC.
Depois de permitir uma vez, o Windows não pergunta de novo nesse PC.

---

## 3. O antivírus apagou o arquivo

O download termina e o arquivo sumiu.

1. Abra a **Segurança do Windows** pelo menu Iniciar
2. Vá em **Proteção contra vírus e ameaças**
3. Em **Histórico de proteção**, encontre a entrada `YouTubeToPC-Setup`
4. Abra, clique em **Ações** e escolha **Permitir no dispositivo**
5. Baixe o arquivo de novo

Com outros antivírus (Avast, AVG, Norton, McAfee), adicione uma exceção para o
instalador ou para a pasta de instalação:

```
C:\Program Files\YouTube to PC
```

Ou pause a proteção em tempo real por um minuto, instale e ligue de novo.

---

## 4. A instalação começa e para no meio

Mensagens sobre o yt-dlp ou sobre verificar a conexão querem dizer que o
instalador não conseguiu baixar as ferramentas de que precisa.

O instalador tem só cerca de 1 MB. Durante a instalação ele baixa yt-dlp, FFmpeg e
QuickJS das páginas oficiais, uns 110 MB no total, e guarda cópias próprias dentro
da pasta do programa.

Tente nesta ordem.

**a. Desligue a VPN e rode o instalador outra vez.** É a causa mais comum.

**b. Libere no firewall.** **Segurança do Windows** →
**Firewall e proteção de rede** → **Permitir um aplicativo pelo firewall** →
**Alterar configurações** → **Permitir outro aplicativo** → **Procurar**, escolha o
instalador, **Adicionar**, e marque **Particular** e **Pública**.

**c. Tente outra rede.** Redes de empresa, de escola e algumas públicas bloqueiam o
GitHub, que é de onde vêm as ferramentas. Uma conexão de casa ou o roteamento do
celular costuma funcionar.

---

## 5. Não acontece nada

- **Dois cliques e nada.** Clique com o botão direito no arquivo e escolha
  **Executar como administrador**.
- **"Este aplicativo não pode ser executado no seu PC".** É preciso Windows 10 ou
  11 de 64 bits. Veja em **Configurações**, **Sistema**, **Sobre**.
- **O arquivo tem 0 bytes ou não abre.** O download foi interrompido. Apague e
  baixe de novo.

---

## Conferir se o arquivo é autêntico

Toda página de versão mostra o SHA-256 do instalador. Abra o PowerShell na pasta do
arquivo e rode:

```powershell
Get-FileHash .\YouTubeToPC-Setup-1.3.3.exe -Algorithm SHA256
```

Se bater com o valor da
[página da versão](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest),
você tem exatamente o arquivo publicado e ninguém mexeu nele.

---

## Por que isso acontece

Um certificado de assinatura custa dinheiro todo ano e, sem ele, o Windows não tem
como saber quem escreveu este programa.

Em troca, o código é aberto, cada versão é compilada em público pelo
[GitHub Actions](https://github.com/bruce-jsh/YouTube-to-PC/actions), e o hash
acima permite confirmar que você recebeu exatamente aquela compilação.

O trabalho para assinar o programa está em andamento. Quando terminar, estes avisos
somem.

---

## Se ainda assim não der

Escreva para **[sunghunet@gmail.com](mailto:sunghunet@gmail.com)** ou abra uma
[issue](https://github.com/bruce-jsh/YouTube-to-PC/issues). Conte qual era a
mensagem e em que passo apareceu. Uma captura de tela ajuda bastante.

<div align="center">

[Voltar à página principal](README.pt.md)

</div>
