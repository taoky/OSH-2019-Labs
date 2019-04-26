# OSH 实验 2: Shell (README)

**Keyu Tao, PB17111630**

---

## 简介

`tsh` 实现了一部分简单的 Shell 功能，列表如下：

- 基于 `readline` 库的界面，包含了简单的对内置命令的自动补全。
- 内建命令有：
  - `cd`：如果没有参数，跳至家目录，否则跳至参数指定目录。
  - `pwd`
  - `export`：如果没有参数，效果与 `env` 相同，否则设置环境变量。
  - `exit`
- 支持管道（包括多重管道）。
- 支持使用 `>`, `<` 和 `>>` 的简单文件重定向（包括多重文件重定向）。
  - 支持基于文件描述符的文件重定向。
  - 支持 `<<` 与 `<<<`。
- 支持基于环境变量的变量代换。
- 支持部分转义符号特性：`\n`, `\r`, `\t` 和 `\\`。
- 在程序执行错误时，会显示程序返回值与 0337 相与的返回码（支持内建命令）。

## 编译

本程序使用 ~~C with `string` and `vector`~~ C++ 编写，多文件，包含了一个 Makefile 文件，需要提前安装 `readline` 库，并且在 `glibc` 下编译（由于使用了 `get_current_dir_name()` 等库函数）。在 `./lab2` 下执行 `make` 即可。

关于函数 `char** builtin_name_completion(const char*, int, int)` 的警告可以忽略。

## 演示

运行实验要求样例。

```
[tao@tao-linux-vmware src]$ ./sh
$ cd /
$ pwd
/
$ ls
bin		    dev   lib	      mnt   root	     sbin  tmp
boot		    etc   lib64       opt   rootfs-pkgs.txt  srv   usr
desktopfs-pkgs.txt  home  lost+found  proc  run		     sys   var
$ ls | wc
     21      21     125
$ ls | cat | wc
     21      21     125
$ env
SHELL=/bin/bash
SESSION_MANAGER=local/tao-linux-vmware:@/tmp/.ICE-unix/802,unix/tao-linux-vmware:/tmp/.ICE-unix/802
WINDOWID=58720259
COLORTERM=truecolor
XDG_CONFIG_DIRS=/etc/xdg
XDG_SESSION_PATH=/org/freedesktop/DisplayManager/Session0
XDG_MENU_PREFIX=xfce-
GTK_IM_MODULE=fcitx
SSH_AUTH_SOCK=/tmp/ssh-DvIkDBtCHIS3/agent.808
XMODIFIERS=@im=fcitx
DESKTOP_SESSION=xfce
SSH_AGENT_PID=809
EDITOR=/usr/bin/nano
GTK_MODULES=canberra-gtk-module:canberra-gtk-module
XDG_SEAT=seat0
PWD=/home/tao/VMWare/lab2/src
LOGNAME=tao
XDG_SESSION_DESKTOP=xfce
QT_QPA_PLATFORMTHEME=qt5ct
XDG_SESSION_TYPE=x11
XAUTHORITY=/home/tao/.Xauthority
XDG_GREETER_DATA_DIR=/var/lib/lightdm-data/tao
GTK2_RC_FILES=/home/tao/.gtkrc-2.0
HOME=/home/tao
LANG=zh_CN.utf8
LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=01;05;37;41:mi=01;05;37;41:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.pdf=00;32:*.ps=00;32:*.txt=00;32:*.patch=00;32:*.diff=00;32:*.log=00;32:*.tex=00;32:*.doc=00;32:*.aac=00;36:*.au=00;36:*.flac=00;36:*.m4a=00;36:*.mid=00;36:*.midi=00;36:*.mka=00;36:*.mp3=00;36:*.mpc=00;36:*.ogg=00;36:*.ra=00;36:*.wav=00;36:*.axa=00;36:*.oga=00;36:*.spx=00;36:*.xspf=00;36:
XDG_CURRENT_DESKTOP=XFCE
VTE_VERSION=5600
XDG_SEAT_PATH=/org/freedesktop/DisplayManager/Seat0
YAOURT_COLORS=nb=1:pkg=1:ver=1;32:lver=1;45:installed=1;42:grp=1;34:od=1;41;5:votes=1;44:dsc=0:other=1;35
GLADE_CATALOG_PATH=:
XDG_SESSION_CLASS=user
TERM=xterm-256color
USER=tao
DISPLAY=:0.0
SHLVL=2
QT_IM_MODULE=fcitx
XDG_VTNR=7
XDG_SESSION_ID=1
MOZ_PLUGIN_PATH=/usr/lib/mozilla/plugins
GLADE_MODULE_PATH=:
XDG_RUNTIME_DIR=/run/user/1000
GLADE_PIXMAP_PATH=:
XDG_DATA_DIRS=/usr/local/share:/usr/share
PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/lib/jvm/default/bin:/usr/bin/site_perl:/usr/bin/vendor_perl:/usr/bin/core_perl:/home/tao/OSH-lab/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin
GDMSESSION=xfce
DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus
MAIL=/var/spool/mail/tao
_=./sh
OLDPWD=/home/tao/VMWare/lab2
LINES=24
COLUMNS=78
$ export MY_OWN_VAR=1
$ env | grep MY_OWN_VAR
MY_OWN_VAR=1
$ Got an EOF. Exiting... （注：按下了 Control + D）
```

