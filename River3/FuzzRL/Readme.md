# Reinforcement learning environment for Fuzzing

## Installation:
- This folder includes the base definitions of a RL Environment for fuzzing binary using our Tool. You have to install first the River3 then come back to this
- The environments are in gymEnvs and are compatible with the OpenAI Gym format !
  Currently we have two environments:
    - the base one, default: RiverBinaryFuzzerBase-v0  (used for general purpose fuzzing of binaries
    - a custom one for demonstrations of our environment extensibility: RiverBinaryCustomForLibPNGEnv

- By using a common OpenAI gym format you can either use your own algorithms to train the environments or use an external library of algorithms like we already did with TFAgents (https://www.tensorflow.org/agents). 
- Note: the code is not tied up to Tensorflow.
