# キャラクタ型デバイスのデータ構造
https://linux-kernel-labs.github.io/master/labs/device_drivers.html  
ここの翻訳。

キャラクタ型デバイスは、struct dev で表される。
これを操作するドライバが使う主要な構造体は、以下の３つ。  

* struct file_operations
* struct file
* struct inode

## file_operations 構造体

## inode構造体とfile構造体

inode 構造体は、ファイルシステムから見たファイルの表現で、個々のファイルを一意に識別する。  
サイズ、権限、アクセス時刻などの時刻情報などの情報を持つ。  

file構造体もファイルを表現するが、よりユーザの視点に近い。  
file構造体がもつ要素としては、 inode、ファイル名、ファイルの場所などがある。  
全てのファイルは、openされると、file構造体に紐づけられるようになる。  

file と inode　の違いは、オブジェクト指向プログラミングを例えに出すとわかりやすい。  
inodeがクラスだとすれば、files　はオブジェクト（inodeクラスのインスタンス）となる。  
inodeはファイルの静的イメージであり、fileは、そこから生成動的に生成されるファイルイメージといった具合だ。  

これをデバイスドライバに当てはめると、  
inodeは、操作対象となるデバイスのマイナー番号、メジャー番号を決定するために使われる。  
そしてfileは、ファイルを実際にopenする時のフラグの決定や、  
private_dataの記録やアクセスに使われる。  

struct file　の内容は、こんな感じ。  

* f_mode
    * READやWRITEなど
* f_flags
    * open時のフラグ（O_RDONLY, O_NONBLOCK　など）
* private_data
    * デバイス固有のデータを保存するためのポインタ。プログラマが設定。
* f_pos
    * ファイルにおけるオフセット

inode構造体は、いろいろな情報の他に、i_cdevフィールドを持つ。  
これは、キャラクタ型デバイスをへのポインタ（注１）。  

注1：struct cdev *i_cdev ということ。

# 実装
デバイスドライバを実装するには、デバイスの情報を含めた構造体と、  
ドライバ内でしようする情報を保持するための構造体を作ると便利。  
そのため、キャラクターデバイスのドライバにおいては、  
この構造体は、cdev構造体を参照させるようにしておきたい。  

```
#include <linux/fs.h>
#include <linux/cdev.h>

struct my_device_data {
    struct cdev cdev;
    /* my data starts here */
    //...
};

static int my_open(struct inode *inode, struct file *file)
{
    struct my_device_data *my_data;

    my_data = container_of(inode->i_cdev, struct my_device_data, cdev);

    file->private_data = my_data;
    //...
}

static int my_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset)
{
    struct my_device_data *my_data;

    my_data = (struct my_device_data *) file->private_data;

    //...
}
```

`struct my_device_data` の `struct cdev cdev` はキャラクタデバイスであり、  
これがシステムに登録されることで、デバイスは識別されるようになる。  
デバイスがopenされると、引数として受け取ったinodeから、container_ofマクロを使うことで、cdevを見つける（注2）。  
また、file->private_data に、struct cdev　を含めた my_device_data がセットされることで、  
以降、write/read/release などにおいて、これを参照できるようになる。  

注２：struct inodeには、対応するキャラクタデバイス struct cdev　へのポインタがある  





