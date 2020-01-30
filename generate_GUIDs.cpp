// generates 100k GUIDs
// uses a statically allocated array on stack (might not work on lower-end machines)
// two functions are available to test is consecutive GUIDs are different or to display all GUIDs


#include <iostream>
#include <chrono>
#include <ctime>

using namespace std;

#define TEST_SIZE 100000

typedef unsigned long long uint64_t;

class Node {
	uint64_t nodeId;
	uint64_t extraCounter;

public:
	Node() : nodeId(0), extraCounter(0) {}
	Node(uint64_t id) : nodeId(id), extraCounter(0) {}

	uint64_t getNodeId() { return this->nodeId; }
	void setNodeId(uint64_t id) { this->nodeId = id; }

	//millisecondsSinceEpoch since epoch
	uint64_t getTimestamp() {
		uint64_t millisecondsSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count();

		return millisecondsSinceEpoch;
	}

	// 10 bits nodeId + 12 bits counter (for 4096 ids) + 42 bits timestamp (milliseconds)
	// in this test I generated ~3-4k IDs in a millisecond
	// with a more performant machine, you could get to over 4k in a ms and GUID generation would start to generate duplicates
	uint64_t getId() {
		uint64_t GUID = 0;
		GUID |= (nodeId << 54);
		GUID |= (extraCounter << 42);
		GUID |= this->getTimestamp();
		if (extraCounter == 4095)
			extraCounter = 0;
		else
			extraCounter++;
		return GUID;
	}
};

void decToBinary(uint64_t n);
void displayAdjacentIdsWhoAreEqual(uint64_t arr[], int arrSize);
void displayArray(uint64_t arr[], int arrSize);

int main()
{
    std::cout << "Hello World!\n"; 

	Node *node = new Node(256);

	cout << node->getNodeId()<<" "<<node->getTimestamp()<<endl;

	volatile int counter = 0;
	uint64_t arr[TEST_SIZE] = { 0 };

	auto start = std::chrono::system_clock::now();
	for (int i = 0; i < TEST_SIZE; i++) {
		arr[counter] = node->getId();
		counter++;

	}
	auto end = std::chrono::system_clock::now();
	cout << "counter is: " << counter << endl;

	std::chrono::duration<double> d = end - start;

	std::time_t startTime = std::chrono::system_clock::to_time_t(start);
	std::time_t endTime = std::chrono::system_clock::to_time_t(end);

	cout << endl;
	cout << "Started at: " << std::ctime(&startTime);
	cout << start.time_since_epoch().count() << endl;
	cout << "Ended at:   " << std::ctime(&endTime);
	cout << end.time_since_epoch().count() << endl;
	cout << endl;

	cout << "Duration: " << d.count() << "s" << endl;
	cout << endl;

	displayAdjacentIdsWhoAreEqual(arr, TEST_SIZE);
	//displayArray(arr, TEST_SIZE);
	
	delete node;
}

void displayAdjacentIdsWhoAreEqual(uint64_t arr[], int arrSize) {

	for (int i = 0; i < arrSize - 1; i++) {
		if (arr[i] == arr[i + 1]) {
			cout << "### " << i << "e duplicat!" << endl;
		}
	}
	cout << "done searching duplicates\n";
}

void displayArray(uint64_t arr[], int arrSize) {

	for (int i = 0; i < arrSize; i++) {
		cout << "arr[" << i << "]=";
		decToBinary(arr[i]);
		cout << endl;
	}
	cout << "done display" << endl;
}

// simple function to display a number in binary
void decToBinary(uint64_t n)
{
	// size of an integer is assumed to be 64 bits 
	for (int i = 63; i >= 0; i--) {
		uint64_t k = n >> i;
		if (k & 1)
			cout << "1";
		else
			cout << "0";
		if (i == 54)
			cout << " ";
		else if (i == 42)
			cout << " ";
		else if (i == 20)
			cout << " ";
	}
	cout << endl;
}
