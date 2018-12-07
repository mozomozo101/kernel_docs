この2つをまとめたもの。  
http://www.alfonsoesparza.buap.mx/sites/default/files/linux-insides.pdf 
https://www.kernel.org/doc/Documentation/driver-model/platform.txt  


## early platform device データの登録
platform deviceは、
[early_platform_add_devices()](https://elixir.bootlin.com/linux/latest/source/drivers/base/platform.c#L1271) 
を使って、当該デバイスを、early_platform_device_list というデバイスリストに追加する。

## カーネルコマンドラインのパース
early_param() は、early処理を行う関数や、それに対応するブートパラメータを登録しておくもののようだ。  
early_param ("xxx", funcx) であれば、  
* xxx: コマンドラインのパラメタ名
* funcx: xxxがパラメタとして与えられた場合に呼ばれる関数。
となる。

earlyprintkなら、マクロ展開の流れはこんな感じ。
ブートパラメータに"earlyprintk"があった場合、

```
early_param ("earlyprintk", setup_early_printk)
	↓
__setup_param("earlyprintk", setup_early_printk, setup_early_printk, 1)
	↓
char __setup_str_setup_early_printk = "earlyprintk";

static obs_kernel_param __setup_setup_early_printk {
		"__setup_str_setup_early_printk",  // カーネルパラメータ
		setup_early_printk,	              // セットされる関数
		1	                            // 0:not early, 1:early
}
```

で、earlyな処理として、コンソールデバイスを登録する処理を書いておく。
```
static int __init setup_early_printk(char *buf)
{
	early_console = &early_console_dev;
	register_console(&early_console_dev);
	return 0;
}
```

上記のように early_param() で登録された関数は、parse_early_param() によって、全て実行される。
ブート時のコマンドラインをパースし、それがearlyだったら、上記のobs_kernel_param内で紐付けられた関数を呼ぶという感じ。  
```
parse_early_param()
  -> parse_early_options(cmdline)
    -> parse_args(cmdline)
      -> do_early_param    // パラメータがearlyなら、obs_kernel_param に設定された関数を呼ぶ
```

## ドライバの登録
early_platform_driver_register_all() を使うと、特定のクラスに属する全ての early platform driver を登録できるよ。
また、early_platform_init() を使って初期化したplatfor driever は、この時点で自動的に登録されてるよ。  


## early platform driver の probe
システムは、特定のクラスを持つ登録済みの early platform driver と 登録済みの early platform deviceを紐付けるため、 ealy_platform_driver_probe() を呼ぶ。
マッチすれば、ドライバがprobe()を呼ぶ。

## early platform driver の probe()内部のお話
起動初期の時点でメモリ割り当てや割り込み登録をする場合、ドライバ側は色々注意しないとダメ。
自分とマッチしたのが、early platform device なのか、それとも、普通の platform device なのか等。

early_param("gbpages", parse_direct_gbpages_on)
カーネルパラメータとして "gbpages" が渡された場合、parse_direct_gbpages_on が呼ばれる。


＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ー
# ここからは、未完成

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

