import os
import gym
import sys

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
import copy

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
		self.obs_map = np.zeros(shape=(OBS_MAP_EDGESIZE, OBS_MAP_EDGESIZE), dtype=np.int32) # Number of time each hashed block was found
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

	def resetTracerSymbolicState(self):
		self.tracer.resetLastRunPathConstraints()

	# This function will try to take the path constraints and modify the current input buffer in place
	# Such that at the next iteration if the same buffer is kept, it should to a new undisovered path yet
	# However, note that
	def symbolicTakeUntakenBranch(self, PathConstraints):
		# Get the astContext
		astCtxt = self.tracer.getAstContext()

		succeeded = False

		# This represents the current path constraint, dummy initialization
		currentPathConstraint = astCtxt.equal(astCtxt.bvtrue(), astCtxt.bvtrue())

		# Go through the path constraints from bound of the input (to prevent backtracking as described in the paper)
		PCLen = len(PathConstraints)
		for pcIndex in range(PCLen):
			pc = PathConstraints[pcIndex]

			# Get all branches
			branches = pc.getBranchConstraints()

			# If there is a condition on this path (not a direct jump), try to reverse it with a new input
			if pc.isMultipleBranches():
				takenAddress = pc.getTakenAddress()
				for branch in branches:
					# Get the constraint of the branch which has been not taken
					branchTargetAddress = branch['dstAddr']
					if  (branchTargetAddress!= takenAddress) and (branchTargetAddress not in self.tracer.allBlocksFound):
						# Check if we can change current executed path with the branch changed
						desiredConstrain = astCtxt.land([currentPathConstraint, branch['constraint']])
						changes = self.tracer.solveInputChangesForPath(desiredConstrain)

						if changes:
							self.input.applyChanges(changes)
							succeeded = True
							break

				if succeeded:
					break

			# Update the previous constraints with taken(true) branch to keep the same path initially taken
			currentPathConstraint = astCtxt.land([currentPathConstraint, pc.getTakenPredicate()])

		return succeeded

	def _runCurrentInput(self, isSymbolic=False):
		crashed = 0 # TODO: fix this
		targetFound, numNewBlocks, allBBsInThisRun = self.tracer.runInput(self.input, symbolized=isSymbolic, countBBlocks=True)
		lastPathConstraints = self.tracer.getLastRunPathConstraints()
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

	def updateObservation(self, info):
		lastPathConstraints = info['lastPathConstraints']
		allBBsInThisRun = info['allBBsFound']

		# Update only the 2D map for now, user is free to override and do its own observations here as he wants !
		for blockAddr in allBBsInThisRun:
			blockAddrInt = int(blockAddr)
			blockHashRow = (blockAddrInt // OBS_MAP_EDGESIZE) % OBS_MAP_EDGESIZE
			blockHashCol = blockAddrInt % OBS_MAP_EDGESIZE

			self.obs_map[blockHashRow][blockHashCol] += 1

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
		succeeded = False
		if actionIndex != RiverUtils.Input.NO_ACTION_INDEX:
			succeeded = self.input.applyAction(actionIndex, params)

		# Then give it to the fuzzer to run it if we applied the action
		if succeeded or actionIndex == RiverUtils.Input.NO_ACTION_INDEX:
			numNewBlocks, crashed, lastPathConstraints, allBBsInThisRun = self._runCurrentInput(isSymbolic=params['isSymbolic'])
		else:
			numNewBlocks, crashed, lastPathConstraints, allBBsInThisRun = 0, 0, [], []

		# Return (obs, reward, done, info)
		obs = self.fill_observation()
		done = crashed
		info = {'lastPathConstraints' : lastPathConstraints,
				'allBBsFound' : allBBsInThisRun}
		self.updateObservation(info)
		return (obs, numNewBlocks, crashed, done, info)

