# linux device model
https://linux-kernel-labs.github.io/master/labs/device_model.html    
https://static.lwn.net/images/pdf/LDD3/ch14.pdf

## バス

```
// バスの定義
struct bus_type ldd_bus_type = {
    .name = "ldd",
    .match = ldd_match,     // ドライバやデバイスが追加された時に呼ばれる
    .hotplug  = ldd_hotplug,// 
};

// バスを登録
ret = bus_register(&ldd_bus_type);

// バスをデバイスとしても登録が必要
struct device ldd_bus = {
    .bus_id   = "ldd0",
    .release  = ldd_bus_release
};

ret = device_register(&ldd_bus);
```

* 疑問
バスをデバイスとしても登録しなきゃいけないのは何故だろう？  
「device_register($ldd_bus)後、/sys/device/ldd0 が生成され、lddバスに接続されたデバイスが、その下に表示されるようになる」と書いてある。
単にデバイスの親子関係を構築するだけなのだろうか？


## デバイス
```
// lddデバイスの定義
struct ldd_device {
    char *name;
    struct ldd_driver *driver;
    struct device dev;    // struct deviceを埋め込む
};

// lddデバイス登録用関数
int register_ldd_device(struct ldd_device *ldddev)
{
    ldddev->dev.bus = &ldd_bus_type;
    ldddev->dev.parent = &ldd_bus;
    ldddev->dev.release = ldd_dev_release;
    strncpy(ldddev->dev.bus_id, ldddev->name, BUS_ID_SIZE);
    return device_register(&ldddev->dev);
}
EXPORT_SYMBOL(register_ldd_device);
```

## ドライバ
```
struct ldd_driver {
    char *version;
    struct module *module;
    struct device_driver driver;
    struct driver_attribute version_attr;
};

int register_ldd_driver(struct ldd_driver *driver)
{
    int ret;
    driver->driver.bus = &ldd_bus_type;
    ret = driver_register(&driver->driver);
    if (ret)
        return ret;
    driver->version_attr.attr.name = "version";
    driver->version_attr.attr.owner = driver->module;
    driver->version_attr.attr.mode = S_IRUGO;
    driver->version_attr.show = show_version;
    driver->version_attr.store = NULL;
    return driver_create_file(&driver->driver, &driver->version_attr);
};
```
