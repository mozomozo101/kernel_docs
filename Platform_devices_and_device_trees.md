lwnの[Platform_devices_and_device_trees](https://lwn.net/Articles/448502/)を翻訳。

platform deviceは便利だし、広く使われているものの、欠点もある。  
デバイスのリソース情報などをコードに埋め込む必要があるため、ボードごとに、それが搭載する全デバイスのリソース情報を、ファイルとして保持する必要がある。
そのため、ボードごとのファイルが、カーネルソース内で激増してしまう。
この解決策として、device treeを使う方法がある。

device treeは、システムのハードウェア構成情報を記述したテキスト。
カーネルは、ブート時にdevice treeを受け取り、どのようなシステムが動いているかを把握する。
device treeは、システムの差異をブートタイムデータとして抽象化することで、カーネルが幅広いハードウェア上で動かせるようになる。

device tree を使用するシステムにおいてplatform deviceを動かす場合、特別な操作は必要無い。


## 補足
[この記事](http://devicetree.org/Device_Tree_Usage)は、device tree事始めとしてすごく良いので、読むがいいさ。




