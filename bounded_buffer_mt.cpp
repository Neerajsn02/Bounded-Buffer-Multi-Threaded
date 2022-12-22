#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "tands.h"
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/time.h>
#include <chrono>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include "var_dec.h"


time_t start;
using namespace std;
FILE *fp;

// Each Task that will be added to the queue will be a struct of this form
// The structure for each task that the producer sends to the queue
// Each task will be a structure with 2 items
// The first item is a call to the respective commands
// The second item is the argument of the function call

typedef struct Task {
    void (*task_function)(int);
    int arg1;
    int arg2;
} Task;

// Creating a Task queue

Task task_queue[256] = { 0 };
int task_count = 0;


// Time implementation

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::duration<float> fsec;
auto t0 = Time::now();


// Create a mutex for the critical sections
pthread_mutex_t mutex_taskqueue;
pthread_mutex_t mutex_filewrite;
pthread_cond_t condition_queue;

void execute_task(Task *task){
    task->task_function(task->arg1);
}



// Function to add task to the queue
// We add an element to the last available space in the queue

void add_task(Task task){
    auto t4 = Time::now();
    fsec fs = t4-t0;

    pthread_mutex_lock(&mutex_filewrite);
    fprintf(fp, "%.3f\t", fs.count());
    fprintf(fp, "id= 0\t");
    fprintf(fp, "Work\t \t");
    fprintf(fp, "%d\n", task.arg1);
    pthread_mutex_unlock(&mutex_filewrite);

    pthread_mutex_lock(&mutex_taskqueue);
    task_queue[task_count] = task;
    task_count++;
    pthread_mutex_unlock(&mutex_taskqueue);

    pthread_cond_signal(&condition_queue);
}

// Consumer routine will dequeue an element and execute the task from the queue

void *consumer(void *arg){
    int thread_num = *(int*)arg;
    // wait if queue is empty
    auto t5 = Time::now();
    fsec fs = t5 - t0;
    pthread_mutex_lock(&mutex_filewrite);
    fprintf(fp, "%.3f\t", fs.count());
    fprintf(fp, "id= %d\t", thread_num);
    fprintf(fp, "Ask\t \n");
    ask_num++;
    pthread_mutex_unlock(&mutex_filewrite);

    pthread_mutex_lock(&mutex_taskqueue);
    while (task_count == 0){
        auto t6 = Time::now();
        fsec fs_3 = t6 - t0;
        fprintf(fp, "%.3f\t", fs_3.count());
        fprintf(fp, "id= %d\t", thread_num);
        fprintf(fp, "Ask\t \t");
        ask_num++;
        pthread_cond_wait(&condition_queue, &mutex_taskqueue);
    }
    pthread_mutex_unlock(&mutex_taskqueue);

    // main while loop will run as long as queue is not empty
    while (task_count > 0) {
        Task task;
        pthread_mutex_lock(&mutex_taskqueue);
    
        task = task_queue[0];  // Pop out first element/task
        auto t6 = Time::now();
        fsec fs_2 = t6 - t0;

        pthread_mutex_lock(&mutex_filewrite);
        fprintf(fp, "%.3f\t", fs_2.count());
        fprintf(fp, "id= 0\t");
        fprintf(fp, "Receive\t \t");
        fprintf(fp, "%d\n", task.arg1);
        rec_num++;
        pthread_mutex_unlock(&mutex_filewrite);

        int i;
        for (i = 0; i < task_count-1;i++){
            task_queue[i] = task_queue[i+1];  // Once we dequeue an object, move all elements to the right
        }
        task_count--;
        
        pthread_mutex_unlock(&mutex_taskqueue);

        
        execute_task(&task); // Actually execute the task (T<> or S<>)

        pthread_mutex_lock(&mutex_filewrite);
        auto t1 = Time::now();
        fsec fs = t1 - t0;
        fprintf(fp, "%.3f\t", fs.count());
        fprintf(fp, "id= %d\t", thread_num);
        fprintf(fp, "Complete\t");
        fprintf(fp, "%d\n", task.arg1);
        complete_num++;
        thread_work[thread_num]++;
        pthread_mutex_unlock(&mutex_filewrite);

        pthread_mutex_lock(&mutex_filewrite);
        fprintf(fp, "%.3f\t", fs.count());
        fprintf(fp, "id= %d\t", thread_num);
        fprintf(fp, "Ask\t \n");
        ask_num++;

        pthread_mutex_unlock(&mutex_filewrite);
             
    }
    free(arg);
    return NULL;
}
    


