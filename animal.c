#define _GNU_SOURCE
#define NUM_OF_POPULATION 10000
#define NUM_OF_FITNESS_INDICES 1
#define NUM_OF_GENS 25
#define NUM_OF_PARENTS (2 * NUM_OF_POPULATION) //amt of parents each child has
#define AMT_OF_ERROR_PER_INDICE 100            //with our # range going from 0 to 100,000, a 100 error per indice means 1/1000 error
#define TOP_X 10

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
void *initializePopulation(void *thread);
void printArr(double arr[]);
double fitnessComparison(double individual[], double goal[]);
void *findParents(void *i);
void parentThreadingFunc();
void initializePopulationThreading();
void haveChildren();
double getError(int popNum);
void calculatePopulationWeights();
void *mutationGenerator(void *thread);
double algorithmInitialization();
void bestMutationChance(int runsToGetAvg);
void populationWeightTopIndividuals();
void pushArrayDownOneIndex(individual *topIndividual, int pushDownAmt);
double threadCountTime(int runsToGetAvg, int threadAmt);
void *produceKids(void *thread);

int MUTATION_CHANCE = 88; // mutation_chance% of mutation, if 20, then 80
int THREAD_COUNT = 4;

double bestFit[NUM_OF_FITNESS_INDICES];
individual *population[NUM_OF_POPULATION];
individual *initialPopulation[NUM_OF_POPULATION]; //duplicates population array to free the pointers

individual *parents[NUM_OF_POPULATION * 2];

int main()
{

    // bestMutationChance(25); //gets the best mutation chance with current crossover function
    int bestThreadCount = 0;
    double bestTime = INFINITY;
    for (int i = 1; i < 101; i++)
    {
        if (NUM_OF_POPULATION % i == 0)
        {
            double t = threadCountTime(500, i);
            if (t < bestTime)
            {
                bestTime = t;
                bestThreadCount = i;
            }
        }
    }

    printf("best thread %d with time %f\n", bestThreadCount, bestTime);

    // algorithmInitialization();

    //dealloc. uses the initial population's pointers since those pointers are gonna get doubled up on in the main arr
    for (int i = 0; i < NUM_OF_POPULATION; i++)
        free(initialPopulation[i]);
}

double threadCountTime(int runsToGetAvg, int threadAmt)
{
    THREAD_COUNT = threadAmt;
    double totalTime = 0;
    printf("threads: %d samplesize: %d \n", threadAmt, runsToGetAvg);

    for (int j = 0; j < runsToGetAvg; j++)
    {
        totalTime += algorithmInitialization();
    }

    double avgTime = totalTime / runsToGetAvg;
    printf("avg time %f with a net time of %f\n", avgTime, totalTime);

    return avgTime;
}

/**
 * Runs through a bunch of mutation chances to find best mutation chance with the current crossover function 
*/
void bestMutationChance(int runsToGetAvg)
{
    int bestMutationChance, initialMutationChance = MUTATION_CHANCE;

    FILE *fp;
    fp = fopen("t22.txt", "w");

    if (fp == NULL)
        exit(1);
    fputs("# of indices,\t # of gens,\t # of pop,\t sample size,\t mutation chance,\t avgTime,\t netTime\n", fp);
    double bestRuntime = INFINITY;
    for (int i = 0; i < 100 - initialMutationChance; i++)
    {
        printf("run #%d with mutation chance %d\n", i, MUTATION_CHANCE);
        double totalTime = 0;

        for (int j = 0; j < runsToGetAvg; j++)
        {
            totalTime += algorithmInitialization();
        }

        double avgTime = totalTime / runsToGetAvg;
        printf("run #%d with avg time %f with a net time of %f\n", i, avgTime, totalTime);

        fprintf(fp, "%d,\t %d,\t %d,\t %d,\t %d, %f,\t %f\n", NUM_OF_FITNESS_INDICES, NUM_OF_GENS, NUM_OF_POPULATION, runsToGetAvg, MUTATION_CHANCE, avgTime, totalTime);

        if (avgTime < bestRuntime)
        {
            bestRuntime = avgTime;
            bestMutationChance = MUTATION_CHANCE;
        }

        MUTATION_CHANCE++;
    }

    printf("for #%d indices, best mutation chance is %d with avg runtime of %f\n", NUM_OF_FITNESS_INDICES, bestMutationChance, bestRuntime);
    fclose(fp);
}

