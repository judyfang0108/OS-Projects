# ThreadScheduling

* [requirements](https://docs.google.com/presentation/d/1UFuPUwd17Hogh5Vp8GZbnrLRAddGvC1j/edit#slide=id.p3)



## Build

-  安裝 json parser

```bash
sudo apt-get install libjson-c-dev
```

- 安裝 git hook

```
make
```

- make simulator

```bash
make simulator
```

- delete .o檔

```bash
make clean
```
## Execution

- execute simulator

```bash
./simulator
```

-  按`Ctrl`+`Z` 會印出結果，按 `Ctrl`+`C` 程式會終止
## References

- [ucontext](https://pubs.opengroup.org/onlinepubs/7908799/xsh/ucontext.h.html)

- [signal handler](https://calvinkam.github.io/csci3150-Fall17-lab-pipes-signal/custom-signal-handler.html)

- [timer](https://www.ibm.com/docs/en/i/7.3?topic=ssw_ibm_i_73/apis/setitime.htm)

- [跟我一起寫Makefile](https://seisman.github.io/how-to-write-makefile/index.html)
