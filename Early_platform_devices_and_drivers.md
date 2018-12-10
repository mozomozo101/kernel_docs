この2つをまとめたもの。  
http://www.alfonsoesparza.buap.mx/sites/default/files/linux-insides.pdf   
https://www.kernel.org/doc/Documentation/driver-model/platform.txt  

# Early Platform Device and Drivers
Early platform interface を使うと、システムの起動初期段階で、platform device driver にplatform data を渡すことができる。

### early platform device データの登録  
platform deviceは
[early_platform_add_devices()](https://elixir.bootlin.com/linux/latest/source/drivers/base/platform.c#L1271) 
を使って、当該デバイスをearly_platform_device_listというearly platform device のリストに追加する。

### カーネルコマンドラインのパース  

#### earlyな処理の登録
earlyな処理は、カーネル起動時に特定のパラメータを渡すことで実行される。  
そのため、あらかじめカーネルコード内で、どのパラメータを受け取った場合にどんな処理を行うかを記述しておく必要がある。  
これは、[early_param](https://elixir.bootlin.com/linux/latest/source/include/linux/init.h#L268) マクロを使って行う。  
このマクロは、以下の形をとる。
```
early_param ("xxx", funcx) 
```
* xxx: コマンドラインのパラメタ名
* funcx: xxxがパラメタとして与えられた場合に呼ばれる関数。

このマクロは、結果的に下記のように展開される。  
つまり、obs_kernel_param構造体により、early処理対象となるブートパラメータと、それがセットされた場合に実行される関数が紐づけられている。
```
char __setup_str_funcx = "xxx";

static obs_kernel_param __setup_funcx {
		__setup_str_funcx,  	// カーネルパラメータ
		,funcx			// セットされる関数			
		1 			// 0:not early, 1:early
}
```

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

#### earlyな処理の実行
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


#### 補足 
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


### ドライバの登録
early_platform_driver_register_all() を使うと、特定のクラスに属する全ての early platform driver を登録できるよ。
また、early_platform_init() を使って初期化したplatfor driever は、この時点で自動的に登録されてるよ。  


### early platform driver の probe
システムは、特定のクラスを持つ登録済みの early platform driver と 登録済みの early platform deviceを紐付けるため、 ealy_platform_driver_probe() を呼ぶ。
マッチすれば、ドライバがprobe()を呼ぶ。

### early platform driver の probe()内部のお話
起動初期の時点でメモリ割り当てや割り込み登録をする場合、ドライバ側は色々注意しないとダメ。
自分とマッチしたのが、early platform device なのか、それとも、普通の platform device なのか等。

early_param("gbpages", parse_direct_gbpages_on)
カーネルパラメータとして "gbpages" が渡された場合、parse_direct_gbpages_on が呼ばれる。


＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ー
# ここからは、未完成っぅｔぅっっ！！！！１１１！？？？

https://lwn.net/Articles/314900/

## early platform drivers V2
Platform Driver は、ドライバとその設定情報を分離できる。  
そのため、ドライバを様々なプラットフォーム上で動かせる。

しかし、early device については、そのような仕組みがない。  
というのも、earlyタイマやearlyシリアルポートなどのearly device は、platform driverのコアコードよりも前に必要となるからだ。
そのため、early driverが設定を取得するには、予めハードコードしておくか、独自の手段を実装するしかない。

このearly platform driver のパッチは、ドライバが自身をシステムに登録し、システムがそれをprobeするための関数セットだ。  
ドライバは、自身を`early_param()` で登録しておく。
するとシステムは、自分のタイミングでいつでもそれをprobeできる。

```
// ドライバ側
static int __init sh_cmt_init(void){
       return platform_driver_register(&sh_cmt_device_driver);
}

static void __exit sh_cmt_exit(void){
       platform_driver_unregister(&sh_cmt_device_driver);
}

early_platform_init("earlytimer", &sh_cmt_device_driver);
module_init(sh_cmt_init);
module_exit(sh_cmt_exit);

// システム側
void __init time_init(void)
{
    early_platform_driver_probe("earlytimer", sh_timer_pdevs,
                                   sh_timer_nr_pdevs, 1);
```







https://lwn.net/Articles/310854/

ここでの "early" とは、initcallよりも前という意味。  
コアデバイスドライバが early_param() を使うことで、early platform driverを取り込むことができる。  

```
/* early interface, used before platform devices are available */
static int __init my_earlytimer(char *buf) {
       return platform_driver_register_early(&my_platform_driver, buf);
}
early_param("earlytimer", my_earlytimer);
 ```
platform_driver_register_early() は、platform driver をリストに