//initializes the algorithm and calls it for each generation
double algorithmInitialization()
{
    struct timeval start, end;
    gettimeofday(&start, NULL); //start timer

    randomNumArr(bestFit); //generates what we will be regarding the best fit array

    initializePopulationThreading();
    printf("pregen individual#1 err= %f\n", getError(0));

    // printf("gen #%d %f\n", 0, getError());

    for (int i = 0; i < NUM_OF_GENS; i++)
    {

        parentThreadingFunc();

        haveChildren();

        //find lowest error in population and break
        double lowestErr = INFINITY;
        int popWithLowestErr;
        int genCounter = i;

        //can multithread finding each of the population's error. can add it to struct as 'currentError' and simply multithread it. maybe faster?
        for (int j = 0; j < NUM_OF_POPULATION; j++)
        {

            double err = getError(j);
            if (err < lowestErr)
            {
                lowestErr = err;
                popWithLowestErr = j;
            }

            if (err < (AMT_OF_ERROR_PER_INDICE * NUM_OF_FITNESS_INDICES))
            {
                // printf("gen #%d had err %f and is converged!\n", i, err);
                i = NUM_OF_GENS;
                break;
            }
        }

        printf("gen #%d pop %d lowest err: %f\n", genCounter, popWithLowestErr, lowestErr);
    }

    gettimeofday(&end, NULL); //end timer

    printf("Time passed %f seconds \n", (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) * 1.0 / 1000000));

    return (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) * 1.0 / 1000000);
}

//get total absolute val error on avg just for debugging
double getError(int popNum)
{
    double error = 0;

    for (int j = 0; j < NUM_OF_FITNESS_INDICES; j++)
    {
        error += fabs(population[popNum]->fitness[j] - bestFit[j]);
    }

    return error;
}

/**
 * crosses the arrays of each 2 parents into a new child, 
 * with each index having a 50% chance of being from parent A or from parent B
*/
void haveChildren()
{

    pthread_t threadNums[THREAD_COUNT];

    //call each thread with the thread num
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        int *j = (int *)malloc(sizeof(int)); //mallocs int such that value does change due to synchronization issues
        *j = i;
        pthread_create(&threadNums[i], NULL, produceKids, (void *)j);
    }

    //joins all the threads
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threadNums[i], NULL);
    }
}

void *produceKids(void *thread)
{
    int threadNum = *(int *)thread;
    free(thread);

    int workPerThread = (int)ceil(NUM_OF_PARENTS / THREAD_COUNT); //100 pop, 4 threads, = 25

    int popIndex = threadNum * workPerThread; //used to update population

    struct timeval time;
    int t;
    int crossOverPoint;

    double tempA[NUM_OF_FITNESS_INDICES];
    double tempB[NUM_OF_FITNESS_INDICES];

    for (int i = threadNum * workPerThread; i < (workPerThread * (threadNum + 1)); i += 2)
    {

        //copy data into temp var
        for (int copyIndex = 0; copyIndex < NUM_OF_FITNESS_INDICES; copyIndex++)
        {

            tempA[copyIndex] = parents[i]->fitness[copyIndex];
            tempB[copyIndex] = parents[i + 1]->fitness[copyIndex];

            gettimeofday(&time, NULL);
            t = time.tv_usec; //random math to maybe make it more chaotic?
            crossOverPoint = (rand_r(&t) % 100);

            if (crossOverPoint < 50)
                population[popIndex]->fitness[copyIndex] = tempA[copyIndex];
            else
                population[popIndex]->fitness[copyIndex] = tempB[copyIndex];
        }

        population[popIndex]->fitnessIndex = fitnessComparison(population[popIndex]->fitness, bestFit); //update fitness

        popIndex++;
    }

    pthread_exit(0);
}

//calls the threading function for the initial population
void initializePopulationThreading()
{
    pthread_t threadNums[THREAD_COUNT];

    for (int i = 0; i < NUM_OF_POPULATION; i++)
    {
        //allocate mem for each population, store original points in initialPopulation array to free them at end of run
        initialPopulation[i] = (individual *)malloc(sizeof(individual));
        population[i] = initialPopulation[i];
    }

    //call each thread with the thread num
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        int currentThread = i;

        pthread_create(&threadNums[i], NULL, initializePopulation, (void *)&currentThread);
    }

    //joins all the threads
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threadNums[i], NULL);
    }
}

