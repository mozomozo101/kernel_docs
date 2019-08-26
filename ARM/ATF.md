# Trusted Board Boot Requirements
[Trusted Board Boot Requirements CLIENT](https://developer.arm.com/docs/den0006/latest/trusted-board-boot-requirements-client-tbbr-client-armv8-a)

Global Platform は、TEE Protection Profile の中で、セキュアなブートプロセスについて仕様を書いてる。
Trusted Board Boot Requirements は、これを実現するためにARMが定めたfirmwareの仕様。
Secure worldとNormal Worldの分離、イメージへのサイン、それらに必要な仕様（API,機能など）などが書かれてる。
ELとかの概念も、ここで説明されてる。

* イメージの認証  
OTPに書かれたルート鍵を Root Of Trust　として、そこから認証された鍵でイメージをサイン。
イメージは、その鍵の証明書やイメージの署名と共に保存しておく。
使用時に認証する。

* ソフトウェアの構成  
ARMのCPUでは、Exception Level　が定められてる。
仕様の対象となるfirmwareは、EL3で動作する。


# TF-A
ATFのマニュアルのトップはここ。  
https://github.com/ARM-software/arm-trusted-firmware/blob/master/readme.rst
https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/design/firmware-design.rst

Trusted Board Boot Requirements を実装したもの。
その他にも、電源管理とかについも実装してるらしい。

* BL1（EL3）  
  * リセットベクタからスタート。
  EL3なのは、ROMに書かれたコードだから、改ざんの恐れがないってことだろう。
  最低限の初期化、BL2をロードするためのストレージのセットアップ、BL2のロードと実行。

* BL2（EL1）  
  * BL3xやNormal Worldのソフトウェアをロードするためのセットアップ。
  BL3xのロードと実行。
  Root Of Trustによる検証がされていないので、BL2はEL1なのかな？

* BL31（EL3）  
  * いわゆるセキュアモニター。
  ノーマルワールドとセキュアワールドの切り替えの管理が目的。
  ネットを見ていると、ATFという言葉を、BL31に対して使ってることもある。

* BL32
  * セキュアワールド。OPTEEなど。

* BL33
  * ノーマルワールドのローダ。u-bootとか。

ATFは、上のようなイメージから構成されるけど、実際、BL32やBL33はOPTEEやu-bootなどを使うので、ARMが実装しているのは、BL31までって感じだな。
