		/*
			CSC360 Assignment1
			Leanne Feng
			V00825004				
			
		The purpose of the assignment is to get familier with using system calls related to processes management and process control block.
		1. put makefile and Pman.c in the same file.
		2. Type in make in command or type gcc Pman.c -lreadline -lhistory -o Pman to compile Pman.c
		3. TYpe ./Pman 
		4. Type bg ./inf(any source file) , not entering anything after bg will give you an error.
		5. Type bglist to show the current processes.
		6. Type bgkill pid to terminate the pid you want, not entering anything after bgkill will give you an error.
		7. Type bgstop pid to stop a process , not entering anything after bgstop will give you an error.	
		8. Type bgstart pid to restart a process, not entering anything after bgstart will give you an error.
		9. Type pstat pid to list information of comm, state, utime, stime, rss, voluntary_ctxt_switches and nonvoluntary_ctxt_switches
		   Also use pstat to check if bgstop and bgstart by checking the status. S: Sleeping; T:Stop; R: Running
		
		*/