要求顺利完成。另外有一些特性如下：

- 对超级用户不同的 prompt。

```
[tao@tao-linux-vmware src]$ sudo ./sh
[sudo] tao 的密码：
# 
```

- prompt 错误显示。

```
$ does_not_exist
does_not_exist: No such file or directory
255| $ ping
Usage: ping [-aAbBdDfhLnOqrRUvV64] [-c count] [-i interval] [-I interface]
            [-m mark] [-M pmtudisc_option] [-l preload] [-p pattern] [-Q tos]
            [-s packetsize] [-S sndbuf] [-t ttl] [-T timestamp_option]
            [-w deadline] [-W timeout] [hop1 ...] destination
Usage: ping -6 [-aAbBdDfhLnOqrRUvV] [-c count] [-i interval] [-I interface]
             [-l preload] [-m mark] [-M pmtudisc_option]
             [-N nodeinfo_option] [-p pattern] [-Q tclass] [-s packetsize]
             [-S sndbuf] [-t ttl] [-T timestamp_option] [-w deadline]
             [-W timeout] destination
2| $ export naive!
export: Your input does not contain '='.
255| $ cd far_away
far_away: No such file or directory
255| $ pwd
/home/tao/VMWare/lab2/src
$ 
```

- 对内置命令的管道与文件重定向。

```
$ pwd
/home/tao/VMWare/lab2/src
$ pwd | wc
      1       1      26
$ pwd > osh
$ ls osh
osh
$ cat osh
/home/tao/VMWare/lab2/src
```

- 多重文件重定向。

```
$ cat < rl.cpp > osh
$ cat osh
#include "rl.h"

char const *builtin_names[] = {
    "cd", "pwd", "export", "exit", NULL
};

（以下输出省略）
```

- 基于文件描述符的文件重定向。

```
$ export wrong_exp 2> osh
255| $ cat osh
export: Your input does not contain '='.
$ cat test.cpp
#include <iostream>
#include <string>
using namespace std;

int main() {
    string s;
    cin >> s;
    cout << "Out: " << s << endl;
    cerr << "Err: " << s << endl;
    return 0;
}
$ g++ test.cpp
$ ./a.out < test.cpp > out 2> err
$ cat out
Out: #include
$ cat err
Err: #include
$ ./a.out < test.cpp > outt 2>&1
$ cat outt
Out: #include
Err: #include
$ ./a.out >> out 2>> err
$ ./a.out >> out 2>> err
naive!
$ cat out
Out: #include
Out: naive!
$ cat err
Err: #include
Err: naive!
```

- `<<`, `<<<` 与转义符号。

```
$ wc << EOF
this
is
a
very
very
long 
test
text
EOF
 8  8 35
$ cat <<< too\ young\ too\ simple,\ sometimes\ naive!\\
too young too simple, sometimes naive!\$ 
$ echo are\tyou\nOK
are	you
OK
$ 
```

- 变量代换。

```
$ env | grep PATH
XDG_SESSION_PATH=/org/freedesktop/DisplayManager/Session0
XDG_SEAT_PATH=/org/freedesktop/DisplayManager/Seat0
GLADE_CATALOG_PATH=:
MOZ_PLUGIN_PATH=/usr/lib/mozilla/plugins
GLADE_MODULE_PATH=:
GLADE_PIXMAP_PATH=:
PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/lib/jvm/default/bin:/usr/bin/site_perl:/usr/bin/vendor_perl:/usr/bin/core_perl:/home/tao/OSH-lab/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin
$ echo $PATH
/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/lib/jvm/default/bin:/usr/bin/site_perl:/usr/bin/vendor_perl:/usr/bin/core_perl:/home/tao/OSH-lab/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin
```

## 不支持的特性、与其它 Shell 可能不一致的特性与未定义行为

- 当 `cd` 与 `exit` 后有管道或文件重定向时，命令没有效果。

```
$ pwd
/home/tao/VMWare/lab2/src
$ cd | wc
      0       0       0
$ pwd
/home/tao/VMWare/lab2/src
$ cd > osh
$ pwd
/home/tao/VMWare/lab2/src
$ exit | wc
      0       0       0
$ exit > osh
$
```

- 若无法向 `/tmp` 写入文件，`<<` 与 `<<<` 会出现错误。
- 命令中同时包含标准输出文件重定向与管道时行为是未定义的。
- 命令中前后包含对相同文件描述符的重复重定向时行为是未定义的。
- 不支持通配符、任务管理。
- 不支持捕获 `SIGINT` 等信号，如果对某个程序按下 `Control + C`，`tsh` 也会被关闭。
- 变量代换
  - 只支持对一个 token 完全匹配情况下的替换。即 `aaa$HOME` 和 `$HOME.3` 都是不受支持的。
  - 如果出现了未定义的变量，`tsh` 会报错，而非替换为空字符串。