# アーキテクチャごとの初期化処理

[Linux Inside 第6章　Architecture-specific initializations, again...](http://www.alfonsoesparza.buap.mx/sites/default/files/linux-insides.pdf#%5B%7B%22num%22%3A255%2C%22gen%22%3A0%7D%2C%7B%22name%22%3A%22XYZ%22%7D%2C0%2C841.8898%2Cnull%5D)
を参照。  

カーネルコマンドラインをパースし、その内容に応じた処理を行うことができる。
設定可能なカーネルコマンドラインのパラメータは、[ここ](https://www.kernel.org/doc/Documentation/admin-guide/kernel-parameters.txt)に書かれてる。


## パラメータに応じた処理の登録
どのパラメータを受け取った場合にどんな処理を行うかを、あらかじめカーネルコード内に記述しておく必要がある。  
これは、[early_param](https://elixir.bootlin.com/linux/latest/source/include/linux/init.h#L268) マクロを使って行う。  
このマクロは、以下のように展開されていく。
```
// xxx: コマンドラインのパラメタ名
// funcx: xxxがパラメタとして与えられた場合に呼ばれる関数。

early_param ("xxx", funcx) 
↓
__stup_param("xxx", funcx, funcx, 1)    // 第4引数は、0: not early, 1: early
↓
char __setup_str_funcx = "xxx";

static obs_kernel_param __setup_funcx {
		__setup_str_funcx,  	// カーネルパラメータ(つまり、"xxx")
		,funcx			// セットされる関数			
		1 			// 0:not early, 1:early
}

```

つまり、[obs_kernel_param](https://elixir.bootlin.com/linux/latest/source/include/linux/init.h#L241)
構造体により、early処理対象となるブートパラメータと、それがセットされた場合に実行される関数が紐づけられている。
earlyprintk の場合、[このように](https://elixir.bootlin.com/linux/v4.20-rc5/source/arch/arm/kernel/early_printk.c#L50)
記述されている。
```
static int __init setup_early_printk(char *buf)
{
	early_console = &early_console_dev;
	register_console(&early_console_dev);
	return 0;
}

early_param ("earlyprintk", setup_early_printk)
```

## earlyな処理の実行
early_param() で登録されたearlyな関数を実行するには、parse_early_param() を呼ぶ。  
すると、ブート時のコマンドラインをパースした上で、下記のように複数の関数を経て、最終的に
[do_early_param()](https://github.com/torvalds/linux/blob/16f73eb02d7e1765ccab3d2018e0bd98eb93d973/init/main.c#L440)
が呼ばれる。  
```
parse_early_param()
  -> parse_early_options(cmdline)
    -> parse_args(cmdline)
      -> do_early_param    // パラメータがearlyなら、obs_kernel_param に設定された関数を呼ぶ
```
この関数は、見ての通り、各struct obs_kernel_param を順に見ていき、earlyフラグが立っていれば、紐付けられた関数を呼ぶ。


## 補足 
struct obs_kernel_param は、下記の定義の通り、.init.setup セクションに配置される。 
```
#define __setup_param(str, unique_id, fn, early)                \
        static const char __setup_str_##unique_id[] __initconst \
                __aligned(1) = str; \
        static struct obs_kernel_param __setup_##unique_id      \
                __used __section(.init.setup)                   \
                __attribute__((aligned((sizeof(long)))))        \
                = { __setup_str_##unique_id, fn, early }
```
.init.setupセクションは、
[include/asm-generic/vmlinux.lds.h](https://github.com/torvalds/linux/blob/16f73eb02d7e1765ccab3d2018e0bd98eb93d973/include/asm-generic/vmlinux.lds.h#L702)
において設定されている。
```
#define INIT_SETUP(initsetup_align)                \
                . = ALIGN(initsetup_align);        \
                VMLINUX_SYMBOL(__setup_start) = .; \
                *(.init.setup)                     \
                VMLINUX_SYMBOL(__setup_end) = .;
```
