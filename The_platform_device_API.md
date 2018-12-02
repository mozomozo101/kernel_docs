LWNの["The Platform device API"](https://lwn.net/Articles/448499/)の翻訳。

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
platform device は、仮想的な"platform bus" に接続され、ここに対応するドライバ（platform driver）が登録されることになる。

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
* デバイスが接続された際、バスが、そのデバイスとドライバを紐付ける仕組み

後者では、platform_driver構造体にデバイス名を保持しておき、新しくデバイスが接続されると、そのデバイス名をキーとして、対応するドライバを探す。  
platform＿driver構造体においてデバイス名を保持する方法は、２つある。  

1つ目は .id_tableフィールドの使用で、以下の形で表現される（が、この方法はあまり使われない）。
```
struct platform_device_id {
	char name[PLATFORM_NAME_SIZE];
	kernel_ulong_t driver_data;
};
```

２つ目は、.driverフィールドに名前を直打ちする方法で、大抵はこっち。  
例えば i2c-gpio ドライバの場合、こんな感じだ。
i2c-gpioという名前のデバイスは、全て、このドライバに紐付けられることになる。
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

### ドライバの登録
カーネルにplatform_driverを登録するには、以下の関数を実行する。
```
int platform_driver_register(struct platform_driver *driver);
```
成功すれば、以降、対応するデバイスが接続されると、ドライバのprobe() が呼ばれる。  
その際、引数として、インスタンス化するデバイスを表す platform_device構造体へのポインタが渡される。

```
struct platform_device {
	const char	*name;
	int		id;
	struct device	dev;
	u32		num_resources;
	struct resource	*resource;
	const struct platform_device_id	*id_entry;
	/* Others omitted */
    };
```

resource構造体は、そのデバイスが使用するリソース（Memory mappped IOレジスタ、割り込みetc.）の場所を指定する。  
使用するリソースの数（num_resouces）配列になっている。  
これらのリソース情報を取得するヘルパ関数が幾つか用意されている。  
ここで、引数"n" は、num_resources個のresource構造体のうち、何番目かを指定する。
```
struct resource *platform_get_resource(struct platform_device *pdev, 
					   unsigned int type, unsigned int n);
					   
struct resource *platform_get_resource_byname(struct platform_device *pdev,
					   unsigned int type, const char *name);
					   
int platform_get_irq(struct platform_device *pdev, unsigned int n);
```

例えば、ドライバは、


## platform device
はじめの方で書いたように、platform device は、カーネルに対してデバイスの存在を伝えるための仕組み。  
このためには、platform_device構造体を作成し、ドライバが自分を探す時に使う名前や、使用するリソースについての情報を、与える必要がある。
以下では、その簡単な例を示す。

```
static struct resource foomatic_resources[] = {
	{
		.start	= 0x10000000,
		.end	= 0x10001000,
		.flags	= IORESOURCE_MEM,
		.name	= "io-memory"
	},
	{
		.start	= 20,
		.end	= 20,
		.flags	= IORESOURCE_IRQ,
		.name	= "irq",
	}
    };

    static struct platform_device my_foomatic = {
	.name 		= "foomatic",
	.resource	= foomatic_resources,
	.num_resources	= ARRAY_SIZE(foomatic_resources),
    };
```

ここでは、foomatic という名前のplatform_deviceを定義している。  
.resouceフィールドより、0x10000000〜0x10001000 のMIMOと、IRQ20を使用することがわかる。

こうしてデバイスを定義したら、あとは
```
int platform_device_register(struct platform_device *pdev);
```
で、システムに知らせる。

カーネルはこれを受け取ると、対応するドライバを探す。  
見つかったら、ドライバのprobe() を呼び、デバイスをインスタンス化する。
デバイスの除去は、platform_device_unregister() で行う。

## platform data
platform_dataフィールドを使うことで、様々なデータをデバイス、ドライバ間で受け渡すことができる。
例えばi2c-gpioデバイスは、GPIOの数と、i2cクロック、データのライン数を、i2c-gpioドライバに渡している。
こんな感じだ。
ドライバは、probe()において、platform_data が指すデータを取得し、使うことができる。
```
#include <linux/i2c-gpio.h>

    static struct i2c_gpio_platform_data my_i2c_plat_data = {
	.scl_pin	= 100,
	.sda_pin	= 101,
    };

    static struct platform_device my_gpio_i2c = {
	.name		= "i2c-gpio",
	.id		= 0,
	.dev = {
		.platform_data = &my_i2c_plat_data,
	}
    };
```

















