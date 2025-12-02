#pragma once
#include <vector>
#include <functional>
#include <algorithm>

class FrameSlicer {

public:

	unsigned int totalNumLoops;
	unsigned int numLoopsEveryIteration;

	unsigned int currentLoopIndex;

	std::function<void(unsigned int&)> loopFunction;

	FrameSlicer(unsigned int _totalNumLoops, unsigned int _numLoopsEveryIteration, std::function<void(unsigned int&)> _loopFunction) {
		totalNumLoops = _totalNumLoops;
		numLoopsEveryIteration = _numLoopsEveryIteration;
		loopFunction = _loopFunction;

		currentLoopIndex = 0;
	}

	void RunSlicer() {
		unsigned int currentLimit = std::min(currentLoopIndex + numLoopsEveryIteration, totalNumLoops);

		for (unsigned int i = currentLoopIndex; i < currentLimit; i++)
		{
			loopFunction(i);
		}

		currentLoopIndex = currentLimit;
	}

	bool Done() {
		return currentLoopIndex >= totalNumLoops;
	}
};