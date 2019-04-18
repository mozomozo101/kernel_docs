詳しくはここを見ること。  
https://www.denx.de/wiki/pub/U-Boot/Documentation/multi_image_booting_scenarios.pdf

## zImage
圧縮されたカーネルイメージ + 展開コード + zImageヘッダ

## uImage
zImage + uImageヘッダ

## モノリシックImage
initramfs, kernel, dtb などを連結した1つのイメージ。
イメージのハッシュチェック等ができない。

## FitImage
kernel, initramfs, dtb などを連結したイメージ。
その構成を、Device Tree と同じフォーマットで記述できる。
ハッシュのチェックもするのでセキュリティにも良い。
