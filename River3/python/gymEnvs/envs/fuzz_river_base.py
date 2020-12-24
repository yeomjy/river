import os
import gym
import sys

# TODO: fix this hardcoded path !!!!!!!!!!!!!!!!
sys.path.append("/home/ciprian/Work/river/River3/python")


"""
Optimize things
see other params from paper
do symbolic exec with given prob when search stagnates
tensorflow agents !

# move other in different env
# write documentation
"""

import gym
from RiverTracer import RiverTracer as RiverTracer
from RiverOutputStats import RiverStatsTextual
import RiverUtils
import numpy as np
from gym import spaces

OBS_MAP_EDGESIZE = 256 # For 2d map of block addresses
OBS_MAX_PATH_LEN = 128 # Maximum path len to be returned
MAX_BBLOCK_HASHCODE = 64000 # Maximum size of a basic block hash code
OBS_PATH_EMBEDDING_SIZE = 128

class RiverBinaryFuzzerBase(gym.Env):
	def __init__(self, args):

		# TODO: how do we deal with this ?
		# Create two tracers : one symbolic used for detecting path constraints etc, and another one less heavy used only for tracing and scoring purpose
		self.tracer = RiverTracer(symbolized=True, architecture=args.architecture, maxInputSize=args.maxLen,
								  targetAddressToReach=args.targetAddress, resetSymbolicMemoryAtEachRun=True)
		self.stateful = args.stateful

		# All possible observations from last run
		# --------------------
		# The map hash
		self.obs_map = np.zeros(shape=(OBS_MAP_EDGESIZE, OBS_MAP_EDGESIZE))
		# The last path through the program - a list of basic block addresses from the program evaluation
		self.obs_path = None
		# Last run as above but in format {basic blocks : how many times}
		self.obs_path_stats = None
		# TODO: take it from the other side
		self.obs_embedding = None
		self.args = args
		#--------------------

		# Load the binary info into the given list of tracers. We do this strage API to load only once the binary...
		RiverTracer.loadBinary([self.tracer], args.binaryPath, args.entryfuncName)
		if args.outputType == "textual":
			outputStats = RiverStatsTextual()

		self.observation_space = spaces.Dict({'map' : spaces.Box(low=0, high=sys.maxsize, shape=(OBS_MAP_EDGESIZE, OBS_MAP_EDGESIZE)), #if args.obs_map else None,
								  				'path' : spaces.Box(low = 0, high=MAX_BBLOCK_HASHCODE, shape=(OBS_MAX_PATH_LEN,)), #if args.obs_path else None,

											   # blocks and their visit count during last run
											  'obs_path_stats' :  spaces.Tuple((spaces.Box(low = 0, high=MAX_BBLOCK_HASHCODE, shape=(OBS_MAX_PATH_LEN,)),
																				spaces.Box(low = 0, high=np.inf, shape=(OBS_MAX_PATH_LEN,)))),  #if args.obs_path_stats else None,
												'obs_embedding' : spaces.Box(low=0, high=256, shape=(OBS_PATH_EMBEDDING_SIZE,))  #if args.obs_embedding else None
								  				})

		# Action Index, Parameters
		self.action_space = spaces.Tuple((spaces.Discrete(RiverUtils.Input.getNumActionFunctors()),
										 spaces.Dict({'isSymbolic' : spaces.Discrete(2)})))

	def _runCurrentInput(self, isSymbolic=False):
		crashed = 0 # TODO: fix this
		lastPathConstraints = self.tracer.getLastRunPathConstraints()
		targetFound, numNewBlocks, allBBsInThisRun = self.tracer.runInput(self.input, symbolized=isSymbolic, countBBlocks=True)
		return numNewBlocks, crashed, lastPathConstraints, allBBsInThisRun

	def setCorpusSeed(self, path):
		initialInputCorpus = ["good"] 	# TODO: load from folder path
		self.corpus = RiverUtils.processSeedDict(initialInputCorpus)  # Transform the initial seed dict to bytes instead of chars if needed

	# You can override this to fill observation and still reuse step and reset function without overriding them
	# in your experiments
	def fill_observation(self):
		obs = {}
		obs['map'] = self.obs_map
		obs['path'] = self.obs_path
		obs['obs_path_stats'] = self.obs_path_stats
		obs['obs_embedding'] = self.obs_embedding
		return obs

	# Reset the program and put it in a new input from the corpus seed
	def reset(self):
		self.input : RiverUtils.Input = RiverUtils.Input() 	# Array of bytes
		self.input.usePlainBuffer = True
		self.input.buffer = [0] * self.args.maxLen # np.zeros(shape=(self.args.maxLen,), dtype=np.byte)
		self.tracer.ResetMem()
		self.tracer.resetPersistentState()

		obs = self.fill_observation()
		return obs

	def step(self, action):
		assert isinstance(action, tuple) and len(action) == 2  # Action should a tuple of (actionIndex, Params)
		actionIndex = action[0]
		params = action[1]
		assert isinstance(params, dict), "The params must be a dictionary"
		params['inputInstance'] = self.input # Put the address of the input buffer as a context parameter

		# Reset program state if stateless
		if not self.stateful:
			self.tracer.ResetMem()

		# First, apply the action given to the input buffer
		succeeded = self.input.applyAction(actionIndex, params)

		# Then give it to the fuzzer to run it
		numNewBlocks, crashed, lastPathConstraints, allBBsInThisRun = self._runCurrentInput(isSymbolic=params['isSymbolic'])

		# Return (obs, reward, done, info)
		obs = self.fill_observation()
		done = crashed
		info = {'lastPathConstraints' : lastPathConstraints,
				'allBBsFound' : allBBsInThisRun}
		return (obs, numNewBlocks, crashed, done, info)

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

	# Could also derive reset and step functions !
	# Reset the program and put it in a new input from the corpus seed
	def reset(self):
		return super(RiverBinaryCustomForLibPNGEnv, self).reset()

	def step(self, action):
		super(RiverBinaryCustomForLibPNGEnv, self).step(action)

