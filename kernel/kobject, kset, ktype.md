kobjectは、元々、Linuxにおいて参照カウントを管理方法を統一するために作られた仕組み。
でも今では、デバイスモデルにおいて使われることがほとんど[1]。

# ３つの要素

## kobject
参照カウント、親へのポインタ、タイプ等の値を持つ。
kobject単体ではあまり意味をなさず、他の構造体に埋め込んで使う。
親へのポインタにより、kobject同士で階層構造を作る。

```
struct kobject {
	char *k_name;
	struct kref;			// 参照カウント
	struct kobject *parent;	// 親kobject（存在するなら）
	struct kset *kset;
	struct kobj_type;
}
```

## ktype
kobjectのタイプを表す。
また、kobjectが作成、破棄される時の振る舞いを管理する。
kobjectは、ktypeが定義されていないと行けない。


## kset
kobjectの集合。
機能は3つ。
* kobjectのグループのコンテナとなること
* sysfsにおけるサブディレクトリになれる。  
各kobjectは、それぞれ他のkobjectの親になれるため、結果的に、ksetは、階層の最上位に来ることになる。sysfsでは、
`/sys/bus/pci/drivers/serial/new-id`とかであれば、/sys/bus がサブシステム、pciがkset、serialがkobjectになる。
* kobjectのホットプラグに対応  
kobjectが追加されたり削除されたりすると、それをueventとしてユーザ空間に通知できる



```
struct kset {
        struct subsystem        *subsys;
        struct kobj_type        *ktype;
        struct list_head        list;			// list_head は、Linuxで使われる連結リスト
        struct kobject          kobj;
        struct kset_hotplug_ops *hotplug_ops;
};
```




# kobject初期化

* kobject初期化
```
void kobject_init(struct kobject *kobj, struct kobj_type *ktype);
```

* 親の登録
```
int kobject_add(struct kobject *kobj, struct kobject *parent,
                const char *fmt, ...);
```

これで、kobjectの名前とその親を設定され、sysfsに登録される。
特定のksetと紐付ける場合は、kobject_add() の前にやっておく。
親がksetの場合は、parentはNULLになる。

* 便利関数
_initと_addを同時にやってくれる、 kobject_init_and_add() なんてのもある。


* 属性の付加

```
int sysfs_create_file(struct kobject *kobj, struct attribute *attr);
int sysfs_create_group(struct kobject *kobj, struct attribute_group *grp);
```
で、kobjectに属性を追加できる。
	

# ueventの送信
```
int kobject_uevent(struct kobject *kobj, enum kobject_action action);
```
デバイスの抜き差しが発生した時、カーネルは、ueventを使い、ユーザスペースにそれを通知する。
ユーザスペースでこれを待ち受けるのが、udevd。
udevdデーモンを介して、udevは、デバイスファイルを/devに作ってくれる。
udevについては[3]を参照。

で、kobjectが登録されると、kernelは、ユーザ空間にそれを周知するため、ueventを送信する。
ueventには、デバイス追加を表す KOBJ_ADD や 取り出しを表す KOBJ_REMOVE がある。

```
static const char *kobject_actions[] = {  
        [KOBJ_ADD] =            "add",  
        [KOBJ_REMOVE] =         "remove",  
        [KOBJ_CHANGE] =         "change",  
        [KOBJ_MOVE] =           "move",  
        [KOBJ_ONLINE] =         "online",  
        [KOBJ_OFFLINE] =        "offline",  
};  
```


## サンプル
[4] にサンプルあり。
実行するとわかるけど、`/kernel/kset-object/[foo,bar,baz]/[foo,bar,baz]` ができる。
`/kernel/kset-object` がkset。
その下のfooとかがkobject。
その下のfooとかがkobjectが持つ属性という感じか。

# 参考URL
[1] https://www.quora.com/What-are-kernel-objects-Kobj  
[2] http://www.infradead.org/~mchehab/kernel_docs/unsorted/kobject.html  
[3] http://www.usupi.org/sysad/114.html  
[4] https://github.com/torvalds/linux/blob/master/samples/kobject/kset-example.c  
