# キャラクタデバイス・ドライバ
https://linux-kernel-labs.github.io/master/labs/device_drivers.html  
ここの翻訳。

# 概要
UNIXにおいては、ハードウェアは、スペシャルデバイスを通じてユーザからアクセスされる。  
これらは/dev ディレクトリにファイルとして存在し、そこへの read, open, wirte などのシステムコールは、  
OSによってデバイスドライバへリダイレクトされる。  
デバイスドライバは、ハードウェアとカーネルがやりとりするためのカーネルコンポーネント（大抵はモジュール）である。

UNIXにおいては、デバイスファイルは２種類ある（従ってデバイスドライバも２種類）。  
キャラクタと、ブロックである。  
そのデバイスがどちらの種類になるかは、転送速度、転送量、転送する際のデータ構成によって決まる。  

キャラクタデバイスは、転送速度が遅く、転送量も少ない。
例えば、キーボード、マウス、シリアルポートなどだ。  
一般的に、このようなデバイスの操作は、バイト単位で行われる。

ブロックデバイスは、転送量が大きく、データはブロック単位で転送される。
これに該当するのは、ハードディスク、CDROM、RAMディスクなどだ。  

これら２種類のデバイスドライバについて、Linuxでは、２つの異なるAPIを用意している。  

# major と minor
UNIX同様、Linuxでも、デバイスはそれぞれに一意の値を割り当てられている。  
この識別子はmajor, minor の２つのパートから構成される。  
majorはデバイスタイプ（IDE, SCSI, serialなどなど）を、minor は、各majorデバイスの通し番号である。
ローカルで使用できるmajor番号は、240〜254。
大抵、major はドライバの種類を、minorは個別のデバイスを識別する。  
また、ドライバはメジャー番号に紐づいており、同じメジャー番号を持つ全てのマイナー番号のデバイスに対して責任を負う。

```
# ls -la /dev/hda? /dev/ttyS?
brw-rw----  1 root disk    3,  1 2004-09-18 14:51 /dev/hda1      // block device (major, minor)=(3, 1)
brw-rw----  1 root disk    3,  2 2004-09-18 14:51 /dev/hda2      // block device (major, minor)=(3, 2)
crw-rw----  1 root dialout 4, 64 2004-09-18 14:52 /dev/ttyS0     // character device (major, minor)=(4, 64)
crw-rw----  1 root dialout 4, 65 2004-09-18 14:52 /dev/ttyS1     // character device (major, minor)=(4, 65)
```

major番号と各デバイス種類の紐付けは、Documentation/devices.txt に書かれている。  
また、自分が作ったオリジナルのデバイスに対してmajor番号を紐付けたい場合は、２つの方法(static, dynamic)がある。

静的にデバイスファイルを作成するなら、mknodコマンドを使う。
```
$ mknod /dev/mycdev c 42 0     // character deivce
$ mknod /dev/mybdev b 240 0    // block device
```

動的に作る場合は、udev を使う。

# キャラクタデバイスのデータ構造

キャラクタ型デバイスは、struct dev で表される。
これを操作するドライバが使う主要な構造体は、以下の３つ。  

* struct file_operations
* struct file
* struct inode

## file_operations 構造体
こういうの。
```
struct file_operations {
    struct module *owner;
    loff_t (*llseek) (struct file *, loff_t, int);
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
    ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
    [...]
    long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
    [...]
    int (*open) (struct inode *, struct file *);
    int (*flush) (struct file *, fl_owner_t id);
    int (*release) (struct inode *, struct file *);
    [...]
```

各関数が受け取る値は、ユーザが使うシステムコールの引数とは異なっている。  
例えば、open(2) はファイルのパスなどを受け取るが、上記のopen にはそれが無い。  
代わりに、file や inode を受け取っているだろう。  

## inode構造体とfile構造体

inode 構造体は、ファイルシステムから見たファイルの表現で、個々のファイルを一意に識別する。  
サイズ、権限、アクセスなどの時刻情報を持つ。  

file構造体もファイルを表現するが、よりユーザの視点に近い。  
file構造体がもつ要素としては、 inode、ファイル名、ファイルの場所などがある。  
全てのファイルは、openされると、file構造体に紐づけられるようになる。  

file と inode　の違いは、オブジェクト指向プログラミングを例えに出すとわかりやすい。  
inodeがクラスだとすれば、files　はオブジェクト（inodeクラスのインスタンス）となる。  
inodeはファイルの静的イメージであり、fileは、そこから動的に生成されるファイルイメージといった具合だ。  