/*
Calls all functions for each thread
*/
void *initializePopulation(void *thread)
{
    // //initialize the indiivdual
    int threadNum = *((int *)(thread));                              //for 4 threads, this would be 0-3
    int workPerThread = (int)ceil(NUM_OF_POPULATION / THREAD_COUNT); //100 pop, 4 threads, = 25

    for (int j = workPerThread * threadNum; j < (workPerThread * (threadNum + 1)); j++)
    {

        randomNumArr(population[j]->fitness);
        population[j]->fitnessIndex = fitnessComparison(population[j]->fitness, bestFit);
    }

    pthread_exit(0);
}
//calculates a weight for each population, which is used to do weighted random reproduction
//for equal distribution among all individuals based on fitness
void calculatePopulationWeights()
{
    double totalSum = 0;

    for (int i = 0; i < NUM_OF_POPULATION; i++)
        totalSum += population[i]->fitnessIndex;

    for (int i = 0; i < NUM_OF_POPULATION; i++)
        population[i]->weight = population[i]->fitnessIndex / totalSum;
}

//generates mutations for each array index for each parent called using threading
void *mutationGenerator(void *thread) //A little weird naming haha
{
    struct timeval time;
    int t;
    int mutationChance;

    int threadNum = *((int *)(thread)); //for 4 threads, this would be 0-3
    free(thread);
    int workPerThread = (int)ceil(NUM_OF_POPULATION / THREAD_COUNT); //100 pop, 4 threads, = 25

    for (int j = workPerThread * threadNum; j < (workPerThread * (threadNum + 1)); j++)
    {

        for (int i = 0; i < NUM_OF_FITNESS_INDICES; i++)
        {
            gettimeofday(&time, NULL);
            t = time.tv_usec;

            mutationChance = rand_r(&t) % 100;
            if (mutationChance >= (100 - MUTATION_CHANCE)) //20% chance = 100 - 20
            {

                //50% chance to double, or 50% to half
                gettimeofday(&time, NULL);
                t = time.tv_usec;
                int binaryMutator = (rand_r(&t) % 2);

                if (binaryMutator == 0)
                    population[j]->fitness[i] *= 1.1; //increase it
                else
                    population[j]->fitness[i] *= .9; //decrease it
            }
        }
    }

    pthread_exit(0);
}

//creates the threads to find the new parents based on current population
void parentThreadingFunc()
{
    pthread_t threadNums[THREAD_COUNT];
    calculatePopulationWeights();

    //threads to find

    //call each thread with the thread num
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        int *j = (int *)malloc(sizeof(int)); //mallocs int such that value does change due to synchronization issues
        *j = i;
        pthread_create(&threadNums[i], NULL, findParents, (void *)j);
    }

    //joins all the threads
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threadNums[i], NULL);
    }

    //mutate the parents with threading
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        int *j = (int *)malloc(sizeof(int)); //mallocs int such that value does change due to synchronization issues
        *j = i;

        pthread_create(&threadNums[i], NULL, mutationGenerator, (void *)j); //each parent found using x threads
    }

    //joins all the threads
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threadNums[i], NULL);
    }
}

/*
Finds the new parents for the next children and puts them in the parents array
Uses weighted randomness, where each individual is given a weight due to their relative fitness, and then uses a random number to choose
one of the weighted parents and puts it in the array
*/
void *findParents(void *currentThread)
{

    struct timeval time;
    double num;
    double newParent; //random num for finding new parent based on weighing function

    int t; //for time
    int threadNum = *(int *)currentThread;

    free(currentThread);

    int workPerThread = (int)ceil(NUM_OF_PARENTS / THREAD_COUNT); //100 pop, 4 threads, = 25

    for (int i = workPerThread * threadNum; i < (workPerThread * (threadNum + 1)); i++)
    {

        if (i > NUM_OF_PARENTS) //shit solution to fix workPerThread not being a clean number
            break;

        num = 0; //sum of weights to find which parent to use

        gettimeofday(&time, NULL);
        t = time.tv_usec;

        newParent = ((double)(rand_r(&t) % 100000)) / 100000; //0 to 999999 because our random doubles go to 0 to 100,000

        for (int j = 0; j < NUM_OF_POPULATION; j++)
        {
            num += population[j]->weight;

            if (newParent < num)
            {
                parents[i] = population[j];

                break;
            }
        }
    }

    pthread_exit(0); //close thread ; job done
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
