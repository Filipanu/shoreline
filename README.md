# shoreline
GUID generation problem

# implementation
- The solution consists of a C++ class which acts as a "node" from the problem
- properties: unsigned long long nodeId
- methods: getNodeId(), getTimestamp(), getId() 
  
The getId() method returns a 64 bit unsigned integer
 - first 10 bits consist of the nodeId (1024 nodes -> 2^10 -> 10 bits; in this way, different nodes can generate IDs at the same time)
 - next 12 bits are from an internal counter (resets to 0 when 4095 is reached)
 - 42 bits timestamp in milliseconds
    - the time in milliseconds returned by the chrono lib in c++ returns a number with the 41'st least significant bit occupied (take this to 42 to keep more dates in the future)
    - current time since epoch is 1,580,402,489,576 in milliseconds
    - ‭01 0110 1111 1111 0111 0101 0100 0011 1000 1110 1000‬
    - ‭16F F754 38E8‬ -> 42 bits
    
I had another solution with 10 bits nodeId + 54 bits timestamp in microseconds(). However, even though this way there were fewer operations to do (no more increment + check if internal counter=4095), the actual performance over 1 million generations wasn't different.
Because the solution presented first was more reliable (guaranteed different consecutive GUIDs) and it generated 1 million GUIDs in [0.33; 0.37]s it was kept.

# tests
## Generate GUIDs test

- Generates 100k GUIDs in [0.028; 0.030] seconds (array allocated on stack)
- Generates 1m GUIDs in [0.33; 0.37] seconds (array was allocated on the heap, takes a little longer to access)

## Client-Server test

- Servers waits for messages. If he receives a "getID" it generated a GUID and sends it back. If he receives "STOP" he closes connection and stops.

- Sent 100k "getID" requests from client -> [4.20; 4.42] seconds
- The goal is not met, but this may be because my laptop isn't a high-end one and because the solution is single-threaded

# asked questions
## Please describe your solution to get_id and why it is correct i.e. guaranteed globally unique
- because in the GUID construction i take into account the nodeID, so even if all nodes generate an id at the exact same time it will be different 
- because i use the timestamp, which makes a GUID composition unique for the first 42 least significant bites (every millisecond)
- because i use 12 bits (4096) for an internal counter which increments after each generation (consecutive generations are guaranteed unique)

## Please explain how your solution achieves the desired performance i.e. 100,000 or more requests per second per node.  How did you verify this?
- to statisfy the 100k generations per second requirement i need 1000 unique GUIDs per millisecond, I can have up to 4096 because of the internal counter
- the generate GUIDs test program manages to generate ~3-4 million GUIDs per second
- the client-server test does not meet this requirement (~20-25k), however, i only tested in a single threaded environment

## How do you manage uniqueness after a node crashes and restarts?  
- assuming the node will retain it's unique id (or be provided an unused id) I think it's impossible for a system to recover from a crash in under a millisecond, thus the nodes which will be generated next will be unique because of the timestamp

## How do you manage uniqueness after the entire system fails and restarts?
- as stated above, because a system will take more than one millisecond to restart, the GUIDs will be unique because of the timestamp
