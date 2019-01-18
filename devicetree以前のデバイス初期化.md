http://schedule2012.rmll.info/IMG/pdf/LSM2012_ArmKernelConsolidation_Petazzoni.pdf  
https://elinux.org/images/f/f9/Petazzoni-device-tree-dummies_0.pdf  
ここを参照。

# ボードファイル

多くの半導体ベンダが、ARMコアにいろんなペリフェラルをくっつけて、SoCとして販売している。
ペリフェラルには、UART、バスコントローラ（USB, SPI, I2C, PCI）、イーサネットコントローラなどなどたくさんある。
ペリフェラルは、SoC内にもボード上にもある。
その様子を表した画像が[こちら](https://github.com/mozomozo101/kernel_docs/blob/master/images/soc-board-peripherals.png)。

同一のSoCであっても、ボードが違えば、そこに載ったペリフェラルや設定が異なる。  
そのため、微妙に内容が異なる大量のボードファイルがLinuxソースツリーに次々と入り込み、そのカオスな状態にLinusは怒った。

## デバイスをprobeするまでの流れ

システムが各ペリフェラルを使うには、各ペリフェラルを初期化し、デバイスとして登録する必要がある。  
そのため、SoCのファイルとボードファイルを用意する。  
SoCファイルには、各ペリフェラルの初期化API、ベースアドレスや割り込み番号を書いておく。  
この初期化APIの中でピン設定などを行い、platform_device_register() 等でデバイスを登録する。  
そしてボードごとに用意されたボードファイルは、必要な分だけSoC用ファイルのペリフェラル初期化APIを呼ぶ。  
early_param() を使って、ドライバを起動初期段階で登録しておけば、この時点でドライバのprobe()関数が呼ばれる。  
![socとboard](https://github.com/mozomozo101/kernel_docs/blob/master/images/soc-file_board-file.jpg)

まとめると、流れはこんな感じ。  
* bootloaderが、r1レジスタにマシンIDを、r2レジスタにATAGへのポインタを入れ、kernel_entry()を呼ぶ
    * bootloaderやATAGについては[こちら](https://github.com/mozomozo101/kernel_docs/blob/edit/ARMLinuxのブート.md)にまとめた
* kernelが起動し、受け取ったマシンIDに紐付いたinit_machine() を呼ぶ
* ボードファイルは、デバイスや、ボード固有の情報を登録するために、SoC用のAPIを呼ぶ
* SoCファイルは、ピンマルチ設定したり、デバイスをplatformバスに登録する
* ドライバがprobe()を呼ぶ


## 例

## ボードファイル
これは、snapper9260 というボードで、ethernetポートを初期化する様子。  
SoC： AT91SAM9260（ARM9搭載）  
arch/arm/mach-at91/board-snapper9260.c  
```c
// 初期化関数。
// 各デバイスのadd, registerなど。
static void __init snapper9260_board_init(void)
{
    at91_add_device_i2c(snapper9260_i2c_devices, 
                        ARRAY_SIZE(snapper9260_i2c_devices));
    at91_register_uart(AT91SAM9260_ID_US0, 1, ATMEL_UART_CTS | ATMEL_UART_RTS);
    at91_add_device_eth(&snapper9260_macb_data);
    [...]
}

// machine_desc構造体で.init_machineをセット
MACHINE_START(SNAPPER_9260, "Bluewater Systems Snapper 9260/9G20 module")
    [...]
    .init_machine   = snapper9260_board_init,
MACHINE_END
```

### SoCファイル
**デバイス定義**  
ここでは、macb っていうデバイス。  
arch/arm/mach-at91/at91sam9260_devices.c

```c
static struct macb_platform_data eth_data;

static struct resource eth_resources[] = {
    [0] = {
        .start  = AT91SAM9260_BASE_EMAC,
        .end    = AT91SAM9260_BASE_EMAC + SZ_16K - 1,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = AT91SAM9260_ID_EMAC,
        .end    = AT91SAM9260_ID_EMAC,
        .flags  = IORESOURCE_IRQ,
    },
};

static struct platform_device at91sam9260_eth_device = {
    .name           = "macb",
    .id             = -1,
    .dev            = {
        .dma_mask               = &eth_dmamask,
        .coherent_dma_mask      = DMA_BIT_MASK(32),
        .platform_data          = &eth_data,
    },
    .resource       = eth_resources,
    .num_resources  = ARRAY_SIZE(eth_resources),
};
```

**SoC用API**  
arch/arm/mach-at91/at91sam9260_devices.c
```c
void __init at91_add_device_eth(struct macb_platform_data *data)
{
    [...]
    if (gpio_is_valid(data->phy_irq_pin)) {
        at91_set_gpio_input(data->phy_irq_pin, 0);
        at91_set_deglitch(data->phy_irq_pin, 1);
    }
    /* Pins used for MII and RMII */
    at91_set_A_periph(AT91_PIN_PA19, 0);    /* ETXCK_EREFCK */
    at91_set_A_periph(AT91_PIN_PA17, 0);    /* ERXDV */
    at91_set_A_periph(AT91_PIN_PA14, 0);    /* ERX0 */
    at91_set_A_periph(AT91_PIN_PA15, 0);    /* ERX1 */
    at91_set_A_periph(AT91_PIN_PA18, 0);    /* ERXER */
    at91_set_A_periph(AT91_PIN_PA16, 0);    /* ETXEN */
    at91_set_A_periph(AT91_PIN_PA12, 0);    /* ETX0 */
    at91_set_A_periph(AT91_PIN_PA13, 0);    /* ETX1 */
    at91_set_A_periph(AT91_PIN_PA21, 0);    /* EMDIO */
    at91_set_A_periph(AT91_PIN_PA20, 0);    /* EMDC */
    if (!data->is_rmii) {
        [...]
    }
    eth_data = *data;
    platform_device_register(&at91sam9260_eth_device);
}

```

**ドライバ**  
ロードした時点でprobeが呼ばれるようになってる。  
なので、ロードは、ボードファイルの処理が終わった後にする必要がありそう。
```c
static int __init macb_probe(struct platform_device *pdev)
{
}
static int __exit macb_remove(struct platform_device *pdev)
{
}
static struct platform_driver macb_driver = {
    .remove         = __exit_p(macb_remove),
    .driver         = {
        .name   = "macb",
        .owner  = THIS_MODULE,
    },
};
static int __init macb_init(void)
{
    return platform_driver_probe(&macb_driver, macb_probe);
}
static void __exit macb_exit(void)
{
    platform_driver_unregister(&macb_driver);
}
module_init(macb_init);
module_exit(macb_exit);

```
