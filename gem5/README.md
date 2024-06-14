# Gem5 fullsystem simulation pipeline

## Simulated machine
* ubuntu18.04
* In-Order CPU with 1core
* Single Thread
* Two-level MESI Cache
* DDR3

## Workload
* mm2 kernel

## Directory
### Kernel
The mm2 binary with gem5 ROI code inserted. Don't compile this directly! This  depends on lib of gem5, you should have gem5 built on your system first. libm5.a is a lib of m5 simulator, I put them on proj-root for simplicity. 

### diskImage
Configuration files for creating a diskImage to boot a OS from stratch. This is for the full-system simulation.

### m5out
The simulation results of Gem5.

### fullsysRun.py
Python scripts for Gem5 fullsystem simulation.
