#define _GNU_SOURCE
#define NUM_OF_THREADS 100
#define NUM_OF_POPULATION 10
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



*/
typedef struct
{
    double fitness[NUM_OF_FITNESS_INDICES];
    pthread_t individualNumber;

} individual;

void randomNumArr(double *arr);
void *threadFunc(individual *person);

individual *population[NUM_OF_POPULATION]; //big number, used to filter to small population
int populationIndex = 0;

individual *childPopulation[NUM_OF_THREADS]; //small number, the population that will be doing the reproduction
int childPopulationIndex = 0;

pthread_mutex_t mutex; //mutex used for writing to shared childPopulation array
double bestFit[NUM_OF_FITNESS_INDICES];

int main()
{
    pthread_t threadNum[NUM_OF_THREADS];

    // //creates all the threads
    // for (int i = 0; i < NUM_OF_THREADS; i++)
    // {
    //     pthread_create(&threadNum[i], NULL, threadFunc, NULL);
    // }

    // //joins all the threads
    // for (int i = 0; i < NUM_OF_THREADS; i++)
    // {
    //     pthread_join(threadNum[i], NULL);
    // }

    randomNumArr(bestFit);

    for (int i = 0; i < NUM_OF_FITNESS_INDICES; i++)
        printf("%f\n", bestFit[i]);
}

/*
Calls all functions for each thread
*/
void *threadFunc(individual *person)
{
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
        arr[i] = (rand_r(&t) % 100) * 0.339 * 2.979649; //random double, more like random ints mapped to doubles
    }
}
