# HW2 Simple Key-value Store

## Directories
- /server ->	server program related sources
- /client ->	client program related sources
- /common ->	common inclusions
- /util ->	common utilization
- /build ->	target build directory

## Building the Project
Code out your `/server/server.c` and `/client/client.c`, then
```shell
gcc -o server server.c -lpthread
gcc -o client client.c -lpthread
make
```

Test your `/build/server` and `/build/client`.
- server端輸入：

```shell
./build/server –p 1234
```

- client端輸入：

```shell
./build/client –h localhost –p 1234
```

## Implementations

### Please briefly describe your multi-threading design pattern

- client端

  在connect成功到指定的IP地址和Host後，創立一個用來receive的thread（向server發送資料）、一個用來send的thread（從server接收資料）

- server端

  建立一個receive的thread（接收從client送來的資料），在這個receive的中，會將client送來的訊息經過處理後，發送相對應指令的資料回去給client，另外為了避免road collision（防止Thread function同時讀寫變量的衝突），使用pthread_mutex_lock(&mutex)為全局變量加一個互斥鎖

### Please briefly describe the your data structure implementation
使用一個Hash Table，將會用到的函式放在hash.h中，在server.c中include "hash.h"，主要的資料結構為Hash Table

利用struct去定義每個資料的型態：

- Hash Table中會有item,linkedlist,size,count
  - item包含key,value
  - linkedlist則是與來串接item，包含item,likedlist *next(下一個item)
  - size為Hash Table的size
  - count則為計算目前table中有幾個item

## References
* [POSIX thread man pages](https://man7.org/linux/man-pages/man7/pthreads.7.html)
* [socket man pages](https://linux.die.net/man/7/socket)
