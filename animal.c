#define _GNU_SOURCE
#define NUM_OF_POPULATION 2
#define NUM_OF_FITNESS_INDICES 10

#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>

typedef struct
{
    double fitness[NUM_OF_FITNESS_INDICES];
    pthread_t individualNumber;
    double fitnessIndex;

} individual;

void randomNumArr(double *arr);
void *initializePopulation(void *person);
void printArr(double arr[]);
double fitnessComparison(double individual[], double goal[]);
void *findParents(void *totalSum);
void parentThreadingFunc();
void initializePopulationThreading();

double bestFit[NUM_OF_FITNESS_INDICES];
individual *population[NUM_OF_POPULATION];
individual *parents[NUM_OF_POPULATION * 2];

pthread_mutex_t mutex; //mutex used for writing to shared childPopulation array

int main()
{
    randomNumArr(bestFit); //generates what we will be regarding the best fit array
    initializePopulationThreading();

    parentThreadingFunc();
}

//calls the threading function for the initial population
void initializePopulationThreading()
{
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

//creates the threads to find the new parents based on current population
void parentThreadingFunc()
{
    pthread_t threadNums[NUM_OF_POPULATION];

    double totalSum = 0;
    for (int i = 0; i < NUM_OF_POPULATION; i++)
        totalSum += population[i]->fitnessIndex;

    //threads to find parents
    for (int i = 0; i < NUM_OF_POPULATION; i++)
    {
        pthread_create(&threadNums[i], NULL, findParents, (void *)&totalSum); //i send totalSum because i dont want to loop and calculate it each time
    }

    //joins all the threads
    for (int i = 0; i < NUM_OF_POPULATION; i++)
    {
        pthread_join(threadNums[i], NULL);
    }
}

/*
Finds the new parents for the next children and puts them in the parents array
Uses weighted randomness, where each individual is given a weight due to their relative fitness, and then uses a random number to choose
one of the weighted parents and puts it in the array
*/
void *findParents(void *totalSum)
{

    double weights[NUM_OF_POPULATION];

    double sum = *((double *)totalSum);

    for (int i = 0; i < NUM_OF_POPULATION; i++)
    {

        weights[i] = ((1 - ((population[i]->fitnessIndex) / sum)) / (NUM_OF_POPULATION - 1)); //gives you each weight which added together give 1.0
    }

    for (int i = 0; i < NUM_OF_POPULATION * 2; i++)
    {

        struct timeval time;
        gettimeofday(&time, NULL);

        int t = time.tv_usec; //random math to maybe make it more chaotic?

        double newParent = ((double)(rand_r(&t) % 100)) / 100;

        double num = 0;
        for (int j = 0; j < NUM_OF_POPULATION; j++)
        {
            num += weights[j];
            if (newParent < num)
            {
                parents[i] = population[j];
                j = NUM_OF_POPULATION;
            }
        }
    }

    pthread_exit(0); //close thread ; job done
}

/*
Calls all functions for each thread
*/
void *initializePopulation(void *person)
{

    // //initialize the indiivdual
    individual *temp = (individual *)person; //MAKE SURE TO FREE THIS
    randomNumArr(temp->fitness);
    temp->fitnessIndex = fitnessComparison(temp->fitness, bestFit);

    pthread_exit(0);
}

// a return value of 0 means you hit the goal! higher than 0 means farther
double fitnessComparison(double individual[], double goal[])
{
    double fitnessFactor = 0;

    for (int i = 0; i < NUM_OF_POPULATION; i++)
    {
        fitnessFactor += fabs(individual[i] - goal[i]);
    }

    return fitnessFactor;
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