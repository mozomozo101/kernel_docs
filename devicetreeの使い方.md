
https://elinux.org/images/f/f9/Petazzoni-device-tree-dummies_0.pdf

# Device tree
## 


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


