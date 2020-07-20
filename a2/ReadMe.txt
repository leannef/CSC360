/*
			CSC360 Assignment2
			Leanne Feng
			V00825004				
			
The purpose of the assignment is to construct a simulator of a router to schedule the transmission of multiple trafic flows.  The scheduler is called Multi-Flow Scheduler (MFS).

Files included:
	-MFS.c
	-makefile
	-ReadMe
	-CSC360 Assignment2.pdf
		1. put makefile and MFS.c in the same file.
		2. Type in make in command  to compile MFS.c
		3. TYpe ./MFS input.txt   (where input.txt is the file you want to test the program with)
		

MFS.c:
	
	2.getmyTime() calculates the total elapsing time for a flow
	3.requestPipe(flow *item) if a flow arrives first and queue is empty then it will immediately  run and be the transmitting flow and then signal otheer flows to continue otherwise the the other flows will be added into queueList and a comparing method will be used to order the flows and identify which flow will be transmitted next. If there exists a flow in transmitting other flows will all be waiting until one of the flow (the top flow in queue) is signaled. Then the top flow in queueList will be the current transmitting flow.

	4. releasePipe(flow *item) signal the thread to begin

	5. *thrFunction(void *flowItem) print out the arrivaltime, transmitting time, id and priority for flows and call method requestPipe*() and releasePipe().

	6. main(int argc, char* argv[]) opens the input file provided in command line and extract the number of flows. Then creates a thread running the *thrFunction() function for each flow and waits for it to terminate.
	
*/
