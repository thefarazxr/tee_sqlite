# tee_sqlite

## quick start

目前是基于qemu进行trustzone环境的模型，需要在linux环境下进行qemu的安装，推荐使用ubuntu 20

### QEMU安装
#### 依赖安装
```sudo dpkg --add-architecture i386```


```sudo apt-get update```


```sudo apt install android-tools-adb android-tools-fastboot autoconf automake bc bison build-essential ccache codespell cpio cscope curl device-tree-compiler expect flex ftp-upload gdisk iasl libattr1-dev libcap-dev libcap-ng-dev libfdt-dev libftdi-dev libglib2.0-dev libgmp-dev libhidapi-dev libmpc-dev libncurses5-dev libpixman-1-dev libssl-dev libtool make mtools netcat ninja-build python-crypto python3-crypto python-pyelftools python3-pycryptodome python3-pyelftools rsync unzip uuid-dev xdg-utils xterm xz-utils zlib1g-dev```

#### 安装 repo

```mkdir ~/bin```

```curl https://mirrors.tuna.tsinghua.edu.cn/git/git-repo -o ~/bin/repo```

```chmod a+x ~/bin/repo```

```export PATH=~/bin:$PATH```


#### 获取 OP-TEE 代码

```mkdir OP-TEE```

```cd OP-TEE/```

```repo init -u https://github.com/OP-TEE/manifest.git -m qemu_v8.xml```

```repo sync -j4 --no-clone-bundle```

#### 编译

```cd ./build```

```make -j2 toolchains```

```make -j `nproc```

#### 拉取仓库并运行 qemu

将本repo的代码拉取，并覆盖相应位置的代码，

```make run```

第一次执行 make run 会编译相关组件再运行 qemu，之后可以直接运行 make run-only 直接运行

正确运行后会弹出两个虚拟的串口终端，其中端口号为 54320 的是非安全系统，端口号为 54321 的为安全系统：

进入 qemu 终端后输入 c 启动系统

非安全系统（Linux）可以通过 root 用户登录

执行tee_sqlite运行命令

```optee_tee_sqlite```



