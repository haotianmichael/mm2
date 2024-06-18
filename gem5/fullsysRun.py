from gem5.components.boards.x86_board import X86Board
from gem5.components.cachehierarchies.ruby.mesi_two_level_cache_hierarchy import (
    MESITwoLevelCacheHierarchy,
)
from gem5.components.memory import DualChannelDDR4_2400
from gem5.components.processors.cpu_types import CPUTypes
from gem5.components.processors.simple_switchable_processor import (
    SimpleSwitchableProcessor,
)
from gem5.isas import ISA
from gem5.resources.resource import obtain_resource, DiskImageResource
from gem5.simulate.exit_event import ExitEvent
from gem5.simulate.simulator import Simulator
import m5
from m5.objects import Root
from m5.stats.gem5stats import get_simstat


# Here we setup a MESI Two Level Cache Hierarchy.
cache_hierarchy = MESITwoLevelCacheHierarchy(
    l1d_size="32kB",
    l1d_assoc=8,
    l1i_size="32kB",
    l1i_assoc=8,
    l2_size="256kB",
    l2_assoc=16,
    num_l2_banks=1,
)

# Setup the system memory.
memory = DualChannelDDR4_2400(size="3GB")


# Here we setup the processor. This is a special switchable processor in which
# a starting core type and a switch core type must be specified. Once a
# configuration is instantiated a user may call `processor.switch()` to switch
# from the starting core types to the switch core types. In this simulation
# we start with Timing cores to simulate the OS boot, then switch to the
# Out-of-order (O3) cores for the command we wish to run after boot.
processor = SimpleSwitchableProcessor(
    starting_core_type=CPUTypes.TIMING,
    switch_core_type=CPUTypes.MINOR,
    isa=ISA.X86,
    num_cores=1,
)

# Here we setup the board. The X86Board allows for Full-System X86 simulations.
board = X86Board(
    clk_freq="3GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
)

# Here we set the Full System workload.
# The `set_kernel_disk_workload` function for the X86Board takes a kernel, a
# disk image, and, optionally, a command to run.

# This is the command to run after the system has booted. The first `m5 exit`
# will stop the simulation so we can switch the CPU cores from KVM to timing
# and continue the simulation to run the echo command, sleep for a second,
# then, again, call `m5 exit` to terminate the simulation. After simulation
# has ended you may inspect `m5out/system.pc.com_1.device` to see the echo
# output.
command=("echo 'About to build mm2';"
    +"cd /home/gem5/kernel;"
    +"gcc archKernel/mm2_chain_dp.c archKernel/kalloc.c -o mm2;"
    +"echo 'Build Done! About to execute mm2';"
    +"m5 exit 0;"
    +"m5 resetstats;"
    +"./mm2;"
    +"m5 dumpstats;"
    +"echo 'Done executing mm2';" 
    +"m5 exit 0;"
)



board.set_kernel_disk_workload(
    kernel=obtain_resource("x86-linux-kernel-4.19.83"),
    #disk_image=obtain_resource("x86-ubuntu-24.04-img"),
    #FIXME: Ubuntu18.04 doesnot support GLIBC2-34 for mm2 execution. so I can't set ROI to mm2.
    disk_image=DiskImageResource(local_path="mm2-x86-ubuntu-18.04-img",root_partition="1",source="src/x86-ubuntu", id="x86-ubuntu-18.04-img"),
    readfile_contents=command,
)

# Here we setup the simulator. We pass the board to the simulator and run it
# but also specify what to do when the simulation exits with the `EXIT` exit
# event. In this case we call the `processor.switch` function on the first
# exit event. For the 2nd the default action will be triggered which exists
# the simulator.
simulator = Simulator(
    board=board,
    on_exit_event={
        ExitEvent.EXIT: (func() for func in [processor.switch])
    },
)

simulator.run()