int main(int argc, char *argv[]){

    // Get the number of threads from the user - n consumer threads

    char *ptr;
    int n = strtol(argv[1], &ptr, 10);
    thread_work = (int *)calloc(n, sizeof(int));

    pthread_mutex_init(&mutex_taskqueue, NULL);
    pthread_mutex_init(&mutex_filewrite, NULL);
    pthread_cond_init(&condition_queue, NULL);

    // Create log file
    char file_name[100];
    char file_zero[100] = "prodcon.0.log";
    int result = remove(file_zero);
    if (argc == 2){
        strcpy(file_name, file_zero);
        fp = fopen(file_name, "a+");
    }
    else if (argc > 2){
        std::stringstream ss;
        string str1 = "prodcon.";
        string str2 = ".log";
        ss << str1 << argv[2] << str2;
        std::string s = ss.str();
        char *file_name = new char [s.length() + 1];
        strcpy (file_name, s.c_str());
        int result = remove(file_name);
        fp = fopen(file_name, "a+");
    }
    
    
    // Get User input
    // Create task based on input and submit it to the queue

    char ch;
    int k, lim;
    char str_t = 'T';
    char str_s = 'S';
    char str_stop = 'O';


    
    // Read input from a file
    std::string line;
    vector <string> lines;
    char cmd;
    int par;

    while (getline(cin, line)){
        std::istringstream iss(line);
        iss >> cmd >> par;
        lines.push_back(line);

        if (cmd == str_t){
            Task t = {
            .task_function = &Trans,
            .arg1 = par,
            .arg2 = 0
            };
            add_task(t);
            work_num++;
        
        }
        else if (cmd == str_s){
            auto t3 = Time::now();
            fsec fs = t3 - t0;
            fprintf(fp, "%.3f\t", fs.count());
            fprintf(fp, "id= 0\t");
            fprintf(fp, "Sleep\t \t");
            fprintf(fp, "%d\n", par);
            Sleep(par); 
            sleep_num++;
        
        }


    }
    
    


    // Create n consumer threads
    pthread_t th[n+1];
    int i;
    for (i = 1; i < n+1; i++){
        int* a = (int*)malloc(sizeof(int));
        *a = i;
        if (pthread_create(&th[i], NULL, &consumer, a) != 0){
            perror("Failed to create thread\n");
            return 1;
        }
    }

    // End consumer threads

    for (i = 1; i < n+1; i++){
        if (pthread_join(th[i], NULL) != 0){
            perror("Failed to end thread\n");
            return 1;
        }
    }
    // Summary statistics
    fprintf(fp, "SUMMARY\n");
    fprintf(fp, "Work: %d\n", work_num);
    fprintf(fp, "Ask: %d\n", ask_num);
    fprintf(fp, "Receive: %d\n", rec_num);
    fprintf(fp, "Complete: %d\n", complete_num);
    fprintf(fp, "Sleep: %d\n", sleep_num);
    for(int i = 1; i < n+1; i++){
        fprintf(fp, "Thread %d: %d\n", i, thread_work[i]);
    }
    
    auto t10 = Time::now();
    fsec fs_final = t10-t0;
    float trans_per_sec = ((float)work_num/ fs_final.count());
    fprintf(fp, "Transactions per Second: %.2f\n", trans_per_sec);
    
    pthread_mutex_destroy(&mutex_taskqueue);
    pthread_mutex_destroy(&mutex_filewrite);
    pthread_cond_destroy(&condition_queue);
    fclose(fp);
    free(thread_work);
    return 0;
}


