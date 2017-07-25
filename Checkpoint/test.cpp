#include <iostream>
#include <fstream>
#include "CR.h"

#define ITERATION 5

int main() {
	int i = 0, start = 0;
	std::vector<int> vec = {0, 1, 2, 3, 4};

	CR cr;
	CheckpointData chpt(i, vec);
	if (!cr.isEmpty()) {
		chpt = cr.recover(chpt);
		start = chpt.getNum();
		vec = chpt.getArr();
	}

	std::vector<int> :: iterator it;
	for (it = vec.begin(); it != vec.end(); ++it) {
		std::cout << *it << '\t';
	}

	for (i = start; i < ITERATION; i++) {
		int j;
		for (j = 0; j < 5; j++) {
			vec[j] = j + i;
		}

		CheckpointData chkpt(i, vec);
		cr.Checkpoint(chkpt);
		sleep(5);
	}
	return 0;
}
