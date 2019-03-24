# OSH 实验 1-2: 利用 Linux 实现 LED 闪烁

**Keyu Tao, PB17111630**

---

*感谢 fjw 同学提供的官方工具链和内核源代码的 `.git` 文件，省去了大量 `git clone` 的时间。*

编译内核照着要求来就行了，没啥好写的，不过最刺激的还是裁剪内核了！

## 裁剪内核

在 `menuconfig` 中有大量的选项可以选择。我们的程序就是闪个灯而已，要一堆奇奇怪怪的功能和硬件驱动干嘛？裁了！

咳咳，首先还是要理一下思路，不然碰到彩虹图就不好玩了……

1. 所有的内核模块都可以去掉。要求中我们的编译命令如下。

   ```shell
   make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j8 zImage modules dtbs
   ```

   但是呢，我们最后安装内核的命令是：

   ```shell
   sudo cp arch/arm/boot/zImage mnt/fat32/$KERNEL.img # 安装内核
   ```

   可以看到，我们只用到了 `zImage`，编译的内核模块没有对闪灯起到任何作用，所以直接关，减少编译时间。

   最后把模块支持也关掉就行。

2. `Networking support` 可以整个关掉，`File systems` 里面可以把除了 `ext4` 和 `/proc`, `sysfs` 以外的东西全关掉（`vFAT` 理论上也能关，但没试）。`Cryptographic API` 和 `Library routines` 也同样是能关则关。

3. `Device Drivers` 也可以整掉不少东西，不过要小心，无法明确用途的最好不要关，与 LED 灯和 SD 卡有关的也不能关。

4. `General Setup` 里面可以选择压缩算法（LZMA 压缩率最大）和优化模式（选择为大小优化），还有个调试符号表可以去掉。

其他的就可以随便看看试试了。

附：此实验内核对应的 `.config` 也在 `lab1/files/` 里面。由于关闭了 deadlock 调度器，屏幕上会显示一行错误，但不影响正常功能；同时把树莓派 logo 也去掉了。

## `/dev/sdc1` 中重要的文件

正如 1-1 报告中提到的那样，除了 `kernel7.img` 外，`bootcode.bin`、`start.elf` （不同情况下会加载其他 `start*.elf`）、`config.txt`、`cmdline.txt` 也是很重要的。此外，对应硬件型号的 `*.dtb` 也会被加载，这些文件最终会传给内核……

但是实践证明：我们最小只需要三个文件（对于实验 1-3），即 `kernel7.img` 、`bootcode.bin` 和 `start.elf` 。而对此实验，还需要 `cmdline.txt` 和对应型号的 `bcm2710-rpi-3-b.dtb`。

## `/dev/sdc1` 的文件系统

用到了 FAT32 文件系统。因为 SoC 上 ROM 固件需要（只支持）从此格式的文件系统中读取 bootloader。可以换成 FAT16。

## 关于 `init`

加载 `init`（而不是别的程序）是因为 `cmdline.txt`。

```
dwc_otg.lpm_enable=0 console=serial0,115200 console=tty1 root=PARTUUID=7ee80803-02 rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait quiet init=/init
```

`init=/init` 指定了 `init` 文件的位置。

## 需要哪些 Linux 编译选项

详见「裁剪内核」部分。

## Linux 为什么会 panic

因为 `init` 退出了。`init` 作为 Linux 启动时开启的第一个程序，有着举足轻重的作用，它也是其他所有进程的父进程。`init` 一般不会像本实验中直接退出，当 `init` 退出后，Linux 系统一部分功能就无法正常完成，很多时候是 `init` 根本没有正常启动成功或者是在运行时崩溃了。所以 Linux 内核选择崩溃。

## References

1. [The boot folder](https://www.raspberrypi.org/documentation/configuration/boot_folder.md)
2. [Device Trees, overlays, and parameters](https://www.raspberrypi.org/documentation/configuration/device-tree.md)