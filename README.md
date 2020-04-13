# consumer-producer
<br/>
Implemented an algorithm that synchronizes between consumers/producers.<br/>
Each of the consumers/producers is represented by thread.<br/>
The input to the program is the number of consumers, the number of producers, the number of items to consume and the size of the buffer <br/>
Of course when one of the threads (producer or consumer) intends to write or read from the buffer the access is done atomically, and this is done with the help of locks and mutexes.<br/>
 
+ **ex3_q2.c** 
This file contains the implementations for the synchronization.<br/>
First, the semaphores and mutexes are initialized. Then all threads (consumers / producers) are created and by conditional variable waiting to wake and get started.<br/>
Once all the threads are booted and created, the program gets them started (by exec_prod, exec_cons functions).<br/>
The synchronization happens by two semaphores that each symbolize how many cells in the buffer are full/empty.<br/>In addition i used 3 mutexes:<br/>
1. Controls access to the buffer.<br/> 
2. Controls access to the stdout.<br/> 
3. Controls how many items are consumed in order for a thread to know if there is any point in waiting to take action.

+ **ex3_q2_items_operation.c**
This file contain calculations (such as check if number is prime, find_two_factors), handle messages to the screen (when consumer/producer take action, when item is added / read from the buffer.  

+ **ex3_q2.h**
Functions declarations for the file that handles synchronization.<br/>

+ **ex3_q2_items_operation.h**
Functions declarations for the file that handles production/consumers operations.<br/>


Programming Language: C.
OS: Linux.
