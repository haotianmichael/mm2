# Gem5 fullsystem simulation pipeline(Deprecated for DAC!)
## Simulated machine
* ubuntu18.04
* In-Order CPU with 1core
* Single Thread
* Two-level MESI Cache
* DDR3

## Workload
* mm2 kernel

## full system simulation
### start from stratch 
You can build a DiskImage by your own, and use your DiskImage to boot OS and start full system simulation, this is for the simulation which requires huge benchmarks.
### modify out-of-box DiskImage 
If you have lightweight workloads to run, you can download fresh DiskImage from gem.resources. And mount it in your host machine and make your changes. ** mm2 use this way to do full system simulation**. Here is the according steps.

```
sudo losetup -fP ubuntu.18.04-img
sudo lsblk
sudo mount /dev/loop0p1 /mnt
cd /mnt   
---write your scripts---
sudo unmount /mnt
sudo losetup -d /dev/loop0

```

## Directory
* Kernel: The mm2 binary with gem5 ROI code inserted. Don't compile this directly! This  depends on lib of gem5, you should have gem5 built on your system first. libm5.a is a lib of m5 simulator, I put them on proj-root for simplicity. 

* diskImage: Configuration files for creating a diskImage to boot a OS from stratch. This is for the full-system simulation.

* m5out: The simulation results of Gem5.

* skylake: configuration files for skylake machine(TODO).

* fullsysRun.py: Python scripts for Gem5 fullsystem simulation.
