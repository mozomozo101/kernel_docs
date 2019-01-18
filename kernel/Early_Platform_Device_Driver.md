# Early Platform Device and Drivers
これらをまとめたもの。   
https://www.kernel.org/doc/Documentation/driver-model/platform.txt  
https://lwn.net/Articles/314900/  

Early platform interface を使うと、システムの起動初期段階で、platform device driver にplatform data を渡すことができる。  
そのためには、予め [early_param() マクロ](https://github.com/mozomozo101/kernel_docs/blob/master/Architecture_specific_initializations.md)を使って、これらの処理をシステム初期段階で行うよう、登録しておく必要がある。

### early platform device データの登録  
platform deviceは
[early_platform_add_devices()](https://elixir.bootlin.com/linux/latest/source/drivers/base/platform.c#L1271) 
を使って、当該デバイスをearly_platform_device_listというearly platform device のリストに追加する。

### カーネルコマンドラインのパース
[parse_early_param()](https://github.com/mozomozo101/kernel_docs/blob/master/Architecture_specific_initializations.md)
を使うことで、カーネルのコマンドラインをパースし、それに紐付けられた処理が実行される。  
予め [early_param() マクロ](https://github.com/mozomozo101/kernel_docs/blob/master/Architecture_specific_initializations.md)によって、platform deviceを登録する処理を紐付けておけば、そこで登録されたplatform device は、起動の初期段階でシステムに登録される。


### ドライバの登録（自信無い）
early_platform_driver_register_all() を使うと、特定のクラスに属する全ての early platform driver を登録できるよ。  
また、early_platform_init() を使って初期化したplatfor driever は、この時点で自動的に登録されてるよ。  

```
#define early_platform_init(class_string, platdrv)		\
	early_platform_init_buffer(class_string, platdrv, NULL, 0)
	
#define early_platform_init_buffer(class_string, platdrv, buf, bufsiz)	\
static __initdata struct early_platform_driver early_driver = {		\
	.class_str = class_string,					\
	.buffer = buf,							\
	.bufsize = bufsiz,						\
	.pdrv = platdrv,						\
	.requested_id = EARLY_PLATFORM_ID_UNSET,			\
};	
```

```
 static int __init sh_cmt_init(void)
 {
        return platform_driver_register(&sh_cmt_device_driver);
 }
 
 static void __exit sh_cmt_exit(void)
 {
        platform_driver_unregister(&sh_cmt_device_driver);
 }
 
 early_platform_init("earlytimer", &sh_cmt_device_driver);
 module_init(sh_cmt_init);
 module_exit(sh_cmt_exit);
```

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

