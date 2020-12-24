import gymEnvs
import gym
import RiverUtils
from tqdm import tqdm
import numpy as np

if __name__ == "__main__":

	# Step 0: make environment
	args = RiverUtils.parseArgs()
	#r = RiverBinaryFuzzerBase(args)
	env = gym.make('RiverBinaryFuzzerBase-v0', args=args)

	folderPath="./corpus"
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
			action = (actionIndex, {'isSymbolic' : isSymbolic}) # Action index, context and parameters
			#print(f"Action applied: {action}")
			obs, reward, newObs, done, info = env.step(action)
			stepIndex += 1
			totalReward += reward

			if stepIndex % 20 == 0:
				print(f"Episode: {epIndex} step: {stepIndex} partial reward: {totalReward}")

		if epIndex % 1 == 0:
			print(f"Episode: {epIndex} totalReward:{totalReward}")

		done = done
