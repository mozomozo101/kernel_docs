RZA1向けBSPのポーティングに関するメモ。  
[Linux4.9版](https://renesasrulz.com/rz/m/files_linux/3262/download)  
[Linux3.8版](https://renesasrulz.com/rz/m/files_linux/2719/download)  



## BSPの変換

### add_new_board.sh を実行
これが生成される
```
arch/arm/boot/dts/r7s72100-rztoaster.dts
arch/arm/mach-shmobile/board-rztoaster.c
arch/arm/configs/rztoaster_defconfig
```

### やること
* ピン設定
    * add_new_board.sh が生成したdtsにはピン設定が書かれているが、それは最低限のものなので、必用に応じて追記する。

* arch/arm/mach-shmobile/board-rztoaster.c
    * devicetreeを使うので不要かもしれないが、LEDハートビートなど、カスタムのstartupコードを入れられる。

* arch/arm/configs/rztoaster_defconfig
    * 必用に応じて追記する

## デバイスツリーの設定
rza1の殆どのドライバは、`arch/arm/boot/dts/r7s72100.dtsi`に集約されている。
これを、ボードごとのdtsから`#include`して使う。
dtsiでは、ほぼすべてのデバイス（compatibleプロパティがあるノード）は、disableになっている。
必要なデバイスについては、dts側で、status=okay となるよう設定を上書きする。
i2cデバイスを設定するなら、こんな感じ。

arch/arm/boot/dts/r7s72100.dtsi
```
i2c3: i2c@fcfeec00 {
	#address-cells = <1>;
	#size-cells = <0>;
	compatible = "renesas,riic-r7s72100", "renesas,riic-rz";
	...

	status = "disabled";
	...
};
```

arch/arm/boot/dts/r7s72100-rztoaster.dts
```
&i2c3 {
	pinctrl-names = "default";
	pinctrl-0 = <&i2c3_pins>;
	status = "okay";            ← 上書き
	clock-frequency = <100000>;
};
```

これでi2cデバイスを有効にできた。
あとは、このi2cデバイスがi2cコントローラとデータをやり取りできるよう、接続先のピンにi2cコントローラを割り当てる。
```
&pinctrl {
	/* RIIC ch3 */
	i2c3_pins: i2c3 {
	pinmux = <RZA1_PINMUX(1, 6, 1)>,	/* P1_6 (ALT1) = RIIC3SCL */
			 <RZA1_PINMUX(1, 7, 1)>;	/* P1_7 (ALT1) = RIIC3SDA */
	};
};
```
![pinctl](https://github.com/mozomozo101/kernel_docs/blob/edit/images/pinctl.png)


## カーネルの展開アドレスの変更
[RZ-RSKボードのマニュアル](https://www.renesas.com/jp/ja/doc/products/tool/doc/004/r20ut3007jg0100-rskrza1h-usermanual.pdf)によると、SDRAMはCS2（0x08000000）に割り当てられている。
従って、uImageの展開部分のコードは下記のようになる。
（この処理はカーネルの1機能なので、仮想アドレスが使われるため、0xf0000000が加算されている）。

arch/arm/boot/compressed/head.S
```
#ifdef CONFIG_AUTO_ZRELADDR
	@ determine final kernel image address
	mov r4, pc
	and r4, r4, #0xf8000000
	add r4, r4, #TEXT_OFFSET
#else
	ldr r4, =zreladdr
#endif
```

従って、手元のカスタムボードのSDRAMがCS3（0x0C000000）の場合、このように修正する。
* CONFIG_AUTO_ZRELADDR=n にする
* zreladdrを手動設定する
	arch/arm/mach-shmobile/Makefile.boot  
```
	loadaddr-y :=
	...
	
	loadaddr-$(CONFIG_MACH_RSKRZA1) += 0x08008000
	loadaddr-$(CONFIG_MACH_MYBOARD) += 0x0C008000	<< 追加
	
	...
	
	__ZRELADDR := $(sort $(loadaddr-y))
	zreladdr-y += $(__ZRELADDR)
	...
```
