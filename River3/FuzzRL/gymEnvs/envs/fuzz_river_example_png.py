import os
import gym
import sys
from . fuzz_river_base import *
import random

"""
Optimize things - see github issue on Triton
see other params from paper
do symbolic exec with given prob when search stagnates
tensorflow agents !
# write documentation
"""

import gym
from RiverTracer import RiverTracer as RiverTracer
from RiverOutputStats import RiverStatsTextual
import RiverUtils
import numpy as np
from gym import spaces


class RiverBinaryCustomForLibPNGEnv(RiverBinaryFuzzerBase):
	def __init__(self, args):
		# Set a custom tokens dictionary
		RiverUtils.Input.tokensDictionary = \
				[	b"\x89PNG\x0d\x0a\x1a\x0a",
					b"IDAT",
					b"IEND",
					b"IHDR",
					b"PLTE",
					b"bKGD",
					b"cHRM",
					b"fRAc",
					b"gAMA",
					b"gIFg",
					b"gIFt",
					b"gIFx",
					b"hIST",
					b"iCCP",
					b"iTXt",
					b"oFFs",
					b"pCAL",
					b"pHYs",
					b"sBIT",
					b"sCAL",
					b"sPLT",
					b"sRGB",
					b"sTER",
					b"tEXt",
					b"tIME",
					b"tRNS",
					b"zTXt"
				]

		# Example how to register a new action functor
		RiverUtils.Input.registerNewActionFunctor(RiverBinaryCustomForLibPNGEnv.shuffleBytesFunctor)

		# Register a map observation by default here, or do your custom one
		# Note that if add a custom observation type, you also need to go back to fill_observation and override it to match your expectations
		args.obs_map = 1
		args.obs_path_stats = 0

		# Then call main functionality
		super(RiverBinaryCustomForLibPNGEnv, self).__init__(args)

	@staticmethod
	def shuffleBytesFunctor(params):
		# You get the input instance and control its buffer in the params
		# Your agent can theoretically control params very detailed if he wants.
		inputInstance = params['inputInstance']
		currentInputLen = len(inputInstance.buffer)
		if currentInputLen == 0:
			return False

		indexStart = params['indexStart'] if 'index' in params else None  # Index where to do the change
		numBytes = params['numBytes'] if 'value' in params else None  # value to change with, can be none and a random will be added there

		if numBytes is None:
			randomPercent = float(np.random.rand() * 1.0)  # A maximum of 1 percent to add
			randomNumItems = float(randomPercent / 100.0) * len(inputInstance.buffer)
			numBytes = int(max(randomNumItems, np.random.randint(low=3, high=10)))

		if indexStart is None:
			indexStart = np.random.choice(currentInputLen)

		indexEnd = min(indexStart + numBytes, currentInputLen)
		sliceValues = inputInstance.buffer[indexStart:indexEnd]
		random.shuffle(sliceValues)

		inputInstance.buffer[indexStart:indexEnd] = sliceValues


	# Could also derive reset and step functions !
	# Reset the program and put it in a new input from the corpus seed
	def reset(self):
		return super(RiverBinaryCustomForLibPNGEnv, self).reset()

	def step(self, action):
		return super(RiverBinaryCustomForLibPNGEnv, self).step(action)

if __name__ == "__main__":
	args = RiverUtils.parseArgs()
	test = RiverBinaryCustomForLibPNGEnv(args)
