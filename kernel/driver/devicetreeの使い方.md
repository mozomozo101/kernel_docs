
https://elinux.org/images/f/f9/Petazzoni-device-tree-dummies_0.pdf


# 書き方
https://elinux.org/Device_Tree_Usage  
https://elinux.org/Device_Tree_Mysteries#Labels  
http://masahir0y.blogspot.com/2014/05/device-tree.html  

```
/dts-v1/;

/ {
    compatible = "acme,coyotes-revenge";	

    cpus {
        cpu@0 {
            compatible = "arm,cortex-a9";
        };
        cpu@1 {
            compatible = "arm,cortex-a9";
        };
    };
};

external-bus {
        ethernet@0,0 {
            compatible = "smc,smc91c111";
        };

        i2c@1,0 {
            compatible = "acme,a1234-i2c-bus";
            rtc@58 {
                compatible = "maxim,ds1338";
            };
        };

        flash@2,0 {
            compatible = "samsung,k8f1315ebm", "cfi-flash";
        };
};

soc {
	#address-cells = < 0x1 >;
	#size-cells = < 0x1 >;
        pic_3: pic@100 {
		reg = < 0x100 0x20 >;
               	interrupt-controller;
	 };
	uart {
		interrupt-parent = < &pic_3 >;
		interrupt-parent-path =  &pic_3 ;
	};
};
```


## 基本的な考え方

### ノード名
```
任意の文字列[@<unit-address]
```
のフォーマット。  
unit-addressは、そのデバイスにアクセスするための先頭アドレス（アドレスが割り当てられている場合）。
後述の reg で書かれた先頭アドレスに成る。

### デバイス
全てのデバイスは、devicetreeにおいて、ノードとして表現される。
その階層構造は、システムにおいてデバイス同士がどう接続されているかを表現している。
バスと、その下にぶら下がっているデバイスという構成。  
extenal-bus には、ethernet、i2c、flashなどのデバイスがぶら下がってるということ。

### compatible
デバイスを表現するノードは、compatibleプロパティが必要。
compatibleは、カーネルが、そのデバイスに紐づくドライバを特定するために使う。

```
compatible = "メーカー名,モデル名"
```

## アドレスの指定について
アドレスを割り当てられるデバイスは、以下の情報を持つ
* reg
* /#address-cells
* /#size-cells

regは、デバイスが使用するアドレス空間を表し、以下のフォーマットを取る（address, lengthはそれぞれ uint32）。
```
reg = <address1 length1 [address2 length2][addres3 length3] ...>  
または  
reg = <address>  
```

とはいえ、こんな風に、ただ値を羅列しても、どれがアドレスでどれが長さだかわからない。
下の例では、アドレスを４つ羅列しているのか、アドレスと長さの組みを２つ並べてるのか、わからない。
```
reg = <0x101f3000 0x1000 0x101f4000 0x0010>;
```

そこで親ノードで、こんなふうに、`#address-cells`と`#size-cells`を使って、address と length のセル数を指定する。
```
#address-cells = <1>;
#size-cells = <1>;
```
こうすれば、0x101f3000〜0x101f3fff, 0x101f4000〜0x101f40ff という２つのレンジを表せられるようになる。
reg内の各値はuint32だ。
そのため、32bitアーキテクチャであれば、#address-cells, #size-cells は1だが、64 bitアーキテクチャであればいずれも2になる。




### CPUのアドレス割り当て


### メモリにマップされたデバイス
```
external-bus {
        #address-cells = <2>;
        #size-cells = <1>;

        ethernet@0,0 {
            compatible = "smc,smc91c111";
            reg = <0 0 0x1000>;
        };

        i2c@1,0 {
            compatible = "acme,a1234-i2c-bus";
            reg = <1 0 0x1000>;
            rtc@58 {
                compatible = "maxim,ds1338";
            };
        };

        flash@2,0 {
            compatible = "samsung,k8f1315ebm", "cfi-flash";
            reg = <2 0 0x4000000>;
        };
    };
```

外部バスが割り当てられている外部アドレス空間は、チップセレクトによって複数に分割される。
そのようなアドレスは、アドレスの１つ目のセルをチップセレクト番号とすることで、表現できる。
上の例では、regは左から順に、
* チップセレクト番号
* チップセレクト番号によって指定されるアドレス空間内におけるオフセット
* 長さ
となる。

