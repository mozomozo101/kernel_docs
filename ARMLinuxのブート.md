# ブートローダ概要
ここの翻訳、まとめ。  
http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html#ATAG_MEM  

## ブートローダの役割
ARM Linuxにおいて、ブートローダが行う必要のある処理は、次の通り。
* メモリの設定
* カーネルを正しいメモリアドレスにロード
* （オプション）initramfsを正しいアドレスにロード
* コンソールの初期化
* カーネルパラメータのセット
* マシンタイプの取得
* レジスタに適切な値を設定し、カーネルに処理を渡す

### メモリの設定
ATAGS内で、ATAG_MEMを使い、カーネルが使用する物理メモリの範囲を指定。

### カーネルのロード
ビルドされた生バイナリであるImageファイルより、圧縮されたzImageを使うのが一般的。
zImageにはヘッダがあり、次のものが含まれる。  

| Offset | Value | Description |
| ---- | ---- | ---- |
| 0x24 | 0x016F2818 | Magic number used to identify this is an ARM Linux zImage |
| 0x28 | start address | The address the zImage starts at |
| 0x2C | end address | The address the zImage ends at |

zImageのロード位置はどこでも良いが、習慣的に、物理RAMの先頭から0x8000（32KB）の位置が使われる。
これは、メモリの先頭あたりに置かれているであろう諸々のデータを壊さないようにするため。

### initial RAMのロード

### コンソール初期化
シリアルポートを初期化する。
ここで初期化されたシリアルポートは、カーネル起動後、シリアルドライバが検知し、カーネルが使用できるようになる。
なお、"console="オプションを使えば、カーネル側に明示的にシリアルポートを指定することもできる。

### カーネルパラメータ
ブートローダは、ATAGSに自身がセットアップした内容を含めて、カーネルに渡す。
ATAGSは、上書きされないよう、RAMの先頭から0x100に配置するのが一般的。
カーネル起動時に、ATAGSのアドレスをR2レジスタに入れる
カーネルは、ATAGSを探す際、デフォルトでRAMの先頭から0x100の位置を見に行くので、必須ではない。

ATAGSに含まれるタグには、例えば以下のものがある。
詳しくは、参照元URLを見ること。
* カーネルパラメータ
* カーネルが使用するメモリ領域
* initramfsの位置

![atags](https://github.com/mozomozo101/kernel_docs/blob/edit/images/IMG_1141.jpg)

### マシンタイプ
カーネルに対して、どんなマシン上で動いているかを知らせる必要がある。
これを表すマシンIDのは、[ARM Linuxのサイト](http://www.arm.linux.org.uk/developer/machines/)に書かれている。
ブートローダはこのマシンIDを取得し、カーネルに伝える必要があるが、その方法はシステム依存なので、このドキュメントの範囲外。

### カーネルの起動
カーネルに処理を渡す際、CPUのレジスタやMMU、キャッシュ等が適切に設定されている必要がある。
詳細は参照元URL。


# u-bootを使ったカーネルブート
http://fowlrat.hatenablog.com/entry/2014/08/31/011056

[カーネルのロード](#カーネルのロード)に書いたとおり、zImageは、RAMの先頭から0x8000の位置に展開する。
ただし、u-boot専用のイメージフォーマットであるuImageには、zImageの前に64byte(0x40)のヘッダが付いている。
そのため、uImiageのロードアドレスは、(RAMの先頭 + 0x8000 - 0x40)となる。

```
=> usb start
=> fatload usb 0 0x0C007FC0 /uImage
=> bootm 0x0C007FC0
```
![u-boot boot](https://github.com/mozomozo101/kernel_docs/blob/edit/images/IMG_1142.jpg)
