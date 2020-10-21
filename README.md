Compile using gcc -o animal animal.c -pthread

Run using ./animal

What this program does is the following:

1. It initializes a best fit array which is our target. This is done with random numbers that are seeded using the current time, multiplied by some constants
which produce the max random value of 100000.

I chose this large number because I wanted a lot of variation in the numbers, and I wanted many chances of error so that I can develop the algorithm to be more efficient

2. It then initializes the population with random numbers, same way we initialized the best fit array
This is done using multithreading. 

I use one thread for each population to initialize them, and calculate their fitness using a comparison function. This function simply finds the absolute value difference between the best fit array and the population's array.

3. Then, utilizing a weighing function, I check to see which populations have the best value based on their errors. The weighing function then tells us a % that we should give to each population for a weighted random parent-choosing process. 

4. I then use multithreading with the weights to find all the new parents, and put them in the parent array.

5. This is done NUM_OF_GENS many times, each time using a simple mathmatical model to try to get closed to the best fit. 
