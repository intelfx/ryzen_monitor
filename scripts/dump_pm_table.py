#!/usr/bin/env python3
#
# Dump the PM table repeatedly while idle and under one, two and all-thread
# load, to work out the layout of a new PM table version. The dumps go to
# ./pm_dumps and can be read back with "ryzen_monitor -t <file> -f <version>
# --dumptable". The load comes from pigz, which must be installed.

import os
import shutil
import struct
import subprocess
import sys
from time import sleep

FS_PATH = '/sys/kernel/ryzen_smu_drv/'
PM_PATH = FS_PATH + 'pm_table'
PMV_PATH = FS_PATH + 'pm_table_version'
CN_PATH = FS_PATH + 'codename'
CN_NAME_PATH = FS_PATH + 'codename_name'

SAMPLES = 15
DELAY = 1
DELAY_START = 10


def codename():
    try:
        with open(CN_NAME_PATH) as fp:
            return fp.read().strip()
    except OSError:
        pass
    # Drivers without codename_name only have the number.
    with open(CN_PATH) as fp:
        return 'codename{}'.format(int(fp.read()))


def pm_table_version():
    try:
        with open(PMV_PATH, 'rb') as fp:
            return struct.unpack('<I', fp.read(4))[0]
    except OSError:
        return 0


def dump(prefix, name, version, idx):
    os.makedirs('pm_dumps', exist_ok=True)
    path = 'pm_dumps/{}_{}_{:08x}_{}.bin'.format(prefix, name.replace(' ', '_').lower(), version, idx)
    print('Writing file: {} ...'.format(path))
    with open(PM_PATH, 'rb') as src, open(path, 'wb') as dst:
        dst.write(src.read())


def dump_under_load(prefix, name, version, pigz, threads):
    args = [pigz, '-11', '-c', '-f'] + (['-p', str(threads)] if threads else [])
    load = subprocess.Popen(args, stdin=open('/dev/zero', 'rb'), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        for i in range(SAMPLES):
            sleep(DELAY)
            dump(prefix, name, version, i)
    finally:
        load.kill()
        load.wait()


def main():
    if os.geteuid() != 0:
        sys.exit('Script must be run as root.')
    if not os.path.isfile(FS_PATH + 'version'):
        sys.exit('The driver does not seem to be loaded.')
    if not os.path.isfile(PM_PATH):
        sys.exit('PM Tables are not supported for this model of processor.')
    pigz = shutil.which('pigz')
    if not pigz:
        sys.exit('pigz is needed to load the processor. Please install it.')

    name = codename()
    version = pm_table_version()
    print('Code Name: {}'.format(name))
    print('PM Table Version: 0x{:08X}'.format(version))

    print('Dumping {} instances of the PM table while idle ...'.format(SAMPLES))
    sleep(DELAY_START)
    for i in range(SAMPLES):
        dump('idle', name, version, i)
        sleep(DELAY)

    for n, (prefix, threads, what) in enumerate((('1tload', 1, 'single thread'), ('2tload', 2, '2 threads'), ('load', 0, 'full'))):
        if n:
            sleep(DELAY_START)
        print('Dumping {} instances of the PM table during {} load ...'.format(SAMPLES, what))
        dump_under_load(prefix, name, version, pigz, threads)

    print('Done!')


if __name__ == '__main__':
    main()
