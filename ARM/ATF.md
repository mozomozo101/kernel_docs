# Trusted Board Boot Requirements
[Trusted Board Boot Requirements CLIENT](https://developer.arm.com/docs/den0006/latest/trusted-board-boot-requirements-client-tbbr-client-armv8-a)

ARMは、セキュアなサービスを提供する上で必要な最も基本的なことは、「SoCリセット直後から、意図されたソフトウェアのみが動作すること」だとしてる。
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
https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/design/firmware-design.rst

Trusted Board Boot Requirements を実装したもの。
その他にも、電源管理とかについも実装してるらしい。

* BL1（EL3）  
リセットベクタからスタート。
EL3なのは、ROMに書かれたコードだから、改ざんの恐れがないってことだろう。
最低限の初期化、BL2をロードするためのストレージのセットアップ、BL2のロードと実行。

* BL2（EL1）  
BL3xやNormal Worldのソフトウェアをロードするためのセットアップ。
BL3xのロードと実行。
Root Of Trustによる検証がされていないので、BL2はEL1なのかな？

* BL3（EL3）  
多分だけど、ここからOPTEEやu-bootを起動する（secure bootのキモみたいなもの）ので
EL3を最高にする必要があったのかと。
EL3相当のセキュリティレベルを確保するために、BL2によるRoot Of Trust　を使った検証が必要なんだろう。