これをデバイスドライバに当てはめると、inodeは、操作対象となるデバイスのマイナー番号、メジャー番号を決定するために使われる。  
そしてfileは、ファイルを実際にopenする時のフラグの決定や、private_dataの記録やアクセスに使われる。  

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

# オペレーションの実装
デバイスドライバを実装する際、デバイスに関する情報（キャラクタデバイスの場合はcdev構造体）とモジュールが使用する情報を、１つの構造体にまとめておくと便利。  
このような構造体として、以下に、`struct my_device_data` を示す。

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
このメジャー番号とマイナー番号は`dev_t`型によって保持され、MKDEVマクロを使うことで取得できる。

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
cdev_add　により、カーネルに通知する（dev_tとcdev_initを紐づける）。  
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

## 補足１（MKDEVについて）
dev_t 型は、major番号とminor番号を結合したもの。  

```
MKDEV(major, minor) とすると、majorとminorを結合し、dev_t型を作ってくれる。

dev_t型：
+------------------------+
|  major |     minor     |
+------------------------+
   12bit       20bit
```

## 補足２（register_chardev_regionは、何をしてるか）
カーネルでは、デバイス番号が被らないよう、major番号をchrdevというハッシュテーブルで、  
各デバイスに属すminor番号を、char_device_struct で記録しておく。  
register_chardev_region は、そこに指定レンジのメジャー、マイナーを登録し、確保するようだ。 
[ここ](https://books.google.co.jp/books?id=h0lltXyJ8aIC&pg=PT570&lpg=PT570&dq=char_device_struct&source=bl&ots=gP3oLW4cKS&sig=tiA4e__Mwhi1HlQgRhZdDyMDGp0&hl=ja&sa=X&ved=2ahUKEwif_KnTy_3eAhUDf7wKHU0JAzIQ6AEwA3oECAcQAQ#v=onepage&q=char_device_struct&f=false)を参照。  
デバイス識別子が割り当てられると、そのデバイスとメジャー番号が、[/proc/device](http://web.mit.edu/rhel-doc/4/RH-DOCS/rhel-rg-ja-4/s1-proc-topfiles.html) に現れる。



## 補足３（cdev_add, cdev_init について）

```
// cdev を初期化し、fopsを登録するだけだ。
void cdev_init(struct cdev *cdev, const struct file_operations *fops)
{
	memset(cdev, 0, sizeof *cdev);
	INIT_LIST_HEAD(&cdev->list);
	kobject_init(&cdev->kobj, &ktype_cdev_default);
	cdev->ops = fops;
}

// dev_t と cdev　を紐づけ、cdevをシステムに登録してる
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

## 補足４（register_chrdevについて）
[register_chrdev()](https://elixir.bootlin.com/linux/latest/ident/__register_chrdev)　は、register_chardev_region(), cdev_add(), cdev_init())をまとめてやってくれる。


## まとめ
新しくデバイスを登録するには、  
1. ユーザからアクセスするなら、mknodでデバイスファイルを作る  
2. register_chrdev_region で、デバイス識別子を確保する  
3. cdev_init でcdevを初期化する  
4. cdev_add でdev_tとcdevを紐づけることで、システムにデバイスを登録する  

ただし、2~4は、register_chrdev()　でも代用可能。


# プロセスのアドレス空間へのアクセス
ドライバは、アプリケーションとハードウェアのインターフェースとなるため、  
ユーザ空間へのアクセスが必要となることもある。
ユーザ空間へのダイレクトなアクセスは、異常な動作やセキュリティの問題を引き起こす。
そのため、以下のマクロや関数を使って行う。

```
#include <asm/uaccess.h>

put_user(type val, type *address);
get_user(type val, type *address);
unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
unsigned long copy_from_user(void *to, const void __user *from, unsigned long n)
```

* put_user: ユーザ空間のaddressアドレスに、値valをセットする
* get_user: ユーザ空間のaddressアドレスの値を、val にセットする
* copy_to_user: カーネル空間のfromアドレスから、ユーザ空間のtoアドレスに、nバイトコピー
* copy_from_user: ユーザ空間のfromアドレスから、カーネル空間のtoアドレスに、nバイトコピー

# open と release

# read と write（未完成）
readやwriteにおいて、ドライバが受け取るバッファのアドレスは、ユーザ空間のもの。  
そのため、copy_to_user や copy_from_user　が必要となる。  

read/write は、次の値を返すことができる。
* 正の値は、転送したデータのバイト数
* ０はファイルの終端を表す（readの場合）
* 負の値は、エラーコードを表す。

また、下記の処理を行わせる必要がある。
* バッファとデバイス間で、可能な限りデータを転送する
* 次のread/writeを開始する位置（オフセット）を更新する
* 転送したデータのバイト数を返す

以下では、バッファサイズ、ユーザ側のバッファサイズ、オフセットを考慮した
read関数の例。

```
static int my_read(struct file *file, char __user *user_buffer,
                   size_t size, loff_t *offset)
{
    struct my_device_data *my_data = (struct my_device_data *) file->private_data;
    ssize_t len = min(my_data->size - *offset, size);

    if (len <= 0)
        return 0;

    /* read data from device in my_data->buffer */
    if (copy_to_user(user_buffer, my_data->buffer + *offset, len))
        return -EFAULT;

    *offset += len;
    return len;
}
```

# ioctl
ioctlはこんな形。

```
static long my_ioctl (struct file *file, unsigned int cmd, unsigned long arg);
```
* cmd: ユーザ空間から送られてきたコマンド
* arg: ユーザ空間のバッファへのポインタ。copy_to_user, copy_from_user　でアクセス。


ioctlを実装する前に、cmdに相当する値を定義する必要がある。  
このコマンドの生成には、`_IOC(dir, type, nr, size)` というマクロを使うと良い。  
_IOC　マクロのパラメータは、以下のとおり。

* dir: データ転送のタイプを表す（_IOC_NONE, _IOC_READ, _IOC_WRITE）
* type: マジックナンバー(Documentation/ioctl-number.txt参照)
* nr: デバイスに対するioctlコード 
* size: 転送するデータのサイズ


# 待ち列（wait queue）
スレッドがデバイス操作の完了を待つという場面はよくあるが、ビジー状態での待ちは避けたいところ。  
待ち列を使うと、イベントが発生するまでスレッドをブロックすることができる。
待ち列は、特定のイベントの発生を待つプロセスのリストだ。  
このキューは、`wait_queue_head_t`型で表される。
ドライバは、このキューによって、自分を使おうとするプロセスを管理する。

```
#include <linux/wait.h>

DECLARE_WAIT_QUEUE_HEAD(wq_name);

// 待ち列の初期化
void init_waitqueue_head(wait_queue_head_t *q);

// conditionがfalseの間、現在のスレッドをキューに入れ（スリープさせ）、他のスレッドのためにスケジューラを呼ぶ。
// 他のスレッドが wake_up 関数を呼ぶと、スリープが解除される。
int wait_event(wait_queue_head_t q, int condition);
int wait_event_interruptible(wait_queue_head_t q, int condition);

// 上の２つと同じだが、timeoutで指定した時間を過ぎると、スリープが解除される。
int wait_event_timeout(wait_queue_head_t q, int condition, int timeout);
int wait_event_interruptible_timeout(wait_queue_head_t q, int condition, int timeout);

// 待ち列のプロセスを全て起こす
void wake_up(wait_queue_head_t *q);
void wake_up_interruptible(wait_queue_head_t *q);
```

以下に、フラグ値が変更されるまで待ち続けるスレッドの例を示す。

```
#include <linux/sched.h>

wait_queue_head_t wq;
int flag = 0;

init_waitqueue_head(&wq);
```

あるスレッドは、flagがゼロ以外になるまでスリープする。
```
wait_event_interruptible(wq, flag != 0);
```

別スレッドでは、flagを変更し、スリープしているスレッドを起こす
```
flag = 1 ;
wake_up_interruptible (&wq);
```


# 練習問題

* readすると適当な文字列を返すキャラクタデバイスを作成する
1. mknodを使って、/dev/so2_cdevというキャラクタデバイスノードを作る
2. /dev/so2_cdevの追加、削除を行うカーネルモジュール so2_cdev を作成する
3. モジュール挿入後、/proc/devices に当該デバイスが生成されることを確認する

* デバイスにアクセスできるプロセスを１つに制限する
残りのプロセスは、アクセスしようとすると、device busy を表す-EBUSY　のerrnoを受け取る。 
この処理は、ドライバのopen関数で実装する。
1. device構造体に atomic_t 変数を用意する
2. デバイス初期化時に、atomic_tを初期化する
3. open時、その変数を使ってアクセス制限を行う。[atomic_cmpxchg](https://www.khronos.org/registry/OpenCL/sdk/1.1/docs/man/xhtml/atomic_cmpxchg.html) を使うといいかも。
4. release時、atomic_tをリセットする
5. open時に、スケジューラをセットしておく（なんだこれは？）
```
set_current_state(TASK_INTERRUPTIBLE);
schedule_timeout(1000);
```
6. `cat /dev/so2_cdev & cat /dev/so2_cdev` を実行してみる。





























