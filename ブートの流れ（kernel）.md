# kernelの展開
u-bootのbootmコマンドは、zImageのエントリーポイントを実行することがわかった。  
じゃあ、そのエントリーポイントはどこだろうか？ 

[このページ](https://www.aandd.co.jp/dvhome/linuxsh/doc/linux-sh-kernel-bootup_J.txt)によると、zImageはこのような構成になってるようだ。  
(arch/sh 向けのものだけど、きっとARMでも、根拠はないけど大体同じはず。）
```
+--------------------------------+
| arch/sh/boot/compressed/head.S |
+--------------------------------+
| arch/sh/boot/compressed/misc.c |
+--------------------------------+
| arch/sh/kernel/sh_bios.c       |
+--------------------------------+
| gzip圧縮されたvmlinux          |
| +----------------------------+ |
| | arch/sh/kernel/head.S      | |
| +----------------------------+ |
| | arch/sh/kernel/init_task.c | |
| +----------------------------+ |
| | init/main.c                | |
| +----------------------------+ |
| | init/version.c             | |
| +----------------------------+ |
| | ......                     | |
| +----------------------------+ |
+--------------------------------+
```




**arch/arm/boot/compressed/vmlinux.lds.S**
```
ENTRY(_start)       // エントリーポイントは _start というシンボル
SECTIONS
{
  . = TEXT_START;
  _text = .;     
  
  .text : {
    _start = .;     // _start はここ
    *(.start)       // xxxx.start というセクションの先頭がエントリーポイントになる。
    *(.text)
    ...
  }
```
じゃあ xxx.start というセクションはどこにあるかというと・・・

**arch/arm/boot/compressed/head.S**
```
.section ".start", #alloc, #execinstr     // .startセクション開始

start:
  mov r7, r1          // マシンIDの取得
  mov r8, r2          // ATAGSのポインタ取得

  #ifdef CONFIG_AUTO_ZRELADDR
	  @ determine final kernel image address
	  mov r4, pc
	  and r4, r4, #0xf8000000
	  add r4, r4, #TEXT_OFFSET
  #else
  	ldr r4, =zreladdr
  #endif
```

[ZRELADDR]([https://cateee.net/lkddb/web-lkddb/AUTO_ZRELADDR.html)は、zImageから抽出されたカーネルイメージが配置され物理アドレスを表す。
TEXT_OFFSET は zImage の配置場所で、デフォルトでは RAMの先頭 0x8000（32KB）の位置。



zImageには、にあるような自己解凍機能が付いてる。
これを使ってkernelを解凍し、エントリーポイント（0xC008000など）に処理を移す・・・んだと思う。

https://www.aandd.co.jp/dvhome/linuxsh/doc/linux-sh-kernel-bootup_J.txt  
https://www.kernel.org/doc/Documentation/arm/Porting
