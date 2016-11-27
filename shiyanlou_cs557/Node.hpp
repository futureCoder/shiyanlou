#ifndef _NODE_HPP_
#define _NODE_HPP_

#include "Random.hpp"
constexpr RANDOM_PARAMETER 100;
struct Node {
	int arrive_time;
	int duration;
	Node *next;
	Node(int arrive_time = 0, 
			int duration = Random::uniform(RANDOM_PARAMETER):
			arrive_time(arrive_time), 
			duration(duration), 
			next(NULL)) {}
};

#endif
