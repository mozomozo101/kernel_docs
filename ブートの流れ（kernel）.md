# kernelの展開
u-bootのbootmコマンドは、zImageのエントリーポイントを実行することがわかった。  
じゃあ、そのエントリーポイントはどこだろうか？  

**arch/arm/boot/compressed/vmlinux.lds.S**
```
...
ENTRY(_start)
SECTIONS
{
  ...
  .text : {
    _start = .;
    *(.start)
    *(.text)
    ...
  }
```
_start すなわち *(.start) セクションがエントリーポイントっぽい。  
で、startセクションの内容を探すと、ここにあった。

**arch/arm/boot/compressed/vmlinux.lds.S**
```
...
// 以降、startセクションに配置
.section ".start", #alloc, #execinstr
...
start:
```



zImageには、にあるような自己解凍機能が付いてる。
これを使ってkernelを解凍し、エントリーポイント（0xC008000など）に処理を移す・・・んだと思う。

