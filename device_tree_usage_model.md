# Device tree 使用モデル
https://www.kernel.org/doc/Documentation/devicetree/usage-model.txt  

DeviceTree（DT）は、以下3つの目的のために使う。
* プラットフォームの特定
* 起動時の設定
* デバイスの登録

## プラットフォームの特定
カーネルは起動初期段階で、DTを使って自分どのマシンで動いているを知り、それに特化した設定を行う。  
ARMであれば、  
* setup_arch(): arch/arm/kernel/setup.c  
* setup_machine_fdt(): arch/arm/kernel/devtree.c  
という順で呼ばれていく。  
setup_machine_fdt() は、ブートローダから受け取ったDTのルートノードにあるcompatibleの値をキーとして、それにマッチするmachine_desc構造体を探していく。  
その際、比較には、各machine_desc構造体（arch/arm/include/asm/mach/arch.h）内のdt_compatに列挙された文字列を使う。  

compatibleには、マシンの正確な名前と、その後ろにオプションの名前が列挙されている。  
例えば TIのBeagleBoardと、その後継マシンであるBeagleBoard xM board は、
いずれもARM7を搭載した[OMAP3ファミリ](https://ja.wikipedia.org/wiki/Texas_Instruments_OMAP#OMAP_3%E3%82%B7%E3%83%AA%E3%83%BC%E3%82%BA)
のSoCであるOMAP 3450 を搭載したボード。  
compatibleはこのようになる。
```
compatible = "ti,omap3-beagleboard", "ti,omap3450", "ti,omap3";
compatible = "ti,omap3-beagleboard-xm", "ti,omap3450", "ti,omap3";
```
"ti,omap3-beagleboard-xm" がマシンの正確な名前。  
そこから右に向かって、SoC名、SoCファミリ名と、だんだんざっくりとした名前になっていく。  
compatibleに複数のマシン名を入れておくのは、同じSoCファミリやSoCを持つのボード間で、被ってる設定をなるべく共有するため。  
上の例では、"ti,omap3450", "ti,omap3" は、それぞれ、SoCごと、SoCファミリごとの設定となる。  

なお、compatibleの文字列は、全て、[ここ](https://github.com/torvalds/linux/tree/master/Documentation/devicetree/bindings)に
ドキュメント化されていなければならない。  

※ compatibleリストされた全ての要素について、machine_descが読み込まれる？？？


## ランタイム設定
ARMでは、ボードに対応するmachine_descが選択された後、ブート初期におけるデバイスツリーのスキャンは、[setup_machine_fdt()(https://elixir.bootlin.com/linux/v4.19.9/source/arch/arm/kernel/devtree.c#L218)が行う。
ここでは、そのスキャンに関する説明をする。

DTは、カーネルパラメータやinitrdイメージの位置などをカーネルに伝える役割もある。
このようなデータは、大抵、/chosen ノードにある。
```
	chosen {
		bootargs = "console=ttyS0,115200 loglevel=8";
		initrd-start = <0xc8000000>;
		initrd-end = <0xc8200000>;
	};
```
bootargsはカーネルパラメータを、initrd-* は、initrdの場所を表す。
なお、initd-endは、initrdイメージの直後のアドレスなので注意すること。

ブート初期段階、アーキテクチャごとのセットアップコードは、[of_scan_flat_dt()](https://elixir.bootlin.com/linux/latest/source/drivers/of/fdt.c#L704)に渡されたコールバック関数によって、必要なデータを取得していく。  
early_init_dt_scan_chosen() は、chosenノード内のカーネルパラメータを、  
early_init_dt_scan_root() は、address-cells や size-cells の値を、  
early_init_dt_scan_memory() は、memoryノードを、それぞれパースしていく。 

## デバイスの登録
ボードが識別され、初期設定データが解析されると、カーネルの初期化は通常の方法で進める。
その中で、machine_desc構造体内に登録されている.init_early(), .init_irq(), .init_machine() などが呼ばれる。
名前の通り、.init_early() はブート初期の段階で必要なマシンのセットアップを、.init_irq() は割り込み設定を行う。
そしてその中で、DTから必要なデータを取り出すために、DTのクエリ関数（include/linux/of\*.h 内の of_\*関数）を呼ぶことができる。

特に注目するべきは、init_machine() で、これは、DTををパースすることでデバイスのリストを取得し、それらをデバイスとして登録する。
DT以前は、ボードファイルを使って、platform_device等を使って静的に定義していたが、DTを使うと、これらを動的に生成できるようになるのだ。
ここでは簡単な例として、platform_deviceをDTを使って表現してみる。
DTをではplatform deviceという概念は無いが、ルートに配置されたデバイスノードと、バスノードにぶら下がった子ノードで表現される。
ここでは、NVIDIAのTegraボードを例にする。

```
/{
	compatible = "nvidia,harmony", "nvidia,tegra20";
	#address-cells = <1>;
	#size-cells = <1>;
	interrupt-parent = <&intc>;

	chosen { };
	aliases { };

	memory {
		device_type = "memory";
		reg = <0x00000000 0x40000000>;
	};

	soc {
		compatible = "nvidia,tegra20-soc", "simple-bus";
		#address-cells = <1>;
		#size-cells = <1>;
		ranges;

		intc: interrupt-controller@50041000 {
			compatible = "nvidia,tegra20-gic";
			interrupt-controller;
			#interrupt-cells = <1>;
			reg = <0x50041000 0x1000>, < 0x50040100 0x0100 >;
		};

		serial@70006300 {
			compatible = "nvidia,tegra20-uart";
			reg = <0x70006300 0x100>;
			interrupts = <122>;
		};

		i2s1: i2s@70002800 {
			compatible = "nvidia,tegra20-i2s";
			reg = <0x70002800 0x100>;
			interrupts = <77>;
			codec = <&wm8903>;
		};

		i2c@7000c000 {
			compatible = "nvidia,tegra20-i2c";
			#address-cells = <1>;
			#size-cells = <0>;
			reg = <0x7000c000 0x100>;
			interrupts = <70>;

			wm8903: codec@1a {
				compatible = "wlf,wm8903";
				reg = <0x1a>;
				interrupts = <347>;
			};
		};
	};

	sound {
		compatible = "nvidia,harmony-sound";
		i2s-controller = <&i2s1>;
		i2s-codec = <&wm8903>;
	};
};
```
.init_machine() において、Tegraボード用のコードは、compatibleプロパティを持ったノードを探し、それらに対し、platform_deviceを作成していく。





