savedcmd_/home/ngn/Desktop/projects/wkit/kernel/shrk.o := ld -m elf_x86_64 -z noexecstack --no-warn-rwx-segments   -r -o /home/ngn/Desktop/projects/wkit/kernel/shrk.o @/home/ngn/Desktop/projects/wkit/kernel/shrk.mod  ; ./tools/objtool/objtool --hacks=jump_label --hacks=noinstr --hacks=skylake --ibt --orc --retpoline --rethunk --sls --static-call --uaccess --prefix=16  --link  --module /home/ngn/Desktop/projects/wkit/kernel/shrk.o

/home/ngn/Desktop/projects/wkit/kernel/shrk.o: $(wildcard ./tools/objtool/objtool)
