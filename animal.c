#define _GNU_SOURCE
#define NUM_OF_POPULATION 100
#define NUM_OF_FITNESS_INDICES 10
#define NUM_OF_GENS 100
#define NUM_OF_PARENTS (2 * NUM_OF_POPULATION) //amt of parents each child has

#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>

typedef struct
{
    double fitness[NUM_OF_FITNESS_INDICES];
    double fitnessIndex;

    double weight;

} individual;

void randomNumArr(double *arr);
void *initializePopulation(void *person);
void printArr(double arr[]);
double fitnessComparison(double individual[], double goal[]);
void *findParents(void *i);
void parentThreadingFunc();
void initializePopulationThreading();
void haveChildren();
double getError();
void calculatePopulationWeights();

double bestFit[NUM_OF_FITNESS_INDICES];
individual *population[NUM_OF_POPULATION];
individual *initialPopulation[NUM_OF_POPULATION]; //duplicates population array to free the pointers

individual *parents[NUM_OF_POPULATION * 2];

int main()
{
    randomNumArr(bestFit); //generates what we will be regarding the best fit array

    initializePopulationThreading();
    printf("gen #%d %f\n", 0, getError());

    for (int i = 0; i < NUM_OF_GENS; i++)
    {

        parentThreadingFunc();

        haveChildren();
    }
    printf("gen #%d %f\n", NUM_OF_GENS, getError());

    //dealloc
    for (int i = 0; i < NUM_OF_POPULATION; i++)
        free(initialPopulation[i]);
}

//get total absolute val error on avg just for debugging
double getError()
{
    double error = 0;
    for (int i = 0; i < NUM_OF_POPULATION; i++)
    {
        for (int j = 0; j < NUM_OF_FITNESS_INDICES; j++)
        {
            error += fabs(population[i]->fitness[j] - bestFit[j]);
        }
    }

    return error / NUM_OF_POPULATION;
}

void haveChildren()
{

    int popIndex = 0; //used to update population

    double tempA[NUM_OF_FITNESS_INDICES];
    double tempB[NUM_OF_FITNESS_INDICES];

    for (int i = 0; i < NUM_OF_PARENTS; i += 2)
    {

        //copy data into temp var
        for (int copyIndex = 0; copyIndex < NUM_OF_FITNESS_INDICES; copyIndex++)
        {
            tempA[copyIndex] = parents[i]->fitness[copyIndex];
            tempB[copyIndex] = parents[i + 1]->fitness[copyIndex];
        }

        //for each indice, gets the average of the two parent's index and sets it to the index in population
        for (int j = 0; j < NUM_OF_FITNESS_INDICES; j++)
        {
            double val = (tempA[j] + tempB[j]) / 2;
            population[popIndex]->fitness[j] = val;
        }

        population[popIndex]->fitnessIndex = fitnessComparison(population[popIndex]->fitness, bestFit); //update fitness

        popIndex++;
    }
}

//calls the threading function for the initial population
void initializePopulationThreading()
{
    pthread_t threadNums[NUM_OF_POPULATION];

    for (int i = 0; i < NUM_OF_POPULATION; i++)
    {
        //allocate mem for each population, store original points in initialPopulation array to free them at end of run
        initialPopulation[i] = (individual *)malloc(sizeof(individual));
        population[i] = initialPopulation[i];

        pthread_create(&threadNums[i], NULL, initializePopulation, (void *)population[i]);
    }

    //joins all the threads
    for (int i = 0; i < NUM_OF_POPULATION; i++)
    {
        pthread_join(threadNums[i], NULL);
    }
}

void calculatePopulationWeights()
{
    double totalSum = 0;

    for (int i = 0; i < NUM_OF_POPULATION; i++)
        totalSum += population[i]->fitnessIndex;

    for (int i = 0; i < NUM_OF_POPULATION; i++)
        population[i]->weight = population[i]->fitnessIndex / totalSum;
}
//creates the threads to find the new parents based on current population
void parentThreadingFunc()
{
    pthread_t threadNums[NUM_OF_PARENTS];
    calculatePopulationWeights();

    //threads to find

    for (int parent = 0; parent < NUM_OF_PARENTS; parent++)
    {
        int *j = (int *)malloc(sizeof(int)); //mallocs int such that value does change due to synchronization issues

        *j = parent;                                                       //sets value of malloc'd int to the parent index
        pthread_create(&threadNums[parent], NULL, findParents, (void *)j); //each parent found using x threads
    }

    //joins all the threads
    for (int i = 0; i < NUM_OF_PARENTS; i++)
    {
        pthread_join(threadNums[i], NULL);
    }
}

/*
Finds the new parents for the next children and puts them in the parents array
Uses weighted randomness, where each individual is given a weight due to their relative fitness, and then uses a random number to choose
one of the weighted parents and puts it in the array
*/
void *findParents(void *pointerToParentIndex)
{

    struct timeval time;
    double num;
    int parentIndex = *(int *)pointerToParentIndex;
    free(pointerToParentIndex);

    num = 0; //sum of weights to find which parent to use

    gettimeofday(&time, NULL);
    int t = time.tv_usec;

    double newParent = ((double)(rand_r(&t) % 100000)) / 100000; //0 to 999999 because our random doubles go to 0 to 100,000

    for (int j = 0; j < NUM_OF_POPULATION; j++)
    {
        num += population[j]->weight;

        if (newParent < num)
        {
            parents[parentIndex] = population[j];

            break;
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

    for (int i = 0; i < NUM_OF_FITNESS_INDICES; i++)
    {
        fitnessFactor += (fabs(individual[i] - goal[i]));
    }

    return (1 / fitnessFactor);
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
