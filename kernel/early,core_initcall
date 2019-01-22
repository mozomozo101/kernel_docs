あとで、きちんと書き直す。  

https://stackoverflow.com/questions/18605653/module-init-vs-core-initcall-vs-early-initcall

module_init()  
core_initcall()  
early_initcall()  
こういうのは、どう使うかの話。  

カーネル起動初期において、`do_basic_setup()`が呼ばれる。
```
static void __init do_basic_setup(void)
{
    cpuset_init_smp();
    usermodehelper_init();
    shmem_init();
    driver_init();
    init_irq_proc();
    do_ctors();
    usermodehelper_enable();
    do_initcalls();
}
```

do_initcall()では
```
static initcall_t *initcall_levels[] __initdata = {
    __initcall0_start,
    __initcall1_start,
    __initcall2_start,
    __initcall3_start,
    __initcall4_start,
    __initcall5_start,
    __initcall6_start,
    __initcall7_start,
    __initcall_end,
};

/* Keep these in sync with initcalls in include/linux/init.h */
static char *initcall_level_names[] __initdata = {
    "early",
    "core",
    "postcore",
    "arch",
    "subsys",
    "fs",
    "device",
    "late",
};

static void __init do_initcall_level(int level)
{
    extern const struct kernel_param __start___param[], __stop___param[];
    initcall_t *fn;

    strcpy(static_command_line, saved_command_line);
    parse_args(initcall_level_names[level],
           static_command_line, __start___param,
           __stop___param - __start___param,
           level, level,
           &repair_env_string);

    for (fn = initcall_levels[level]; fn < initcall_levels[level+1]; fn++)
        do_one_initcall(*fn);
}

static void __init do_initcalls(void)
{
    int level;

    for (level = 0; level < ARRAY_SIZE(initcall_levels) - 1; level++)
        do_initcall_level(level);
}
```

例えばmodule_initであれば、
```
#define module_init(x)  __initcall(x);
```

こう定義いる。
module_init(my_func) は、最終的にこう展開される。
```
static initcall_t __initcall_my_func6 __used
__attribute__((__section__(".initcall6.init"))) = my_func;
```

initcall6.initセクションに、my_funcが追加された感じだ。
ここで、__initcall6_startは、initcall6.initセクションの先頭ポインタ。
このため、順番に呼ばれてく。
