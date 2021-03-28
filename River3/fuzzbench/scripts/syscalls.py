#!/usr/bin/env python3
from os import listdir
from os.path import isfile, join
from re import split
from sys import argv

def process(parts):
    for part in parts:
        if 'syscall' in part:
            function_name = split(' |:', parts[0])[1][1:-1]
            return function_name

def process_c(parts, c_functions):
    syscalls = set()
    for part in parts:
        if 'call' in part:
            try:
                function = part.split('call')[1].split()[1][1:-1]
                if function in c_functions:
                    syscalls.add(function)
            except:
                pass
    for function in syscalls:
        c_functions.remove(function)

    return syscalls

def find_sysclass(file_path):
    syscall_invoking_functions = []
    syscall_c_functions = set()

    # Get all Linux syscall functions
    # https://man7.org/linux/man-pages/man2/syscalls.2.html
    syscall_functions = set()
    with open("linux_syscalls", 'rt') as f:
        for syscall in f:
            syscall_functions.add(syscall.strip())
            syscall_functions.add('__libc_' + syscall.strip())
            syscall_functions.add('__' + syscall.strip())

    with open(file_path, 'rt') as f:
        part = []
        for line in f:
            part.append(line)
            if line == '\n':
                syscall = process(part)
                if syscall:
                    syscall_invoking_functions.append(syscall)

                syscall = process_c(part, syscall_functions)
                syscall_c_functions.update(syscall)

                part = []

    return syscall_invoking_functions, syscall_c_functions

def main(objdump_path):
    results = {}
    for objdump_file in listdir(objdump_path):
        print('Processing:', objdump_file)
        if isfile(join(objdump_path, objdump_file)):
            results[objdump_file] = find_sysclass(join(objdump_path, objdump_file))

    syscalls_invoking = set()
    syscalls_c = set()
    for objdump_file, (invoking_functions, c_functions) in results.items():
        print(objdump_file)
        print('Syscall invoking functions:')
        for function in invoking_functions:
            print('\t - ', function)
        print()
        print('Syscall C functions:')
        for function in c_functions:
            print('\t - ', function)
        print()

        syscalls_invoking.update(invoking_functions)
        syscalls_c.update(c_functions)

    print('Number of distinct functions invoking syscalls:', len(syscalls_invoking))
    for syscall in syscalls_invoking:
        print('\t', syscall)
    print()

    print('Number of distinct C functions:', len(syscalls_c))
    for syscall in syscalls_c:
        print('\t', syscall)



if __name__ == '__main__':
    if len(argv) != 2:
        print('Usage: python {} /path/to/text/objdump/')
        exit(1)
    main(argv[1])
