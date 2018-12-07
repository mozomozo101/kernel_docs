https://lwn.net/Articles/314900/

# early platform drivers V2
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



https://www.kernel.org/doc/Documentation/driver-model/platform.txt

early platform interface は、システムブートの初期段階に、platformデバイスドライバに対し、platform dataを渡す。  
ここでは、earlyprintk クラスのシリアルコンソールを導入する流れを示す。

## early platform device データの登録
platform deviceは、early_platform_add_devices() を使って、当該デバイスや、そのplatform dataを、
early_platform_device_list というデバイスリストに追加する。
シリアルコンソールの場合、ここで登録されるplatform_dataは、シリアルポート向けのハードウェア設定である。

## カーネルコマンドラインのパース



