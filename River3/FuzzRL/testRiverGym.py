import gymEnvs
import gym
import sys

sys.path.append("../python")

import RiverUtils
from tqdm import tqdm
import numpy as np

def testBaseEnvironment():
	# Step 0: make environment
	args = RiverUtils.parseArgs()
	# r = RiverBinaryFuzzerBase(args)
	env = gym.make('RiverBinaryFuzzerBase-v0', args=args)

	folderPath = "./corpus"
	env.setCorpusSeed(folderPath)

	# Random policy action for testing purposes
	done = False
	numEpisodes = 10
	numMaxStepsPerEpisode = 100
	for epIndex in tqdm(range(numEpisodes)):
		obs = env.reset()
		stepIndex = 0
		totalReward = 0
		while not done and stepIndex < numMaxStepsPerEpisode:
			# Choose an action
			isSymbolic = False
			if np.random.rand() < 0.25:
				isSymbolic = True
			actionIndex = np.random.choice(len(RiverUtils.Input.actionFunctors))
			action = (actionIndex, {'isSymbolic': isSymbolic})  # Action index, context and parameters
			# print(f"Action applied: {action}")
			obs, reward, newObs, done, info = env.step(action)
			stepIndex += 1
			totalReward += reward

			if stepIndex % 20 == 0:
				print(f"Episode: {epIndex} step: {stepIndex} partial reward: {totalReward}")

		if epIndex % 1 == 0:
			print(f"Episode: {epIndex} totalReward:{totalReward}")

		done = done

def testCustomPNGEnvironment():
	# Step 0: make environment
	args = RiverUtils.parseArgs()
	# r = RiverBinaryFuzzerBase(args)
	env = gym.make('RiverBinaryCustomForLibPNGEnv-v0', args=args)

	folderPath = "./corpus"
	env.setCorpusSeed(folderPath)

	# Random policy action for testing purposes
	done = False
	numEpisodes = 10
	numMaxStepsPerEpisode = 100
	isInputAlreadyModifiedBySolver = False # If true, it means that SMT solver changed the input buffer so skip one step from your action ideally !

	for epIndex in tqdm(range(numEpisodes)):
		obs = env.reset()
		stepIndex = 0
		totalReward = 0

		# Keep counter if the reward has modified or not in the last T steps
		prev_reward = 0
		num_stepsSameReward = 0
		stepsSameReward_thresholdForSymbolic = 2 # At how many steps to apply symbolic stuff if things stagnate

		while not done and stepIndex < numMaxStepsPerEpisode:
			# Choose an action
			isSymbolicStep = False

			# Check if blocked in local optima
			if num_stepsSameReward >= stepsSameReward_thresholdForSymbolic:
				# Apply symbolic tracing ?
				if np.random.rand() < 0.5:
					#print(f"Applying a symbolic step at step {stepIndex}!")
					num_stepsSameReward = 0
					isSymbolicStep = True

			# If solver modified this last frame do not take any action in this step
			if isInputAlreadyModifiedBySolver == True:
				actionIndex = RiverUtils.Input.NO_ACTION_INDEX
				isInputAlreadyModifiedBySolver = False
			else:
				actionIndex = np.random.choice(len(RiverUtils.Input.actionFunctors))

			action = (actionIndex, {'isSymbolic': isSymbolicStep})  # Action index, context and parameters
			#print(f"Action applied: {action}")
			obs, reward, newObs, done, info = env.step(action)
			stepIndex += 1
			totalReward += reward

			# If a symbolic step was used, modify randomly one of the blocks to get a new unexplored branch, or maybe a low probability one
			if isSymbolicStep:
				# Take the last path constraints from the symbolic execution step
				pathConstraints = info['lastPathConstraints']

				# Modify the input buffer to take a different bramnch, will return True if found
				succeeded = env.symbolicTakeUntakenBranch(pathConstraints)

				if succeeded:
					isInputAlreadyModifiedBySolver = True


				# Very important to call this to reset the last execution state of the symbolic tracer in the end !
				# OTHERWISE CONDITIONS WILL PROPAGATE BETWEEN RUNS !
				env.resetTracerSymbolicState()

			if prev_reward == totalReward:
				num_stepsSameReward += 1

			if stepIndex % 20 == 0:
				print(f"Episode: {epIndex} step: {stepIndex} partial reward: {totalReward}")

			prev_reward = totalReward

		if epIndex % 1 == 0:
			print(f"Episode: {epIndex} totalReward:{totalReward}")

		done = done



if __name__ == "__main__":
	#testBaseEnvironment()
	testCustomPNGEnvironment()