### メモリにマップされないデバイス
プロセッサからはアドレスをマップされないデバイスもある。
こういうデバイスは、CPUからは直接アクセスできず、親デバイスのドライバを介してのみアクセスする。
i2cデバイスを例にする。

```
i2c@1,0 {
            compatible = "acme,a1234-i2c-bus";
            #address-cells = <1>;
            #size-cells = <0>;
            reg = <1 0 0x1000>;
            rtc@58 {
                compatible = "maxim,ds1338";
                reg = <58>;
            };
        };
```

* 補足  
RTC(リアルタイムクロック)は、時計機能を実装したIC。  
i2cは周辺デバイスとシリアル通信するための規格なので、i2c経由でRTCを制御しているというわけだ。
[ここ](http://nahitafu.cocolog-nifty.com/nahitafu/2017/01/zynqrtc-07c3.html)を見るとわかるけど、
i2cデバイスをopenし、ioctlで目的のデバイスの(i2cデバイス上の)アドレスを渡すことで、制御できるようだ。

### 割り込み関連

* interrupt-controller
	* 現在のノードが割り込みコントローラであることを表す
* interrupt
	* 割り込みコントローラによって管理される割り込みを列挙する
	* ２つ以上の値を列挙可能で、それらの解釈は割り込みコントローラによる
		* 以下の例では、１つ目は割り込みID、２つ目は割り込みコントローラが使用するフラグ
* #interrupt-cells
	* interruptに列挙するセルの数
* interrupt-parent
	* 現在のノードに対する割り込みコントローラのphandle
	* rootノードにinterrupt-parentを書くと、それはシステム全体の割り込みコントローラになる。
		* 子ノードがその設定を継承するため
* interrupt-names
	* interruptsに列挙した値へのアクセス用の文字列を列挙。
	* 詳しくは[-nameプロパティ](#-nameプロパティ)

```
/ {
    compatible = "acme,coyotes-revenge";
    #address-cells = <1>;
    #size-cells = <1>;
    interrupt-parent = <&intc>;

    ...

    serial@101f0000 {
        compatible = "arm,pl011";
        reg = <0x101f0000 0x1000 >;
        interrupts = < 1 0 >;
    };

    intc: interrupt-controller@10140000 {
        compatible = "arm,pl190";
        reg = <0x10140000 0x1000 >;
        interrupt-controller;
        #interrupt-cells = <2>;
    };

    external-bus {

    ...
    
        ethernet@0,0 {
            compatible = "smc,smc91c111";
            reg = <0 0 0x1000>;
            interrupts = < 5 2 >;
        };

        i2c@1,0 {
            compatible = "acme,a1234-i2c-bus";
            #address-cells = <1>;
            #size-cells = <0>;
            reg = <1 0 0x1000>;
            interrupts = < 6 2 >;
            rtc@58 {
                compatible = "maxim,ds1338";
                reg = <58>;
                interrupts = < 7 3 >;
            };
        };
    };
};
```

### power-domains
よくわからないけど、SoCでは、電力管理の単位を、幾つかに分割してるらしい。
これは、各区画で調度良い電力を提供することで、省電力につなげるのが目的らしい。
power-domains は、この電力管理をするデバイスに関するデバイスが繋がるバスみたい。
	

### ラベル  
上での、pic_3: みたいなやつ。他ノードから参照するために使う。  
<&ipc_3> のように参照すると、そのラベルは後述するphandleに変換される。  
&pic_3 のように参照すると、そのノードへのフルパスとなる。  

```
        soc {
	        ...
                pic_3: pic@100 {
			reg = < 0x100 0x20 >;
                        interrupt-controller;
                };
		uart {
			interrupt-parent = < &pic_3 >;
			interrupt-parent-path =  &pic_3 ;
			
		};
        };
```
は、このように変換される。
```
	soc {
		...
		pic_3: pic@100 {
			reg = <0x100 0x20>;
			interrupt-controller;
			linux,phandle = <0x1>;
			phandle = <0x1>;
		};
		uart {
			interrupt-parent = <0x1>;
			interrupt-parent-path = "/soc/pic@100";
		};
	};
```
このように、ラベルを使ったノードを参照は、ノードのプロパティを後から変更する場合によく使う。  
これは、soc/pic@100 の regの値を変更する様子。

```
$ cat example_label_b.dts 

        soc {
		#address-cells = < 0x1 >;
		#size-cells = < 0x1 >;

                pic_3: pic@100 {
			reg = <0x100 0x20 >;
                        interrupt-controller;
                };

        };
	&pic_3 {
		reg = <0x200 0x30 >;
	};


$ dtc -O dts example_label_b.dts 
	soc {
		#address-cells = <0x1>;
		#size-cells = <0x1>;

		pic_3: pic@100 {
			reg = <0x200 0x30>;    ← 書き換わってる
			interrupt-controller;
		};
	};
``` 


### phandle  
各ノードを識別するための通し番号で、デバイスツリー内で一意。  
他のノードがそのノードを参照するときに使う。  
このようにすると、picノードを参照するために、値1のphandleが定義される。
```
      pic@10000000 {
              phandle = < 1 >;
      };
```
  
他のデバイスは、このpicノードを、`< 1 >`で参照できるようになった。  
uartノードが自身の割り込みコントローラを指定するなら、こうなる。
```
      uart@20000000 {
              interrupt-parent = < 1 >;
      };
```

とはいえ、実際のところ、このようにphandleを整数値を使って明示的に指定することは少ない。  
大抵の場合、こうやる。
```
      PIC_3: pic@10000000 {
              interrupt-controller;
      };

      uart@20000000 {
              interrupt-parent = < &PIC_3 >;
      };
```

`&`は、それに続く文字列が、同名のラベルへのphandleであることを示す。  
DTCは、phandleとして使われるようなラベルを見つけると、値がかぶらないように、ランダムな整数を生成する。  
つまりこの場合、`< &PIC_3 >` のようにphandle参照している部分と、`PIC_3: pic@10000000` というラベルを使っている部分があるため、
DTC内部で、ランダム値に変換されるというわけ。

## 補足

### -nameプロパティ
devicetreeにおいて、複数の値を列挙したリストを見る。
これらは通常、indexを使ってアクセスされるが、デバイスよっては、indexより文字列の方が識別しやすい場合もある。
そんな時のために、-nameプロパティを用意している。
以下の例では、reg の <0x4a064000 0x800> は、"config"という名前で

```
usb {
	compatible = "ti,usb-host";
	reg = <0x4a064000 0x800>, <0x4a064800 0x200>,
	      <0x4a064c00 0x200>;
	reg-names = "config", "ohci", "ehci";
	interrupts = <14>, <15>;
	interrupt-names = "ohci", "ehci";
};
```

https://elixir.bootlin.com/linux/latest/source/Documentation/devicetree/bindings/resource-names.txt


## 使い方
ブートローダがDevice Treeを・・・
* サポートしている場合
    * カーネルとDTイメージをロードして、カーネルを起動する
    * device tree以前とは異なり、r1レジスタは使わない
    * DTBのアドレス、カーネルパラメータ等をr2レジスタに入る。
    * bootm <*kernel addr*> <*dtb addr*>
* サポートしてない場合
    * CONFIG_ARM_APPENDED_DTB をyにしてカーネルをビルド
    * kernelに、「kernelイメージのすぐうしろにDTBがあるから、探してね」と伝えられる
    * kernel + DTB は、自分で用意する必要あり
    ```
    $ cat arch/arm/boot/zImage arch/arm/boot/dts/myboard.dtb >> my-zImage
    $ mkimage ... -d my-zImage my-uImage
    ```
    * あくまでデバッグ用。


## dtb, dts, dtc
Device Source Tree（DTS）を記述して、
Device Tree Compiler（DTC）でコンパイルすると、
Device Tree Blob（DTB）が出来上がる。
このdtbを、ブート時にカーネルへ渡す。

これで中身見れる。
dtc -I dtb -O dts Image-r8a7795-salvator-x.dtb

# わからないこと
interrupt-parent がGICを指していない場合の意味がよくわからない。
interrupt-parent = <&gpoi0>とか。
