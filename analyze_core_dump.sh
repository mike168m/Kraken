mkdir -p ./cores/

# set ulimit size to unlimited to force linux kernel to print dumps
ulimit -c unlimited

# override ubutnu apport nonesense. Screw you canonical !

sudo sysctl -w kernel.core_pattern=./cores/%e.%p.%h.%t

make

./kraken_demo

core_dump_file_path=./cores/$( ls ./cores -t | head -n1 )

gdb -c $core_dump_file_path -ex 'symbol-file ./kraken'\
                            -ex 'sharedlibrary'\
                            -ex 'exec-file ./kraken_demo'
                            -ex 'thread apply all bt full'
