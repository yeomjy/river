# Reinforcement learning environment for Fuzzing

## Installation:
- This folder includes the base definitions of a RL Environment for fuzzing binary using our Tool. You have to install first the River3 (https://github.com/unibuc-cs/river/tree/master/River3) then come back to this
- The environments are in gymEnvs and are compatible with the OpenAI Gym format !
  Currently we have two environments:
    - the base one, default: RiverBinaryFuzzerBase-v0  (used for general purpose fuzzing of binaries
    - a custom one for demonstrations of our environment extensibility: RiverBinaryCustomForLibPNGEnv

- By using a common OpenAI gym format you can either use your own algorithms to train the environments or use an external library of algorithms like we already did with TFAgents (https://www.tensorflow.org/agents). 
- Note: the code is not tied up to Tensorflow.

## Defaults in the base environment
  - Observations: a 2D bash of observed binary blocks (check self.obs_map), an embeedding of the last path used, a plain path of basic blocks
  - Rewards: number of different basic blocks found
  - No custom dictionary (all bytes range are allowed)
  - Actions functors (similar to the one used by LibFuzzer https://llvm.org/docs/LibFuzzer.html)
  - Put your own corpus in setCorpus function

## Extended environment as an example:
You can customize your own custom observations, actions, rewards and tokens dictionary !

  - RiverBinaryCustomForLibPNGEnv implemented in fuzz_river_example.png.py contains a demonstration how to extend the base environment.
  - Take a look at init function to see how a dictionary containing PNG tokens is added dynamically to the already existing dictionary.
  - Take a look at how we used registerNewActionFunctor function to register a new operation called 'Shuffle'. 

## Combining symbolic execution and fuzzing with RL

A very good example that succedded to almost double the rewards in short time in our experiments was the combination of symbolic execution and fuzzing.
The idea is this:
 - When for a couple of steps (parameter) is remaining the same:
     A. Do a symbolic execution to get the path constraints and branch points addresses along the path
     B. If there is a chance to get to a new block that was not seen yet:
        B1. Modify the input to get to that path at the next run
        
 - The implementation can be seen in testRiverGym.py script, inside testCustomPNGEnvironment function
        
### Note: check the parameters from River3 base first, since all things above are related to it.
