#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/syscall.h>
#include <assert.h>
#include <math.h>

struct account 
{
    atomic_int deposit;
    char name [100];
    int id;
    atomic_int myLock;
};

struct account account_array[500];
volatile int stat=1;
volatile int cnt;



void money_transfer(int from, int to, int amount)
{
    int expect;

    //避免deadlock
    if (from < to ) 
    { 
        do{expect=0;}while(!atomic_compare_exchange_strong(&(account_array[from].myLock), &expect, 1));
        do{expect=0;}while(!atomic_compare_exchange_strong(&(account_array[to].myLock), &expect, 1));
    } 
    else 
    {
        do{expect=0;}while(!atomic_compare_exchange_strong(&(account_array[to].myLock), &expect, 1));
        do{expect=0;}while(!atomic_compare_exchange_strong(&(account_array[from].myLock), &expect, 1));
    }

    //鎖定戶頭以後,開始轉帳
    atomic_store(&(account_array[from].deposit),atomic_load(&(account_array[from].deposit))-amount);
    atomic_store(&(account_array[to].deposit),atomic_load(&(account_array[to].deposit))+amount);
    cnt++;

    
    //轉帳結束,可以解開鎖
    atomic_store(&(account_array[from].myLock),0);  //unlock(&account_array[from]);
    atomic_store(&(account_array[to].myLock),0);    //unlock(&account_array[to]);

}


void sighandl_transfer(int sig)
{
    stat = 0;
}

void thread(void *givenName)
{
    int id = (intptr_t)givenName;
    account_array[id].id = id+1;

    signal(SIGINT, sighandl_transfer);

    sprintf((account_array+id)->name,"user%d",account_array[id].id);
    
    while( stat )
    {
        
        int amount = (rand()%491)+10;
        int to;    
        do{
            to = rand()%5;
        }while( to == id );

        money_transfer(id, to, amount);
        int slp = rand()%3+1;
        usleep(slp);
    }
}

int main()
{
    srand(time(NULL));

    printf("money transfering...\n");

    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*500);

    for(long i=0; i<500; i++)
    {
        atomic_store(&account_array[i].deposit, 10000);
        atomic_store(&account_array[i].myLock, 0);
        pthread_create(&tid[i], NULL, (void *) thread, (void*)i);
    }

    
    for(int i=0; i<500; i++)
		pthread_join(tid[i], NULL);
    
    int num = 0;
    printf("\nresult:\n");

    for( int i=0; i<500; i++ )
    {
        printf("id: %d, name: %s, money: %d\n",account_array[i].id,account_array[i].name,atomic_load(&account_array[i].deposit));
        num = num + atomic_load(&account_array[i].deposit);
    }

    printf("total money : %d\n",num);
    printf("total times : %d\n",cnt);
    
    return 0;

}