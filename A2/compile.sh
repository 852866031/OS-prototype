#!/bin/bash
gcc -c shell.c interpreter.c shellmemory.c kernel.c cpu.c pcb.c ram.c
gcc -o mykernel shell.o interpreter.o shellmemory.o kernel.o cpu.o pcb.o ram.o
rm *.o
