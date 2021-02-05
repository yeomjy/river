# Contents

- [Complete Fuzzing Solutions for Embedded Firmware](#complete-fuzzing-solutions-for-embedded-firmware)
- [Tools for Embedded Firmware Assesment](#tools-for-embedded-firmware-assesment)
	- [CPU Emulators](#cpu-emulators)
		- [Qiling](#qiling)
		- [QEMU vs. Unicorn](#qemu-vs-unicorn)
	- [Peripherals Emulation](#peripherals-emulation)
		- [HALucinator](#halucinator)
		- [P2IM & DICE](#p2im-dice)
			- [P2IM](#p2im)
			- [DICE](#dice)
		- [Pretender](#pretender)
	- [Fuzzers](#fuzzers)
		- [hal-fuzz](#hal-fuzz)
		- [unicornafl](#unicornafl)
		- [afl-unicorn](#afl-unicorn)
		- [TriforceAFL](#triforceafl)

# Complete Fuzzing Solutions for Embedded Firmware

All the fuzzing solutions presented below:

1. do **not** require access to the source code; 
2. do **not** need any dedicated hardware;

 | Approach | CPU Emulation Framework | Peripherals Emulation | Baseline Fuzzer | Architectures Supported | Instrumentation |
 |-------------|-------------|-------------|-------------|-------------|-------------|
 | [Qiling](https://qiling.io) and [unicornafl](https://github.com/AFLplusplus/unicornafl) | [Qiling](https://qiling.io) | None | AFL++ | x86(16/32/64), ARM(64), MIPS, EVM, WASM | [unicornafl](https://github.com/AFLplusplus/unicornafl) |
 | [hal-fuzz](https://github.com/ucsb-seclab/hal-fuzz) | [Unicorn](https://github.com/unicorn-engine/unicorn) | [HALucinator](https://github.com/embedded-sec/halucinator)  |  AFL | M-profile ARM CPU | [afl-unicorn](https://github.com/Battelle/afl-unicorn) |
 | [P2IM](https://github.com/RiS3-Lab/p2im) and [TriforceAFL](https://github.com/nccgroup/TriforceAFL) | [QEMU](https://www.qemu.org) | [P2IM](https://github.com/RiS3-Lab/p2im) | AFL | ARM | AFL QEMU mode|

\* AFL is used in black-box fuzzer mode for non-instrumented binaries (i.e. for binaries that **cannot be re-compiled** with `afl-gcc`). See AFL [docs](https://github.com/google/AFL#6-fuzzing-binaries) for more details.

# Tools for Embedded Firmware Assesment

## CPU Emulators

 | CPU Emulator | Architectures Supported |
 |-------------|-------------|
 | [Qiling](https://github.com/qilingframework/qiling) | x86(16/32/64), ARM(64) MIPS, EVM, WASM |
 | [Unicorn](https://www.unicorn-engine.org/) | Arm, Arm64 (Armv8), M68K, Mips, Sparc, & X86 (include X86_64) |
 | [QEMU](https://www.qemu.org) | [complete list](https://wiki.qemu.org/Documentation/Platforms) |

### Qiling

Check the Github [repository](https://github.com/qilingframework/qiling) and the [documentation](https://qiling.io/).

Qiling is a binary emulation framework that extends the features of Unicorn. It provides a high-level API to emulate binaries and various features to analyze them (see [documentation](https://docs.qiling.io/en/latest/)).

Qiling can be used to fuzz closed-source binaries. The authors are offering a [fuzzing demo](https://docs.qiling.io/en/latest/demo/#fuzzing-with-qiling-unicornafl) with `unicornafl`. 
`unicornafl` will be responsible for the I/O communication between the underlying Unicorn instance and AFL++ and for starting eact the fuzzing iteration.

The advantages of Qiling for fuzzing closed-source binaries are:

 - easy to write a Python script to start the fuzzing process.
 - allows instrumentation at various levels such as instruction/basic-block/memory-access/exception/syscall/IO.
 - if needed, Qiling already has a small database of mock-up syscalls for Linux.  

*Note:* In order to fuzz with Qilling with a custom fuzzer, the underlying Unicorn project should be patched to integrate with that specific fuzzer.

### QEMU vs. Unicorn
QEMU is a generic machine emulator. On the otherside, Unicorn is a framework for CPU emulation based on QEMU. 

Documentation for QEMU can be found [here](https://www.qemu.org/documentation/).

Documentation for Unicorn can be found [here](https://www.unicorn-engine.org/docs/). Also, check an in-depth [presentation](https://www.unicorn-engine.org/BHUSA2015-unicorn.pdf) from 2015 about Unicorn.

An article provided by Unicorn devs describing the main differences between QEMU and Unicorn can be checked [here](https://www.unicorn-engine.org/docs/beyond_qemu.html).

QEMU is focused on emulating whole systems and has a lot of bloatware code for emulating devices such as mouse, keyboard, sound card etc. The Unicorn project has stripped all this bloatware and emulated only CPU instructions. This makes it more suitable/efficient for emulating embedded binaries.   

**Note:** Unicorn provides a built-in layer of dynamic instrumentation vs. QEMU that has only static instrumentation. 

## Peripherals Emulation

### HALucinator
The code for HALucinator can be reviewed in the Github [repository](https://github.com/embedded-sec/halucinator).
The details of the concepts can be read in their [paper](https://www.usenix.org/system/files/sec20-clements.pdf).

Notes from from their paper:

- the goal of HALucinator is to fully re-host embedded firmware (i.e. no need for dedicated hardware to run the firmware).
- the hardest part of the re-hosting process is creating mock-ups/handlers for peripheral devices (without proper mock-ups for peripherals the firmware will crash early in the execution process or, even worse, will not even boot).
- HALucinator makes use of HAL libraries to create mock-ups for peripherals.
- *Hardware Abstraction Libraries* (HAL) are libraries provided by board vendors to ease the process of writing firmware code. Most of the HALs/SDKs are publicly available which is a key advantage for HALucinator.
- HALucinator enables the process of replacing HALs with high level implementation function (i.e. handlers comunicating with the emulator are replacing the functions that were formerly coupled with the hardware).
- LibMatch is a tool used to locate the HALs function addresses in the firmware (by using binary analysis and the source code provided by the vendors).
- For each new collection of HAL functions (or SDK) the developer should write his own replacement functions.
	- Once the developer is translating a HAL library to handlers, the code can be re-used for every board that is using that specific HAL library. 
	- most of the high level implementation are trivial (returning constants or interacting with the model itself and returning a result).
	- a few of them are not trivial to implement (when the HAL library does not abstract the hardware well enough or the HAL library uses in-the-house elements like its own heap allocator).
- *Note:* Setting up HALucinator to emulate a binary might require manual work if the HAL handlers are not implemented.

![image](https://github.com/unibuc-cs/river/blob/add_docs/docs/diagrams/halucinator.png)
Images are taken from HALucinator Youtube [presentation](https://www.youtube.com/watch?v=7mFqTjfLuEM&ab_channel=USENIX).

### P2IM & DICE

#### P2IM
Check the Github [repository](https://github.com/RiS3-Lab/p2im) or [paper](https://www.usenix.org/system/files/sec20-feng.pdf) to see details about P2IM.

P2IM is generating a peripheral model based on the classification of the registers accessed by the firmware during execution.

P2IM is launching the firmware and records the requests to read or write each registers. 
Based on that, P2IM is choosing a category for each register (categories are control, status, data, control-status). Thus, an I/O model for the firmware is created. 
P2IM can emulate the firmware using this model by feeding input as expected to each read or write call preventing crashes during execution.

Advantages of P2IM:

- does not require manual assistance to inferr a peripheral model (completely automated).
- offers full-emulation, no need for dedicated hardware.
- works for blob-firmware (i.e. firmware run on bare metal).

Disadvantages of P2IM:

- gives false-positive, 79% of the firmware (given in their repo) was emulated succesfully without human intervention. 
- does not have support for direct memory access (DMA) - this is fixed by [DICE](https://github.com/RiS3-Lab/DICE-DMA-Emulation).
- requires low level test cases (i.e. at the level of registers).
- According to HALucinator paper:
*"P2IM only considers sequences of
MMIO interactions as input; when a crash is found, this must
be mapped back to the external stimulus, requiring a deep
understanding of the external peripherals’ MMIO interface.
HLE-based approaches do not suffer from this problem, as
they work only with this external stimulus, and the inputs can
be readily replayed against real and virtualized targets alike."*

#### DICE
The following work of P2IM is DICE ([repository](https://github.com/RiS3-Lab/DICE-DMA-Emulation), [paper](https://arxiv.org/pdf/2007.01502.pdf))

DICE extends P2IM capabilities by adding support for DMA (i.e. can fuzz directly via memory).

### Pretender
The project can be found in the Github [repository](https://github.com/ucsb-seclab/pretender). 
Also, more details on Pretender concepts can be found in the [paper](https://www.usenix.org/system/files/raid2019-gustafson.pdf).

- Pretender uses ML/AI to model all of the MMIO accesses of the firmware.
- With the model obtained, one can emulate the firmware inside QEMU in *survivable execution* mode (i.e. the firmware is able to execute without the presence of the hardware without crashing or stalling). 
- Pretender is relying on Avatar (hardware-in-the-loop emulator) to obtain firmware models.
- The target firmware can be run inside QEMU or analyzed with `angr`.
- The advantage is that is the peripheral model extraction is fully automated.
- The disadvantage of Pretender is that it needs the actual hardware to train the model for a certain architecture/mcu/board/peripheral.  

## Fuzzers

### unicornafl
`unicornafl` is an extension of AFL++ with support for Unicorn. Check Github [repository](https://github.com/AFLplusplus/unicornafl).
 
TODO:
 1. Add notes on `unicornafl`
 2. Add diagrams. 

### hal-fuzz
Check Github [repository](https://github.com/ucsb-seclab/hal-fuzz).

- `hal-fuzz` extends HALucinator with fuzzing abilities.
- `hal-fuzz` is a fuzzer written for blob firmwares (executed with no underlying OS, specifically without MMU).

![image](https://github.com/unibuc-cs/river/blob/add_docs/docs/diagrams/HALucinator.jpg)

### afl-unicorn
`afl-unicorn` extends the AFL fuzzer. Check the Github [repository](https://github.com/Battelle/afl-unicorn) and the official [documentation](https://github.com/Battelle/afl-unicorn/blob/master/unicorn_mode/README.unicorn).

For an in-depth tutorial on how to fuzz with `afl-unicorn` check [Tutorial AFL-Unicorn - Part 1](https://medium.com/hackernoon/afl-unicorn-fuzzing-arbitrary-binary-code-563ca28936bf). 

To use alf-unicorn one must have/implement:

- Relevant binary code to be fuzzed.
- Knowledge of the memory map and a good starting state.
- Good initial test cases (i.e. seed corpus). 
- A Unicorn-based test harness which should have the following functionalities:
	- Loads binary code into memory.
	- Loads and verifies data to fuzz from a command-line specified file.
	- Adds memory map regions.
	- Sets up registers and memory state for beginning of test.
	- Emulates at least one instruction (see [docs](https://github.com/Battelle/afl-unicorn/blob/master/unicorn_mode/README.unicorn) for more details).
	- Emulates the interested code from beginning to end.
	- If a crash is detected, the test harness must 'crash' by throwing a signal (SIGSEGV, SIGKILL, SIGABORT, etc.).

![image](https://github.com/unibuc-cs/river/blob/add_docs/docs/diagrams/alf_unicorn.png)

Diagram taken from [Tutorial AFL-Unicorn - Part 1](https://medium.com/hackernoon/afl-unicorn-fuzzing-arbitrary-binary-code-563ca28936bf).

### TriforceAFL
Check the Github [repository](https://github.com/nccgroup/TriforceAFL) and the internal [documentation](https://github.com/nccgroup/TriforceAFL/blob/master/docs/triforce_internals.txt).

AFL already has built-in support for QEMU for *user-mode emulation*. TriforceAFL is exteding AFL to use *full system emulation*.

![image](https://github.com/unibuc-cs/river/blob/add_docs/docs/diagrams/triforce_afl.jpg)

- the guest OS is booting up and loads a **driver** to control the fuzzing life-cycle.
- the driver adds a special instruction `aflCall` that supports several operation:
	- `startForkserver` = spawns a forked copy of the virtual machine (VM) process that will execute one fuzzing iteration.
	- `getWork`  = reads an input from a file from the *host* OS and copy the contents of it into a buffer in the *guest* OS.
	- `startWork` = enables AFL tracing that marks the starting of the actual fuzzing iteration.
	- `endWork` = test case is completed. The VM fork exits with a certain code that is communicated back to AFL to determine the outcome of the test case (i.e. the binary crashed or not).
	- The VM can also end a test case when a panic is detected.

**Note:** The fuzzer runs in a forked copy of the VM (the in-memory state of the kernel for each test case is isolated). 

**Note:** To better isolate components during fuzzing, it is desirable to use an in-memory file system like Linux ramdisk image.

*Note:* To see changes added in TriforceAFL from the original AFL and QEMU, one can check the commits listed [here](https://github.com/nccgroup/TriforceAFL/blob/master/docs/triforce_internals.txt#L236).

The guest OS inside QEMU can communicate back and forth with the host machine/OS via virtio-vsock. [virtio-vsock](https://wiki.qemu.org/Features/VirtioVsock) is a host/guest communications char device. It allows applications in the guest and host to communicate.
