[これ](https://renesasrulz.com/rz/m/files_linux/3262/download)のメモ。

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


