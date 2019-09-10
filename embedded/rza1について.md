# SoCモジュールについて
renesasが、家電、ヘルスケア、セキュリティ等の分野で使いやすいようなモジュールを1つのチップに集約したのが、RZA1シリーズってわけか。  
RZA1に搭載されたモジュールは、ここに書かれてる。  
https://www.renesas.com/jp/ja/products/microcontrollers-microprocessors/rz/rza/rza1h.html

大きく分けるとこんな感じだ。  
* CPU
* RAM
* システム系
  * DMAコントローラ、割り込みコントローラ、JTAG、暗号アクセラレータ など
* タイマー
  * RTC, WTD など
* グラフィック系
  * JPEGコーデック、ビデオデコーダ など
* 外部インターフェース
  * UART、i2cバスインターフェース、ethernetコントローラ、SPI,USB,GPIO など

# アドレス空間
アドレス空間（メモリ空間）は、CPUが直接アクセスできるアドレスの範囲のこと。  
32bit OSであれば、2^32 = 4GBのアドレス空間を持つことになる。    
RZA1のHWマニュアルでは、メモリアドレスの上位ビットによって、メモリ空間をこのように分類している。

|  アドレス  |  種類  |  外部/内部  |
| ---- | ---- | ---- |
|  0x00000000 - 0x17FFFFFF  |  CS0~5。通常空間、バイト選択付きSDRAM  |  外部空間  |
|  0x18000000 - 0x3FFFFFFF  |  SPIマルチIOバス空間、内蔵RAM、内臓周辺モジュール  |  内部空間  |

つまり、SDRAMやEEPROMはいずれもSoc外部にあるデバイスだが、アクセスに使うアドレス空間が違うってことだ。  
SDRAMはチップセレクトによって選択される外部アドレスを使ってアクセスする。  
EEPROM等のi2cデバイスは、内部アドレスを使ってi2cコントローラにアクセスし、それにデータ受け渡しを依頼する形になるというわけだ。  

![micon](https://github.com/mozomozo101/kernel_docs/blob/master/images/IMG_1148.jpg)


# 
# i2cについて
i2cコントローラ自体は内部アドレス空間にあるものの、そこに接続されている機器へは、i2cバス内でのアドレスを指定してアクセスする。
![micon](https://github.com/mozomozo101/kernel_docs/blob/master/images/IMG_1172.jpg)

# クロックについて
## クロック発振器
CPUも周辺デバイスも、クロックで同期を取っている。
そのクロックを発生させるのが、クロック発振器。

クロック発振器は、バスごとに各周辺デバイスのクロック入力端子に接続されている。  
水晶発振器などの発振回路から入力を受けると、バスごとにPLLで周波数変換（分周or逓倍）し、接続された周辺デバイスにクロックを供給する。  
各周辺デバイスは、入力されたクロックを、自身に合うよう、更に周波数変換して使用することがある。  

## rza1hの場合
![clock-rza1h](https://github.com/mozomozo101/tech_memo/blob/master/images/IMG_1153.jpg)

クロック発振器側の分周設定は、devicetreeで。

**r7s72100.dtsi**
```
	clocks {
		ranges;
		#address-cells = <1>;
		#size-cells = <1>;

		/* 発振回路に関する設定（外部クロック） */
		extal_clk: extal {
			#clock-cells = <0>;
			compatible = "fixed-clock";
			/* If clk present, value must be set by board */
			clock-frequency = <0>;
		};

		usb_x1_clk: usb_x1 {
			#clock-cells = <0>;
			compatible = "fixed-clock";
			/* If clk present, value must be set by board */
			clock-frequency = <0>;
		};

		rtc_x1_clk: rtc_x1 {
			#clock-cells = <0>;
			compatible = "fixed-clock";
			/* If clk present, value must be set by board to 32678 */
			clock-frequency = <0>;
		};

		rtc_x3_clk: rtc_x3 {
			#clock-cells = <0>;
			compatible = "fixed-clock";
			/* If clk present, value must be set by board to 4000000 */
			clock-frequency = <0>;
		};

		/* 各バスに出力するクロック設定 */
  /* ノード名は、バスクロックの名前に相当 */
		b_clk: b {
			#clock-cells = <0>;
			compatible = "fixed-factor-clock";
			clocks = <&cpg_clocks R7S72100_CLK_PLL>;
			clock-mult = <1>;
			clock-div = <3>;
		};
		p1_clk: p1 {
			#clock-cells = <0>;
			compatible = "fixed-factor-clock";
			clocks = <&cpg_clocks R7S72100_CLK_PLL>;
			clock-mult = <1>;
			clock-div = <6>;
		};
		p0_clk: p0 {
			#clock-cells = <0>;
			compatible = "fixed-factor-clock";
			clocks = <&cpg_clocks R7S72100_CLK_PLL>;
			clock-mult = <1>;
			clock-div = <12>;
		};
  ```

