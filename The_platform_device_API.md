# The platform device API

かつてLinuxでは、システム起動前に、デバイスがどこにあるのかを、カーネルに伝える必要があった。  
この情報がないと、ドライバは、自分が対応するデバイスのIOポートや割り込み線がどれなのかわからなかった。  
でも今は、PCIのようなバスがある。  
PCIバスにつながったデバイスは、システムに対し、自分がどんなデバイスで、どのリソースを使うか伝えることができる。  
これにより、カーネルは起動時に、使用できるデバイスを全て把握できる。

しかし、組み込みの世界では、CPUから見つけられないデバイスがたくさんある（注：これは何故？）。  
このようなデバイス情報を取得する仕組みとして、Platform Devices がある。


## Platform drivers（未完成）
platform device は、`struct platform_device` で表現される（linux/platform_device.h）。  
platform device は、仮想的な"platform bus" に接続され、ここに対応するドライバ（platform driver）
が登録されることになる。

platform_driver構造体
```
struct platform_driver {
	int (*probe)(struct platform_device *);
	int (*remove)(struct platform_device *);
	void (*shutdown)(struct platform_device *);
	int (*suspend)(struct platform_device *, pm_message_t state);
	int (*resume)(struct platform_device *);
	struct device_driver driver;
	const struct platform_device_id *id_table;
};
```

platform_driver構造体において必要なものは、2種類ある。
* probe() と remove()
* バスがデバイスとドライバを紐付ける仕組み

後者には、2つの要素がある。
1つ目は id_table で、以下の形で表現される。
```
struct platform_device_id {
	char name[PLATFORM_NAME_SIZE];
	kernel_ulong_t driver_data;
};
```
デバイスが新しく接続されると、platform bus は、そのデバイスの名前をキーとして、対応するドライバを探す。
そして、ドライバのid_table のnameが一致していると、両者を紐付ける。  
・・・とはいえ、実際のところ、id_tableをセットしているplatform driverは少ない。  
大抵、platform_driver構造体の.driverフィールドに、名前を直打ちしてることが多い。  
例えば i2c-gpio ドライバの場合、こんな感じだ。

```
static struct platform_driver i2c_gpio_driver = {
	.driver		= {
		.name	= "i2c-gpio",
		.owner	= THIS_MODULE,
	},
	.probe		= i2c_gpio_probe,
	.remove		= __devexit_p(i2c_gpio_remove),
};
```

i2c-gpioという名前のデバイスは、全て、このドライバに紐付けられることになる。

platform driverは、以下の関数により、カーネルに自身の存在を知らせる。

```
int platform_driver_register(struct platform_driver *driver);
```





