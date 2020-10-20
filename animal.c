#define _GNU_SOURCE
#define NUM_OF_POPULATION 10
#define NUM_OF_THREADS (NUM_OF_POPULATION * NUM_OF_POPULATION)
#define NUM_OF_FITNESS_INDICES 10

#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>

/*

start with X threads

each thread randomly fills an array with numbers
There will be an array of best fit
array of arrays of best 


have compariiison functio return 0 to 1

for each child, choose two parents using the wieghted random fitness

for each child randomly mutate % of indices to mutate

parents weight over the sum of all the weights


*/
typedef struct
{
    double fitness[NUM_OF_FITNESS_INDICES];
    pthread_t individualNumber;

} individual;

void randomNumArr(double *arr);
void *initializePopulation(void *person);
void printArr(double arr[]);

individual *population[NUM_OF_POPULATION]; //big number, used to filter to small population
int populationIndex = 0;

individual *childPopulation[NUM_OF_THREADS]; //small number, the population that will be doing the reproduction
int childPopulationIndex = 0;

pthread_mutex_t mutex; //mutex used for writing to shared childPopulation array
double bestFit[NUM_OF_FITNESS_INDICES];

int main()
{
    randomNumArr(bestFit); //generates what we will be regarding the best fit array

    pthread_t threadNums[NUM_OF_POPULATION];

    for (int i = 0; i < NUM_OF_POPULATION; i++)
    {
        population[i] = (individual *)malloc(sizeof(individual)); //allocate mem for each population

        pthread_create(&threadNums[i], NULL, initializePopulation, (void *)population[i]);
    }

    //joins all the threads
    for (int i = 0; i < NUM_OF_POPULATION; i++)
    {
        pthread_join(threadNums[i], NULL);
    }
}

/*
Calls all functions for each thread
*/
void *initializePopulation(void *person)
{

    // //initialize the indiivdual
    individual *temp = (individual *)person; //MAKE SURE TO FREE THIS
    randomNumArr(temp->fitness);

    pthread_exit(0);
}

//initializes the array with pseudo random doubles
void randomNumArr(double *arr)
{

    //psuedo random seed with time
    struct timeval time;
    gettimeofday(&time, NULL);

    int t = time.tv_usec; //random math to maybe make it more chaotic?

    for (int i = 0; i < NUM_OF_FITNESS_INDICES; i++)
    {
        arr[i] = (rand_r(&t) % 100000) * 0.339 * 2.94988200619; //random double, more like random ints mapped to doubles. Max num is 100,000 exactly
    }
}

void printArr(double arr[])
{

    for (int i = 0; i < NUM_OF_FITNESS_INDICES; i++)
        printf("%f\n", arr[i]);
}