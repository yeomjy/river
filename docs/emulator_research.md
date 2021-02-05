# Contents

- [Overview](#overview)
	- [Full Fuzzing Solutions Comparison](#full-fuzzing-solutions-comparison)
	- [Emulators Comparison](#emulators-comparison)
- [QEMU vs. Unicorn](#qemu-vs-unicorn)
- [Triforce AFL](#triforce-afl)
- [P2IM & DICE](#p2im-dice)
	- [P2IM](#p2im)
	- [DICE](#dice)
- [afl-unicorn](#afl-unicorn)
- [HALucinator & hal-fuzz](#halucinator--hal-fuzz)
- [Pretender](#pretender)

# Overview 

## Full Fuzzing Solutions Comparison
 | Tool | No hardware required | Full System Emulation | Baseline Fuzzer | CPU Emulation | Peripherals Emulation | Dynamic Instrumentation | Automated Model Extraction |
 |-------------|-------------|-------------|-------------|-------------|-------------|-------------|-------------|
 | [HALucinator](https://github.com/embedded-sec/halucinator) | :heavy_check_mark: | :heavy_check_mark: | ➖* | QEMU | HALucinator (replacing HALs with handlers) | [Avatar<sup>2](https://github.com/avatartwo/avatar2); [angr](https://github.com/angr/angr) | ❌ |
 | [hall-fuzz](https://github.com/ucsb-seclab/hal-fuzz) | :heavy_check_mark: | :heavy_check_mark: | AFL** | Unicorn | HALucinator | Unicorn; [angr](https://github.com/angr/angr) | ❌ |
 | [afl-unicorn](https://github.com/Battelle/afl-unicorn) | :heavy_check_mark: | :heavy_check_mark: | AFL** | Unicorn | ➖ | Unicorn | ➖ |
 | [P2IM](https://github.com/RiS3-Lab/p2im) | :heavy_check_mark: | :heavy_check_mark: | ➖* | QEMU | P2IM (peripherals model based on registers recognition)| TODO | :heavy_check_mark: |
 | [Triforce AFL](https://github.com/nccgroup/TriforceAFL) | :heavy_check_mark: | :heavy_check_mark: | AFL** | QEMU | ➖ | TODO | ➖ |
 | [Pretender](https://github.com/ucsb-seclab/pretender) | ❌ | :heavy_check_mark: | ➖* | QEMU | Pretender (AI/ML techniques)| [Avatar<sup>2](https://github.com/avatartwo/avatar2) | :heavy_check_mark: |

\* A drop-in fuzzer (such as AFL) can be added.

\** AFL used in black-box fuzzer mode for non-instrumented binaries (i.e. for binaries that **cannot** be re-compiled with `afl-gcc`). See AFL [docs](https://github.com/google/AFL#6-fuzzing-binaries) and [Technical Details 12) Binary-only instrumentation](https://lcamtuf.coredump.cx/afl/technical_details.txt) for more details.

## Emulators Comparison

 | Emulator | Full System Emulation | Dynamic Instrumentation | Architectures supported | Communication Host - Guest OS|
 |-------------|-------------|-------------|-------------|-------------|
 | [Qiling](https://github.com/qilingframework/qiling) | :heavy_check_mark: | TODO | X86, X86_64, Arm, Arm64, MIPS, 8086 | TODO |
 | [Unicorn](https://www.unicorn-engine.org/) | :heavy_check_mark: | TODO | ARM, ARM64, M68K, MIPS, SPARC, X86 | TODO |
 | [QEMU](https://www.qemu.org/) | ❌ | injected hooks after basic blocks | ARM, x86, [etc](https://wiki.qemu.org/Documentation/Platforms) | [virtio-vsock](https://wiki.qemu.org/Features/VirtioVsock) |

# QEMU vs. Unicorn
Documentation for QEMU can be found [here](https://www.qemu.org/documentation/).

Documentation for Unicorn can be found [here](https://www.unicorn-engine.org/docs/). Unicorn is a framework for CPU emulation based on QEMU. Also, one can check an in-depth [presentation](https://www.unicorn-engine.org/BHUSA2015-unicorn.pdf) from 2015 about Unicorn.

An article provided by Unicorn devs describing the main differences between QEMU and Unicorn can be checked [here](https://www.unicorn-engine.org/docs/beyond_qemu.html).

**Note:** Unicorn provides a built-in layer of dynamic instrumentation vs. QEMU that has only static instrumentation. 

# Triforce AFL
All the code is publicly available in the Github [repository](https://github.com/nccgroup/TriforceAFL). Also, one can check internal [documentation](https://github.com/nccgroup/TriforceAFL/blob/master/docs/triforce_internals.txt).

AFL already has built-in support for QEMU *user-mode emulation*. TriforceAFL is exteding AFL to include QEMU *full system emulation*.

To see changes added in TriforceAFL from the original AFL and QEMU, one can check the commits listed [here](https://github.com/nccgroup/TriforceAFL/blob/master/docs/triforce_internals.txt#L236).

![image](https://github.com/unibuc-cs/river/blob/add_docs/docs/diagrams/triforce_afl.jpg)

- the guest OS is booting up and loads a **driver** to control the fuzzing life-cycle.
- the driver adds a special instruction `aflCall` that supports several operation:
	- `startForkserver` = "Every operation in the virtual machine after this call will run in a forked copy of the virtual machine that persists only until the end of a test case"
	- `getWork`  = reads an input from a file from the *host* OS and copy the contents of it into a buffer in the *guest* OS
	- `startWork` = enables AFL tracing
	- `endWork` = test case is completed. The VM fork exits with a certain code that is communicated back to AFL to determine the outcome of the test case.
	- The VM can also end a test case when a panic is detected.

**Note:** The fuzzer runs in a forked copy of the VM (the in-memory state of the kernel for each test case is isolated). 

**Note:** To better isolate components during fuzzing, it is desirable to use an in-memory file system like Linux ramdisk image.

The guest OS inside QEMU can communicate back and forth with the host machine/OS via virtio-vsock. [virtio-vsock](https://wiki.qemu.org/Features/VirtioVsock) is a host/guest communications char device. It allows applications in the guest and host to communicate.

# P2IM & DICE

## P2IM
Check the Github [repository](https://github.com/RiS3-Lab/p2im) or [paper]((https://www.usenix.org/system/files/sec20-feng.pdf)) to see details about the project.

P2IM is generating a peripheral model based on the classificarion of the registers accessed by the firmware.

P2IM is launching the firmware and watches the requests to read or write certain registers. Based on that, P2IM is choosing a category for each register (control, status, data, control-status). By feeding input as expected to each read / write call, the firmware will not crash during execution.

Advantages of P2IM:

- does not require manual assistance to inferr a peripheral model (completely automated).
- offers full-emulation, no need for hardware-in-the-loop.
- works for blob firmware.

Disadvantages of P2IM:

- gives false-positive, 79% of the firmware (given in thei repo) was emulated succesfully without human intervention. 
- does not have support for direct memory access (DMA). 
- requires low level test cases (i.e. at the level of registers).
- According to HALucinator paper:
"P2IM only considers sequences of
MMIO interactions as input; when a crash is found, this must
be mapped back to the external stimulus, requiring a deep
understanding of the external peripherals’ MMIO interface.
HLE-based approaches do not suffer from this problem, as
they work only with this external stimulus, and the inputs can
be readily replayed against real and virtualized targets alike."

## DICE
The following work of P2IM is DICE ([repository](https://github.com/RiS3-Lab/DICE-DMA-Emulation), [paper](https://arxiv.org/pdf/2007.01502.pdf))

DICE extends P2IM capabilities by adding support for DMA (i.e. can fuzz directly via memory).

# afl-unicorn
All the code is public. Check the Github [repository](https://github.com/Battelle/afl-unicorn) for in-depth details.

To write this small snippet of information, I used as reference the official [documentation](https://github.com/Battelle/afl-unicorn/blob/master/unicorn_mode/README.unicorn).

To use alf-unicorn one must have/implement:

- Relevant binary code to be fuzzed
- Knowledge of the memory map and good starting state
- Good initial seed corpus
- Unicorn-based test harness which should have the following functionalities:
	- Loads binary code into memory.
	- Loads and verifies data to fuzz from a command-line specified file.
	- Adds memory map regions.
	- Sets up registers and memory state for beginning of test.
	- Emulates at least one instruction (see [docs](https://github.com/Battelle/afl-unicorn/blob/master/unicorn_mode/README.unicorn) for more details).
	- Emulates the interested code from beginning to end.
	- If a crash is detected, the test harness must 'crash' by throwing a signal (SIGSEGV, SIGKILL, SIGABORT, etc.).

![image](https://github.com/unibuc-cs/river/blob/add_docs/docs/diagrams/alf_unicorn.png)

Diagram taken from [Tutorial AFL-Unicorn - Part 1](https://medium.com/hackernoon/afl-unicorn-fuzzing-arbitrary-binary-code-563ca28936bf).

# HALucinator & hal-fuzz
The code for HALucinator can be reviewed  in the Github [repository](https://github.com/embedded-sec/halucinator). `hal-fuzz` is a related project with a different [repository](https://github.com/ucsb-seclab/hal-fuzz). 

The details of the concepts can be read in their [paper](https://www.usenix.org/system/files/sec20-clements.pdf).

- `hal-fuzz` is meant for blob firmwares.
- the goal of HALucinator is to fully re-host embedded firmware.
- the hardest part of the re-hosting process are the models for peripherals (without a proper model for peripherals the firmware will not boot or will crash early in the execution process).
- *Hardware Abstraction Libraries* (HAL) are libraries provided by dev board vendors to ease the writing of the firmware code. Most of the HALs/SDK are publicly available which is a key advantage for HALucinator.
- HALucinator enables the process of replacing HALs with high level implementation function (i.e. handlers that are comunicating with the emulator are replacing the funtiona that were formerly coupled with the hardware).
- LibMatch is a tool used to locate the HALs function addresses in the firmware (by using binary analysis and the SDK provided by the vendors).
- For each new collection of HAL functions (or SDK) the developer should write his own replacement functions (HALucinator requires manual work).
	- Once the developer is translating a HAL library to handlers, the code can be re-used for every board that is using that specific HAL library. 
	- most of the high level implementation are trivial (returning constants or interacting with the model itself and returning a result).
	- a few of them are not trivial to implement (when the HAL library does not abstract the hardware well enough or the HAL library uses in-the-house elements like its own heap allocator).
![image](https://github.com/unibuc-cs/river/blob/add_docs/docs/diagrams/halucinator.png)
Images are taken from HALucinator Youtube [presentation](https://www.youtube.com/watch?v=7mFqTjfLuEM&ab_channel=USENIX).

![image](https://github.com/unibuc-cs/river/blob/add_docs/docs/diagrams/HALucinator.jpg)

# Pretender
The project can be found in the Github [repository](https://github.com/ucsb-seclab/pretender). Also, more details on Pretender concepts can be found in the [paper](https://www.usenix.org/system/files/raid2019-gustafson.pdf).

- Pretender uses ML/AI to model all of the MMIO accesses of the firmware.
- With the model obtained, one can emulate the firmware inside QEMU in *survivable execution* mode (i.e. the firmware is able to execute without the presence of the hardware without crashing or stalling). 
- Pretender is relying on Avatar (hardware-in-the-loop emulator) to obtain firmware models.
- The target firmware can be run inside QEMU or analyzed with `angr`.
- The advantage is that is the peripheral model extraction is fully automated.
- The disadvantage of Pretender is that it needs the actual hardware to train the model for a certain architecture/mcu/board/peripheral.  
