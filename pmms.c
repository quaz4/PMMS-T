/*
    Created by William Stewart 18349788
    A program that calculates the matrix multiplication sum of two matrices
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "pmms.h"
#include "fileIO.h"

/*Shared memory*/
pthread_mutex_t mutex;

pthread_cond_t non_full;
pthread_cond_t non_empty;

int full;
int empty;

int m;
int n;
int k;

int* a;
int* b;
int* c;
Subtotal sub;

/*Thread function pointer*/
void* producer(void* arg);

/*Producer function, thread execution starts here*/
void* producer(void* arg)
{
    int temp;
    int i;
    int j;

    /*Assign row number*/
    static int row = -1;
    row++;

    /*Produce content*/
    temp = 0;

    /*Calculate row in array c*/
    for(i = 0; i < k; i++)
    {
        for(j = 0; j < n; j++)
        {
            temp = temp + a[row*n + j] * b[i + j*k];
        }

        c[row*k + i] = temp;
        temp = 0;
    }

    /*Calculate row total to store in subtotal*/
    for(i = 0; i < k; i++)
    {
        temp = temp + c[row*k + i];
    }

    /*Aquire mutex lock*/
    pthread_mutex_lock(&mutex);

    /*Block if buffer full*/
    while(full)
    {
        pthread_cond_wait(&non_full, &mutex);
    }

    /*Insert info into buffer subtotal*/
    sub.subtotal = temp;
    sub.tid = pthread_self();

    /*Let other threads know array is now full*/
    full = 1;
    empty = 0;

    /*Release mutex lock*/
    pthread_mutex_unlock(&mutex);

    /*Notify thread buffer is full*/
    pthread_cond_signal(&non_empty);

    return NULL;
}

/*Using m threads, calculates the matrix multiplication sum*/
int calcTotal(int m, int n, int k, int* arrayA, int* arrayB)
{
	int i;
    int errCheck;
    pthread_t* threads;
    int total;

    pthread_attr_t attr;

    total = 0;
    full = 0;
    empty = 1;

    threads = (pthread_t*)malloc(sizeof(pthread_t)*m);

    /*Initialise mutex and conditional variables*/
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&non_full, NULL);
    pthread_cond_init(&non_empty, NULL);

    pthread_attr_init(&attr);

    /*Create m threads*/
    for(i = 0; i < m; i++)
    {
        errCheck = pthread_create(&threads[i],&attr , producer, NULL);

        /*Check for errors*/
        if(errCheck != 0)
        {
            perror("Error creating theads");
        }
    }

    /*Remove from values from buffer m times*/
    for(i = 0; i < m; i++)
    {
        /*Aquire mutex lock*/
        pthread_mutex_lock(&mutex);
        
        /*Block if empty*/
        while(empty)
        {
            pthread_cond_wait(&non_empty, &mutex);
        }

        /*Consume content*/
        printf("Subtotal produced by thread with ID %lu: %d\n", sub.tid, sub.subtotal);
        total = total + sub.subtotal;

        full = 0;
        empty = 1;

        /*Release mutex lock*/
        pthread_mutex_unlock(&mutex);

        /*Notify thread buffer is empty*/
        pthread_cond_signal(&non_full);
    }

    /*Output final total*/
    printf("Total: %d\n", total);

    free(threads);

	return 0;
}

/*Takes in file names of matrix A and B as arguments as well as their dimensions*/
/*execution eg: pmms matrix_A matrix_B M N K*/
int main(int argc, char** argv)
{
    /*Convert paramaters from strings to ints*/
    m = atoi(argv[3]);
    n = atoi(argv[4]);
    k = atoi(argv[5]);

    /*Allocate memory for matrix arrays*/
    /*Uses logical 2D array so can be used in shared memory*/
    a = (int*)malloc(sizeof(int)*m*n);
    b = (int*)malloc(sizeof(int)*n*k);
    c = (int*)malloc(sizeof(int)*m*k);
    
    /*Read files and assign to arrays*/
    readFile(argv[1], m, n, a);
    readFile(argv[2], n, k, b);

    calcTotal(m, n, k, a, b);

    /*Free shared arrays*/
    free(a);
    free(b);
    free(c);

    /*Destroy mutex and conditional variables*/
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&non_empty);
    pthread_cond_destroy(&non_full);

    return 0;
}