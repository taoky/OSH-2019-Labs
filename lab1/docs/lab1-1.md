# OSH 实验 1-1: 树莓派启动过程和原理

**Keyu Tao, PB17111630**

---

在一台传统的 PC 上，粗略的启动过程大致是：

1. BIOS 被电醒。
2. BIOS 检查并初始化硬件。
3. BIOS 加载 MBR 的 bootstrap code，启动非常短小的第一阶段 bootloader。
4. 由第一阶段 bootloader 加载、启动第二阶段 bootloader。
5. 第二阶段 bootloader 加载硬盘上的配置。在多系统的情况下，第二阶段 bootloader 一般会让用户选择操作系统。
6. 第二阶段 bootloader 根据参数启动操作系统内核。
7. 以 Linux 为例，内核自解压（如果被压缩了）、初始化、加载 `init` 程序（如 `systemd`）。
8. `init` 程序将需要打开的服务、守护进程等开出来。最后启动 X、桌面管理器（如 `gdm`）。
9. 桌面管理器显示登录框，用户输入用户名、密码后桌面管理器加载对应的桌面环境（DE）或窗口管理器（WM），它们可能再会打开需要的程序。启动完成。

而在树莓派上，启动时候发生了什么呢？

## 树莓派的不同启动阶段

1. 在树莓派上电的时候，GPU 首先启动，并且从 SoC 的 ROM 上加载第一阶段 bootloader（这 GPU 真厉害！）。

2. 第一阶段 bootloader 读取 SD 卡（根据 OTP (One-Time Programmable) 配置的不同，它也可以从其他的来源读取），尝试加载第二阶段 bootloader `bootcode.bin` 到二级缓存（此时内存尚不可用）。这个文件需要放置在 FAT32 或 FAT16 文件系统中。第二阶段的 bootloader 同样也在 GPU 上运行。

3. 第二阶段 bootloader 初始化内存。在 2012/10 的版本前，它会将第三阶段 bootloader `loader.bin` 从 SD 卡加载到内存中，但在之后的版本中，`loader.bin` 被移除了，第二阶段 bootloader 会直接加载 `start.elf`。

4. （如果有）第三阶段 bootloader 加载 `start.elf`。

5. 如果使用 `file` 查看 `start.elf` 的文件类型，会发现：

   ```
   start.elf: ELF 32-bit LSB executable, Broadcom VideoCore III, version 1 (SYSV), statically linked, stripped
   ```

   它还是在 GPU 上运行的。`start.elf` 会读取 `config.txt`，以此配置硬件。它还会读取 `cmdline.txt`，以作为启动内核 `kernel7.img` 的参数（对于另一些树莓派型号，加载的文件是 `kernel.img`）。

6. `start.elf` 加载 `kernel7.img`，内核开始运行，此时的程序才真正运行在 CPU 上。

7. 内核（假设是 Linux）根据启动的参数初始化硬件、其本身的一些数据，之后启动 `init`，剩下来的就和传统 PC 上启动 Linux 几乎没有区别了。

## 启动过程与传统计算机的差异

由上讨论可知，在加载内核之前，树莓派启动是和传统计算机相差比较大的。首先传统计算机中 BIOS/UEFI 干的活在树莓派上更像是第一阶段 bootloader 做的事情，并且传统计算机初始化需要 CPU，但树莓派上则是 GPU 做的。

第二点是，尽管树莓派要求的 SD 卡需要 MBR，但是它只使用 MBR 来读取分区结构，以找到 `boot` 分区，bootstrap 不会被理睬。而传统计算机上，BIOS 会加载 MBR 上的 bootstrap 来启动；而 UEFI 会读取 EFI 系统分区（ESP），选择指定的引导程序加载。

## 启动过程需要的文件系统

1. FAT16/32，作为 `boot` 分区的文件系统。上文提到，`bootcode.bin` 和 `start.elf` 等文件都在此分区上。使用 FAT 和 MBR，可能是因为对于固件来说，实现简单。
2. 加载的操作系统指定根目录的文件系统。对于 Linux 来说，一般是 `ext4`。

## Reference

1. [Linux startup process](https://en.wikipedia.org/wiki/Linux_startup_process)
2. [The BIOS/MBR Boot Process](https://neosmart.net/wiki/mbr-boot-process/)
3. [How does Raspberry Pi boot?](https://raspberrypi.stackexchange.com/questions/10489/how-does-raspberry-pi-boot)
4. [What is the boot sequence?](https://raspberrypi.stackexchange.com/questions/10442/what-is-the-boot-sequence)
5. [Boot flow](https://www.raspberrypi.org/documentation/hardware/raspberrypi/bootmodes/bootflow.md)
6. [Overview of Raspberry PI boot Sequence for FreeBSD](https://github.com/freebsd/crochet/tree/master/board/RaspberryPi/boot)
7. [kernel.img and kernel7.img?](https://www.raspberrypi.org/forums/viewtopic.php?t=101122)
8. [Standalone partitioning explained](https://github.com/raspberrypi/noobs/wiki/Standalone-partitioning-explained)
9. [Unified Extensible Firmware Interface](https://wiki.archlinux.org/index.php/Unified_Extensible_Firmware_Interface)