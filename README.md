River: fuzz testing for binaries using AI
=========

River is an [open-source framework](https://github.com/unibuc-cs/river) that uses AI to guide the fuzz testing of binary programs.

# Contents

- [Intro](#intro)
	- [River 3.0 Architecture](#river-30-architecture)
	- [Scientific Publications](#scientific-publications)
- [Installation](#installation)
- [Testing](#testing)
	- [How to build your program for testing with River?](#how-to-build-your-program-for-testing-with-river)
	- [How to use the Concolic (SAGE-like) tool?](how-to-use-the-concolic-sage-like-tool)
	- [How to use the Reinforcement-Learning-based Concolic tool?](#how-to-use-the-reinforcement-learning-based-concolic-tool)
	- [Testing with docker](#testing-with-docker)
- [Future Work](#future-work)

# Intro

## River 3.0 Architecture 
The architecture of River 3.0 is given below:

![RIVER 3.0 architecture](River3/River3-architecture.png?raw=true "RIVER 3.0 architecture")

- We are going to have two backends:
 0. LLVM based tools and Libfuzzer
 1. Binary execution using Triton and LIEF 

Currently, we have 1 (binary execution) functional, while 0 (LLVM) is work in progress.
- It is **cross-platform running on all common OSs and support architectures: x86, x64, ARM32, ARM64**
- River 3.0 will be a collection of tools that runs over the backend mentioned above. 

## Scientific Publications
River is developed in the Department of Computer Science, University of Bucharest. [Ciprian Paduraru](mailto:ciprian.paduraru@fmi.unibuc.ro) is the lead developer. 

Scientific publications related to River can be found below:
- C. Paduraru, R. Cristea, A. Stefanescu. [Automatic Fuzz Testing and Tuning Tools for Software Blueprints](https://www.dropbox.com/scl/fi/2kbmmquzbm8tdwd3t0yd0/icsoft23-blueprints.pdf?rlkey=gn1w605jhucubm050z7l72k2u&dl=1). In Proc. of 18th Int. Conf on Software Technologies (ICSOFT'23), vol. 1, pp. 151-162, SciTePress, 2023. (best paper award)
- C. Paduraru, M. Paduraru, A. Stefanescu. [RiverGame - a game testing tool using artificial intelligence](https://dl.dropbox.com/s/34rnxuthq6f0lgz/icst22.pdf). In Proc. of 15th IEEE Conference on Software Testing, Verification and Validation (ICST'22), pp. 422-432, IEEE, 2022.
- C. Paduraru, R. Cristea, E. Staniloiu. [RiverIoT - a Framework Proposal for Fuzzing IoT Applications](https://dl.dropbox.com/s/jduj2g1xe3bv4hb/serp4iot.pdf). In Proc. of 3rd Int. Workshop on Software Engineering Research & Practices for the Internet of Things (SERP4IoT'21), co-located with ICSE'21, pp. 52-58, IEEE, 2021
- C. Paduraru, M. Paduraru, A. Stefanescu. [RiverFuzzRL — An Open-Source Tool to Experiment with Reinforcement Learning for Fuzzing](https://www.dropbox.com/s/p5afaw4vi4pxt5s/icst21.pdf?dl=1). In Proc. of 14th IEEE Conference on Software Testing, Verification and Validation (ICST'21), pp. 430-435, IEEE, 2021.
- E. Staniloiu, R. Cristea, B. Ghimis. [IoT Fuzzing using AGAPIA and the River Framework](https://dl.dropbox.com/s/fvxuvyaw7237nb1/icsoft21.pdf). In Proc. of 16th Int. Conf. on Software Technologies (ICSOFT'21), pp. 324-332, SciTePress, 2021
- B. Ghimis, M. Paduraru, A. Stefanescu. [RIVER 2.0: An Open-Source Testing Framework using AI Techniques](https://www.dropbox.com/s/cxu28e7nmsuocx6/langeti20-river.pdf?dl=1). In Proc. of LANGETI'20, workshop affiliated to ESEC/FSE'20, pp. 13-18, ACM, 2020.
- C. Paduraru, M. Paduraru, A. Stefanescu. [Optimizing decision making in concolic execution using reinforcement learning](https://www.dropbox.com/s/v2kv3g4y1h0cmsg/amost20.pdf?dl=1). In Proc. of A-MOST'20, workshop affiliated to ICST'20, pp. 52-61, IEEE, 2020.
- C. Paduraru, B. Ghimis, A, Stefanescu. [RiverConc: An Open-source Concolic Execution Engine for x86 Binaries](https://www.dropbox.com/s/a5qnbazsywd26if/icsoft20-river.pdf?dl=1). In Proc. of 15th Int. Conf on Software Technologies (ICSOFT'20), pp. 529-536, SciTePress, 2020.
- C. Paduraru, M. Melemciuc, B. Ghimis. Fuzz Testing with Dynamic Taint Analysis based Tools for Faster Code Coverage. In Proc. of 14th Int. Conf on Software Technologies (ICSOFT'19), pp. 82-93, SciTePress, 2019.
- C. Paduraru, M. Melemciuc. An Automatic Test Data Generation Tool using Machine Learning. In Proc. of 13th Int. Conf on Software Technologies (ICSOFT'18), pp. pp. 506-515, SciTePress, 2018.
- C. Paduraru, M. Melemciuc, M. Paduraru. Automatic Test Data Generation for a Given Set of Applications Using Recurrent Neural Networks. Springer CCIS series, vol. 1077, pp. 307-326, Springer, 2018.
- C. Paduraru, M. Melemciuc, A. Stefanescu. [A distributed implementation using Apache Spark of a genetic algorithm applied to test data generation](https://www.dropbox.com/s/vuak2gh590j9tdb/pdeim17.pdf?dl=1). In Proc. of PDEIM'17, workshop of GECCO’17, GECCO Companion, pp. 1857-1863, ACM, 2017.
- T. Stoenescu, A. Stefanescu, S. Predut, F. Ipate. [Binary Analysis based on Symbolic Execution and Reversible x86 Instructions](https://www.dropbox.com/s/qs8xbpvym6y80xl/fundamenta17.pdf?dl=1), Fundamenta Informaticae 153 (1-2), pp. 105-124, 2017.
- T. Stoenescu, A. Stefanescu, S. Predut, F. Ipate. [RIVER: A Binary Analysis Framework using Symbolic Execution and Reversible x86 Instructions](https://www.dropbox.com/s/goidvl0dqavuy77/fm16.pdf?dl=1). In Proc. of 21st International Symposium on Formal Methods (FM'16), LNCS 9995, pp. 779-785, Springer, 2016.

Presentations:
- A-MOST'20 (ICST workshop): [[slides](https://dl.dropbox.com/s/sz09gxagwb0dhq6/a-most-river.pdf)], [[video presentation](https://youtu.be/xfomXR3MN94)]


# Installation

1. Clone this repo with --recursive option, since we have some external submodules.
```
git clone --recursive https://github.com/unibuc-cs/river
```

2. Build Triton as documented in `River3/ExternalTools/Triton/README.md` or use the guidelines from the project's
[documentation](https://github.com/JonathanSalwan/Triton). 

3. Install LIEF with `pip install lief` or build LIEF from `River3/ExternalTools/LIEF` according to the project's
[documentation](https://github.com/lief-project/LIEF).

4. Install `numpy` with `pip install numpy`.

5. Install `tensorflow` with `pip install tensorflow`.

To check if the dependencies are installed correctly, run the following commands in a Python console
(tested on Python 3.7):

```
$ python3
>>> import triton
>>> import lief
>>> import numpy
```

# Core tool Testing

Currently we have implemented as a proof of concept a Generic Concolic Executor and Generational Search (SAGE), the first open-source version (see the original paper [here](https://patricegodefroid.github.io/public_psfiles/cacm2012.pdf)) that can be found in `River3/python`.

The `River3/python/concolic_GenerationalSearch2.py` is the recommended one to use for performance.

You can test it against the programs inside `River3/TestPrograms` (e.g. the crackme_xor, sage, sage2).
Feel free to modify them or experiment with your own code.

## How to build your program for testing with River?
`gcc` can be used to build the test binaries for River. 
The command below is compiling one of the examples provided in 'River3/TestPrograms': 
```
$ gcc -g -O0 -o crackme_xor crackme_xor.c
```

**NOTE:** The flags -g and -O0 are optional. 
They can be used for debugging and understanding the asm code without optimizations.


# How to use the Concolic (SAGE-like) tool?

In order to see the parameters supported by the script and how to use them, you can run:
```
$ cd path/to/river/River3/python
$ python3 concolic_GenerationalSearch2.py -h
```

To run `River3/python/concolic_GenerationalSearch2.py` for the binary `River3/TestPrograms/crackme_xor` navigate to 
`River3/python` and run the command below:
```
$ cd path/to/river/River3/python
$ python3 concolic_GenerationalSearch2.py \
	--binaryPath ../TestPrograms/crackme_xor \
	--architecture x64 \
	--maxLen 5 \
	--targetAddress 0x11d3 \
	--logLevel CRITICAL \
	--secondsBetweenStats 10 \
	--outputType textual
``` 

`--architecture` can be set to x64, x86, ARM32, ARM64.

`--targetAddress` parameter is optional. It is for "capture the flag"-like kind of things, where you want to get to a certain address in the binary code.
If you have access to the source code, you can use the following command to get the address of interest. The execution will stop when the target is reached, otherwise it will exhaustively try to search all inputs.
```
$ objdump -M intel -S ./crackme_xor
```
`--logLevel` is working with the `logging` module in Python to show logs, basically you put here the level you want to see output. Put DEBUG if you want to see everything outputed as log for example.

`--secondsBetweenStats` is the time in seconds to show various stats between runs.

---
**NOTE**

By default, `concolic_GenerationalSearch2.py` will search for a function named `RIVERTestOneInput` to use as a testing entrypoint.
If one desires to change the entrypoint, he can use the `--entryfuncName` option.
The example bellow sets `main` as the entrypoint:
```
--entryfuncName "main"
```
---

# How to use Reinforcement-Learning for general purpose fuzzing combined with symbolic execution ? 
Check the folder inside this branch: https://github.com/unibuc-cs/river/tree/master/River3/FuzzRL

# How to use the Reinforcement-Learning-based Concolic tool?

**NOTE:** Our implementation is done using Tensorflow 2 (2.3 version was tested). 
You can manually modify the parameters of the model from `River3/python/RLConcolicModelTf.py`.

The command below is an example on how to run `River3/python/concolic_RLGenerationalSearch.py` 
for the binary `River3/TestPrograms/crackme_xor`:
```
$ python3 concolic_RLGenerationalSearch.py \
	--binaryPath ../TestPrograms/crackme_xor \
	--architecture x64 \
	--maxLen 5 \
	--targetAddress 0x11d3 \
	--logLevel CRITICAL \
	--secondsBetweenStats 10 \
	--outputType textual
``` 

The parameters have the same description as above. 

# Testing using `docker`

We also provide a `Dockerfile` that builds an image with all the required dependencies and the latest `river` flavour.
The `Dockerfile` can be found in the `docker/` folder.
Inside the `docker/` folder are two files:

* The `Dockerfile` that builds the image
* A `Makefile` for convenience

To build the image navigate to the `docker/` directory and run `make`:
```
$ cd path/to/river/docker/
$ make
```

This will build the docker image locally.
View the available images with:
```
$ docker image ls
```

To start a self-destructing container that runs a simple test, run:
```
$ make test
```

This will test both `concolic_GenerationalSearch2.py` and `concolic_RLGenerationalSearch.py`.
The test for `concolic_RLGenerationalSearch.py` will print a bunch of warnings.
This is expected, as `tensorflow` is just warning about the fact that it didn't find the CUDA libraries, so it wont be able to run on a GPU and it will use the CPU instead.

To start a long running container with an interactive `/bin/bash` session, run:
```
$ make run
```

To connect to an existing, long running container, run:
```
$ make bash
```

To delete the container and local image crated, run:
```
$ make clean
```

# Performance aspects:
 - If you know the limit of your input put it as maxLen. We'll optimize things behind based on this. Putting a much larger value that needed is demetrial to performance
 - If your input is huge and can be constantly changed in large chuncks (e.g. shifting, adding, erasing bytes in the middle), use option usePlainBuffer=True when declaring your buffer ! By default it is false, which means that we consider sparse changes over existing input but there are cases such as image inputs or text when the input is not sparse. 
 TODO: should we change default to True ??
 
 - Work in progress: Currently we have a performance loss because we symbolize the memory at each step but this is not really needed. We could symbolize the memory variables once on the input then keep it like this, only changing memory references. I've opened a github issue here because Triton doesn't allow us to do this, check it here: https://github.com/JonathanSalwan/Triton/issues/976
 
# Debugging aspects:
 - Sometimes it is usefull to see which variables from the input are symbolized and which are alive. Check Tracer.debugShowAllSymbolicVariables function in RiverTracer.py for this purpose.

# Future Work
 - Output graphics
 - Corpus folder like in libfuzzer
 - Group of interesting inputs in folders
 - RL implementation
 - Parallelization
 - We are going to convert the evaluation API to Google FuzzBench and Lava kind of benchmarks

P.S. Please [get in touch](mailto:ciprian.paduraru@fmi.unibuc.ro) if you would like to collaborate on this exciting and humble project! ^_^
