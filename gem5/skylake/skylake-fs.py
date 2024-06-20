""" Script to run a SPEC benchmark in full system mode with gem5.

    Inputs:
    * This script expects the following as arguments:
        ** kernel:
                  This is a positional argument specifying the path to
                  vmlinux.
        ** disk:
                  This is a positional argument specifying the path to the
                  disk image containing the installed SPEC benchmarks.
        ** cpu:
                  This is a positional argument specifying the name of the
                  detailed CPU model. The names of the available CPU models
                  are available in the getDetailedCPUModel(cpu_name) function.
                  The function should be modified to add new CPU models.
                  Currently, the available CPU models are:
                    - kvm: this is not a detailed CPU model, ideal for testing.
                    - o3: DerivO3CPU.
                    - atomic: AtomicSimpleCPU.
                    - timing: TimingSimpleCPU.
"""
import os
import sys

import m5
import m5.ticks
from m5.objects import *

from system.fs import MySystem
from system.ruby_system import MyRubySystem
from system.core import *

def create_system(linux_kernel_path, disk_image_path, detailed_cpu_model, memory_system):
    # create the system we are going to simulate
    ruby_protocols = [ "MI_example", "MESI_Two_Level", "MOESI_CMP_directory"]
    if memory_system == 'classic':
        system = MySystem(kernel = linux_kernel_path,
                        disk = disk_image_path,
                        num_cpus = 1, # run the benchmark in a single thread
                        TimingCPUModel = detailed_cpu_model)
    elif memory_system in ruby_protocols:
        system = MyRubySystem(kernel = linux_kernel_path,
                        disk = disk_image_path,
                        num_cpus = 1, # run the benchmark in a single thread
                        mem_sys = memory_system,
                        TimingCPUModel = detailed_cpu_model)
    else:
        m5.fatal("Bad option for mem_sys, should be "
        "{}, or 'classic'".format(', '.join(ruby_protocols)))

    # For workitems to work correctly
    # This will cause the simulator to exit simulation when the first work
    # item is reached and when the first work item is finished.
    system.work_begin_exit_count = 1
    system.work_end_exit_count = 1

    # set up the root SimObject and start the simulation
    root = Root(full_system = True, system = system)

    if system.getHostParallel():
        # Required for running kvm on multiple host cores.
        # Uses gem5's parallel event queue feature
        # Note: The simulator is quite picky about this number!
        root.sim_quantum = int(1e9) # 1 ms

    return root, system


def boot_linux():
    '''
    Output 1: False if errors occur, True otherwise
    Output 2: exit cause
    '''
    print("Booting Linux")
    exit_event = m5.simulate()
    exit_cause = exit_event.getCause()
    success = exit_cause == "m5_exit instruction encountered"
    if not success:
        print("Error while booting linux: {}".format(exit_cause))
        exit(1)
    print("Booting done")
    return success, exit_cause

def run_spec_benchmark():
    '''
    Output 1: False if errors occur, True otherwise
    Output 2: exit cause
    '''
    print("Start running benchmark")
    exit_event = m5.simulate()
    exit_cause = exit_event.getCause()
    success = exit_cause == "m5_exit instruction encountered"
    if not success:
        print("Error while running benchmark: {}".format(exit_cause))
        exit(1)
    print("Benchmark done")
    return success, exit_cause

def copy_spec_logs():
    '''
    Output 1: False if errors occur, True otherwise
    Output 2: exit cause
    '''
    print("Copying SPEC logs")
    exit_event = m5.simulate()
    exit_cause = exit_event.getCause()
    success = exit_cause == "m5_exit instruction encountered"
    if not success:
        print("Error while copying SPEC log files: {}".format(exit_cause))
        exit(1)
    print("Copying done")
    return success, exit_cause

if __name__ == "__m5_main__":

    cpu_name = "o3"
    mem_sys = "MESI_Two_Level"
    linux_kernel_path = "x86-linux-kernel-4.19.83"
    disk_image_path = "./mm2-x86-ubuntu-18.04-img"

    if not no_copy_logs and not os.path.isabs(m5.options.outdir):
        print("Please specify the --outdir (output directory) of gem5"
              " in the form of an absolute path")
        print("An example: build/X86/gem5.opt --outdir /home/user/m5out/"
              " configs-spec-tests/run_spec ...")
        exit(1)

    output_dir = os.path.join(m5.options.outdir, "speclogs")

    # Get the DetailedCPU class from its name
    detailed_cpu = getDetailedCPUModel(cpu_name)
    if detailed_cpu == None:
        print("'{}' is not define in the config script.".format(cpu_name))
        print("Change getDeatiledCPUModel() in run_spec.py "
              "to add more CPU Models.")
        exit(1)

    if not benchmark_size in ["ref", "train", "test"]:
        print("Benchmark size must be one of the following: ref, train, test")
        exit(1)

    root, system = create_system(linux_kernel_path, disk_image_path,
                                 detailed_cpu, mem_sys)

    # Create and pass a script to the simulated system to run the required
    # benchmark
    system.readfile = ("./skylake/readfile")

    # needed for long running jobs
    if not allow_listeners:
        m5.disableAllListeners()

    # instantiate all of the objects we've created above
    m5.instantiate()

    # booting linux
    success, exit_cause = boot_linux()

    # reset stats
    print("Reset stats")
    m5.stats.reset()

    # switch from KVM to detailed CPU
    if not cpu_name == "kvm":
        print("Switching to detailed CPU")
        system.switchCpus(system.cpu, system.detailed_cpu)
        print("Switching done")

    # running benchmark
    success, exit_cause = run_spec_benchmark()

    # output the stats after the benchmark is complete
    print("Output stats")
    m5.stats.dump()

    if not no_copy_logs:
        # create the output folder
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        # switch from detailed CPU to KVM
        if not cpu_name == "kvm":
            print("Switching to KVM")
            system.switchCpus(system.detailed_cpu, system.cpu)
            print("Switching done")

        # copying logs
        success, exit_cause = copy_spec_logs()
