# OSH 实验 3: Web Server

**Keyu Tao, PB17111630**

------

## 简介

一个使用了 `epoll` 与多进程的简单的网络服务器，使用 ~~C + unordered_map~~ C++，适用于请求密集且受信任的网络中。

特性：

- 支持 URL 包含「大小写英文字母，数字，连字符下划线和斜线」以及「句点」符号的情况，会直接访问文件，返回内容。
  - 不支持文件名中含空格的情况（不支持 urlencode）。
  - 不支持列目录，也不会自动转向 `index.html`。
  - 对可能的路径穿越漏洞做了判断处理。
- 实验中的其他要求。

## 编译与运行

确保可以使用 `arm-linux-gnueabihf-g++`。直接 `make` 即可。将 `server` 丢到树莓派上，放置于网站根目录。运行即可。若没有问题，不会有任何输出。

## 设计

此程序会 `fork()` 出 `CPU 核心数` 个进程，每个进程维护一个 `epoll_fd`。epoll 设置为边缘触发模式。`epoll_wait` 返回之后，迭代返回数组中的文件描述符。

- 如果是 `serv_sock`，就 `accept()`，途中可能会有多个进程被唤醒（惊群），那么就忽略 EAGAIN 和 EWOULDBLOCK 错误。之后设置客户端 socket 为非阻塞的，设置事件属性为 `EPOLLIN | EPOLLET`，将其加入 `epoll_fd`。
- 如果对应事件有 EPOLLIN 属性，就使用 `handle_clnt()` 处理。这里会最多读取 8192 个字节（作为 HTTP 请求的第一行肯定是足够了），剩余的不读取（对我们处理没有用）。之后检查 PATH，打开文件，写 HTTP 返回头。如果需要返回文件内容，返回对应的文件描述符。然后放入 `unordered_map` 中，更改事件属性为 `EPOLLOUT | EPOLLET`，放回 `epoll_fd`。
- 如果对应事件有 EPOLLOUT 属性，就使用 `sendfile()` 向 socket 写 8192 个字节。如果写完，就关闭。否则继续放回 `epoll_fd`。

此外：

- 如果子进程崩溃了，会自动 `fork()` 出新的进程。
- 忽略了 `SIGPIPE` 信号，在下载大文件时断开连接不会导致程序结束。
- 对服务器 socket 设置了 `SO_REUSEPORT` 和 `SO_REUSEADDR`。

## `siege` 结果

```
siege -c 100 -r 100 http://172.20.10.11:8000/index.html
```

`index.html` 为 125 bytes，计算机与树莓派连接在同一手机热点中。在树莓派上测试，返回如下：

```
Transactions:		       10000 hits
Availability:		      100.00 %
Elapsed time:		        8.76 secs
Data transferred:	        1.19 MB
Response time:		        0.09 secs
Transaction rate:	     1141.55 trans/sec
Throughput:		        0.14 MB/sec
Concurrency:		       97.26
Successful transactions:       10000
Failed transactions:	           0
Longest transaction:	        0.19
Shortest transaction:	        0.01
```

增大 `-c` 与 `-r` 的值：

```
$ siege -c 200 -r 300 http://172.20.10.11:8000/index.html
... （省略）
Transactions:		       60000 hits
Availability:		      100.00 %
Elapsed time:		       50.24 secs
Data transferred:	        7.15 MB
Response time:		        0.16 secs
Transaction rate:	     1194.27 trans/sec
Throughput:		        0.14 MB/sec
Concurrency:		      193.39
Successful transactions:       60000
Failed transactions:	           0
Longest transaction:	        1.29
Shortest transaction:	        0.02
```

## 关于畸形请求

事实上，只要 HTTP 请求的第一行是这样的格式：

```
SOMESTRING PATH SOMESTRING
```

程序就会将第一个空格替换为 `.`，第二个空格替换为 `\0`，将指针指向原先的第一个空格处，检查是否路径穿越、文件是否存在、是否为 regular file。

如果不符合要求，会返回 `500`。由于这个逻辑，不能保证对于所有不标准的请求，都返回 `500`，但能够确保不会产生不安全的运行结果。

## 已知的问题

如果有大量的客户端保持「悬而未决」的连接，可能会导致服务程序无法响应其他的请求，造成 DoS 攻击。