# tee_sqlite

## quick start

The current model is based on qemu for trustzone environment. 
qemu needs to be installed in a Linux environment. 
Ubuntu 20 is recommended. 


### QEMU installation
#### Dependency installation
```sudo dpkg --add-architecture i386```

Faraz: To check if above command worked or not, run dpkg --print-foreign-architectures
This should give "i386" as Output.


```sudo apt-get update```


```sudo apt install android-tools-adb android-tools-fastboot autoconf automake bc bison build-essential ccache codespell cpio cscope curl device-tree-compiler expect flex ftp-upload gdisk iasl libattr1-dev libcap-dev libcap-ng-dev libfdt-dev libftdi-dev libglib2.0-dev libgmp-dev libhidapi-dev libmpc-dev libncurses5-dev libpixman-1-dev libssl-dev libtool make mtools netcat ninja-build python-crypto python3-crypto python-pyelftools python3-pycryptodome python3-pyelftools rsync unzip uuid-dev xdg-utils xterm xz-utils zlib1g-dev```

#### Install repo

```mkdir ~/bin```

```curl https://mirrors.tuna.tsinghua.edu.cn/git/git-repo -o ~/bin/repo```

```chmod a+x ~/bin/repo```

```export PATH=~/bin:$PATH```


#### Get OP-TEE code
```mkdir OP-TEE```

```cd OP-TEE/```

```repo init -u https://github.com/OP-TEE/manifest.git -m qemu_v8.xml```

```repo sync -j4 --no-clone-bundle```

#### Compile
```cd ./build```

```make -j2 toolchains```

```make -j `nproc```

#### Pull the warehouse and run qemu
Pull the code from this repo and overwrite the code at the corresponding location.
```make run```

The first time you execute make run, it will compile the relevant components and then run qemu. After that, you can run make run-only directly.

After running correctly, two virtual serial port terminals will pop up. The one with port number 54320 is a non-secure system, and the one with port number 54321 is a secure system:

Enter the qemu terminal and enter c to start the system

Non-secure systems (Linux) can log in via the root user

Execute tee_sqlite run command
```optee_tee_sqlite```


### Farazuddin's interpretation:

The command "sudo dpkg --add-architecture i386 " enables 32-bit package installation on 64-bit Debian Linux systems.

This command enables multi-architecture support and adds the i386 architecture to your system, allowing you to install and use 32-bit packages alongside your 64-bit ones.

This command is useful for running older or legacy software that only comes in 32-bit versions or for ensuring compatibility with certain applications that require 32-bit libraries.

To check if above command worked or not, run dpkg --print-foreign-architectures
This should give "i386" as Output.

 Above were the steps for installing QEMU as part of setting up an OP-TEE (Open Portable Trusted Execution Environment) development environment. 
 
 These steps aim to ensure that you have the necessary dependencies, 
 including QEMU, to build and run an OP-TEE environment for TrustZone development.

 To verify if qemu is installed properly, run the following
 " qemu-system-arm --version "

 For errors like the following, 
 ```
 libtool : Depends: libc6-dev but it is not going to be installed or
                    libc-dev
 uuid-dev : Depends: libc6-dev but it is not going to be installed or
                     libc-dev
 zlib1g-dev : Depends: libc6-dev but it is not going to be installed or
                       libc-dev
E: Unable to correct problems, you have held broken packages.


 ```

 Above can happen for various reasons, but it's typically due to conflicts between package versions or because some packages are missing from the repositories.

Run the following steps:
Update the Package Lists: sudo apt-get update
Resolve Broken Packages: sudo apt-get -f install
Install missing dependencies: sudo apt-get install libc6-dev g++

To easily resolve dependency issues try "Synaptic" - it's a graphical package manager
```sudo apt-get install synaptic```

For errors like the following,
```
The following packages have unmet dependencies:
 libc6-dev : Depends: libc6 (= 2.31-0ubuntu9.7) but 2.31-0ubuntu9.9 is to be installed
E: Unable to correct problems, you have held broken packages.
```
Run the following steps:
Update the Package Lists: sudo apt-get update
Clear held packages: sudo apt-mark unhold libc6
This command explicitly specifies the desired version of libc6
```sudo apt-get install libc6=2.31-0ubuntu9.7```

```sudo apt-get install libc6-dev g++```

The command 
```export PATH=~/bin:$PATH``` is used to modify the PATH environment
 variable by adding a new directory (~/bin) to the beginning of the existing PATH 


#### Get OP-TEE code
```mkdir OP-TEE```
Purpose: This command creates a directory named "OP-TEE" in the current working directory. You'll likely use this directory to store the OP-TEE source code and related files.

```cd OP-TEE/```

```repo init -u https://github.com/OP-TEE/manifest.git -m qemu_v8.xml```
Purpose: This command initializes the repo tool for the OP-TEE project, pointing it to the manifest repository and specifying the desired manifest file. 
It sets up the project's repository structure and prepares it for the next step.

```repo sync -j4 --no-clone-bundle```
Purpose: This command downloads the source code of the OP-TEE project 
and its dependencies from the Git repositories specified in the manifest.
It synchronizes your local workspace with the remote repositories, ensuring you have the latest code.

#### Compile
```cd ./build```

```make -j2 toolchains```

```make -j `nproc```

The provided set of commands is used to compile 
and run the OP-TEE (Open Portable Trusted Execution Environment) project, 
specifically for a TrustZone environment using QEMU (Quick EMUlator).

This command will:
    -Change the current directory to the build directory.
    -Compile the OP-TEE toolchains using up to 2 parallel threads.
        * Compile OP-TEE using as many parallel threads as there are CPU cores on your system.
        * The -j option specifies the number of parallel threads to use. You can adjust this number depending on your system's capabilities. 
            For example, if you have a 4-core CPU, you can set -j4 to speed up the compilation process.

Once the compilation process is complete, you will have a working OP-TEE installation.

#### Pull the warehouse and run qemu
Pull the code from this repo and overwrite the code at the corresponding location.
```make run```

The first time you execute make run, it will compile the relevant components and then run qemu. After that, you can run make run-only directly.

After running correctly, two virtual serial port terminals will pop up. The one with port number 54320 is a non-secure system, and the one with port number 54321 is a secure system:

Enter the qemu terminal and enter c to start the system

Non-secure systems (Linux) can log in via the root user

Execute tee_sqlite run command  
```optee_tee_sqlite```

- After successfully entering the QEMU terminal 
  and logging in as the root user in the non-secure (Normal) world, 
  the next step is to access the secure world within the TrustZone environment
  
- Depending on your project's configuration, 
you may need to navigate to the directory where the optee_tee_sqlite component is located. 
Once in the appropriate directory, run the optee_tee_sqlite command to execute the trusted component within the secure world. The exact functionality and output will depend on your project's specific OP-TEE configuration and usage.

Additional:
https://medium.com/macoclock/share-folder-between-macos-and-ubuntu-4ce84fb5c1ad
Above is an article describing how to setup shared folders between host and virtual OS (Ubuntu)


