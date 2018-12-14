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









