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
サイズ、権限、アクセスなどの時刻情報を持つ。  

file構造体もファイルを表現するが、よりユーザの視点に近い。  
file構造体がもつ要素としては、 inode、ファイル名、ファイルの場所などがある。  
全てのファイルは、openされると、file構造体に紐づけられるようになる。  

file と inode　の違いは、オブジェクト指向プログラミングを例えに出すとわかりやすい。  
inodeがクラスだとすれば、files　はオブジェクト（inodeクラスのインスタンス）となる。  
inodeはファイルの静的イメージであり、fileは、そこから動的に生成されるファイルイメージといった具合だ。  

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

inode構造体は、いろいろな情報の他に、キャラクタ型デバイス(struct cdev)へのポインタとなる、
i_cdevフィールドを持つ。  

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


# キャラクタデバイスの登録と削除
デバイスの登録と削除する際は、メジャー番号とマイナー番号を指定する。
これらのデバイス識別子は、MKDEVマクロを使って生成され、dev_t 型によって保持され、

デバイス識別子を静的に割り当てるには、`register_chrdev_region`　または`unresigrer_chrdev_region`を使用する。
動的に割り当てるには alloc_chrdev_region　関数を使う方法もあり、これもオススメ。

```
#include <linux/fs.h>

int register_chrdev_region(dev_t first, unsigned int count, char *name);
void unregister_chrdev_region(dev_t first, unsigned int count);
```


ここでは静的割り当ての様子を示す。  
メジャー番号は`my_majour`、マイナー番号は、`my_first_minor`から始まる`my_minotr_count`個を予約する。
```
#include <linux/fs.h>
...

err = register_chrdev_region(MKDEV(my_major, my_first_minor), my_minor_count,
                             "my_device_driver");
if (err != 0) {
    /* report error */
    return err;
}
...
```

デバイス識別子を割り当てたら、キャラクタデバイスは初期化し（cdev_init）、  
cdev_add　により、カーネルに通知する。  
cdev_add　は、デバイスが呼び出しに応答する準備ができた後に一度だけ呼ばれる。  
デバイスの除去は、cdev_del によって行われる。  


以下は、`MY_MAX_MINORS` 個のデバイスを登録、初期化している。

```
#include <linux/fs.h>
#include <linux/cdev.h>

#define MY_MAJOR       42
#define MY_MAX_MINORS  5

struct my_device_data {
    struct cdev cdev;
    /* my data starts here */
    //...
};

struct my_device_data devs[MY_MAX_MINORS];

const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write,
    .release = my_release,
    .unlocked_ioctl = my_ioctl
};

int init_module(void)
{
    int i, err;

    err = register_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS,
                                      "my_device_driver");
    if (err != 0) {
        /* report error */
        return err;
    }

    for(i = 0; i < MY_MAX_MINORS; i++) {
        /* initialize devs[i] fields */
        cdev_init(&devs[i].cdev, &my_fops);
        cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
    }

    return 0;
}
```

下記では、デバイスの除去と登録解除を行なっている。
```
void cleanup_module(void)
{
    int i;

    for(i = 0; i < MY_MAX_MINORS; i++) {
        /* release devs[i] fields */
        cdev_del(&devs[i].cdev);
    }
    unregister_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS);
}
```


## 個人的注釈（cdev_add, cdev_init について）

```
// cdev を初期化し、fopsを登録するだけだ。
void cdev_init(struct cdev *cdev, const struct file_operations *fops)
{
	memset(cdev, 0, sizeof *cdev);
	INIT_LIST_HEAD(&cdev->list);
	kobject_init(&cdev->kobj, &ktype_cdev_default);
	cdev->ops = fops;
}

// dev_t と cdev　を紐づけている
int cdev_add(struct cdev *p, dev_t dev, unsigned count)
{
	int error;

	p->dev = dev;
	p->count = count;

	error = kobj_map(cdev_map, dev, count, NULL,
			 exact_match, exact_lock, p);
	if (error)
		return error;

	kobject_get(p->kobj.parent);

	return 0;
}
```


# プロセスのアドレス空間へのアクセス


