Compile using gcc -o animal animal.c -pthread -lpthread -lrt

Run using ./animal

What this program does is the following:

1. It initializes a best fit array which is our target. This is done with random numbers. 

Our random number generation is done using the current time.

2. It then initializes the population with random numbers, same way we initialized the best fit array

3. It then computed the parents of the population, and recalculates the population using a simple averging function 
that I will be changing later. 

4. Repeats for hoever many generations you say.

5. Returns avg error each generation
