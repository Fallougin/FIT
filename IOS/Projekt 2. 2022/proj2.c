#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <signal.h>


#define shmem_key 0x6543210 // id key for shared memory
#define MEM_ORDER __ATOMIC_SEQ_CST // memory order for atomic operations
#define WRITE_TO_FILE

#ifndef WRITE_TO_FILE
    #define fp stdout
#endif

///'help' message
const char *helpMessage =
    " There was an error processing the values entered from stdin, please ensure they meet the following format.\n"
    "\n"
    " Usage:   ./proj2 NO NH TI TB   where:\n"
    "\n"
    "   NO    -number of oxygen\n"
    "         -has to be in range:  0 < NO <= 1000\n"
    "   NH    -number of Hydrogens\n"
    "         -has to be in range:  0 < NH <= 1000\n"
    "   TI    -the maximum time (in milliseconds) for which Oxygen and Hydrogen atoms are generated\n"
    "         -has to be in range:  0 <= TI <= 1000\n"
    "   TB    -the maximum time (in milliseconds) for which H2O molecules are created\n"
    "         -has to be in range:  0 <= TB <= 1000\n"
    "\n"
    " All arguments have to be whole numbers."
    "\n";

/********************************
 *                              *
 *        SHARED MEMORY         *
 *                              *
 *******************************/ 
typedef struct {
    volatile int action;        // action counter
    volatile int countH;        // counter for Hydrogens whos ready
    volatile int countO;        // counter for Oxygens whos ready
    volatile int countH2O;      // counter for H2O molecules
    volatile int usedH;         // number of Hydrogen atom used
    volatile int usedO;         // number of Oxygen atom used

    sem_t semPrint;             // semaphore used by all processes to prevent simultaneous writing into the .out file

    pthread_mutexattr_t psharedm;  // attributes for mutexes
    pthread_condattr_t psharedc;   // attributes for condition variables
    pthread_mutex_t mutWaitH;   // mutex used for condition variable condWaitH
    pthread_cond_t condWaitH;   // conditional variable for waiting of Hydrogen process
    pthread_mutex_t mutWaitH2O; // mutex used for condition variable condWaitH2O
    pthread_cond_t condWaitH2O; // conditional variable for waiting of H2O molecule creation

    volatile bool readyH;       // Hydrogen process is ready for H2O molecule creation
    volatile bool readyH2O;     // H2O is created by Oxygen process

    volatile bool error;        // process creating error flag
    volatile bool finished;     // one of processes is finished
    volatile bool finishedH;    // creating of Hydrogen atoms is finished
    volatile bool finishedO;    // creating of Oxygen atoms is finished
    volatile bool creating;     // molecule is being created
} sharedMem;


/********************************
 *                              *
 *       GLOBAL VARIABLES       *
 *                              *
 *******************************/ 
int NO = 0; //input argv[0] for number of oxygen
int NH = 0; //input argv[1] for number of Hydrogens
int TI = 0; //input argv[2] for maximal time delay for oxygen 
int TB = 0; //input argv[3] for maximal time delay for Hydrogens 

sharedMem *shmem; //shared memory segment pointed

FILE *fp; //file pointer

/********************************
 *                              *
 *          FUNCTIONS           *
 *                              *
 *******************************/

/**
 *  argsCheck
 * -----------------------------------------------
 *  @brief: checks if input arguments meet required format
 *  
 *  @param argc: standard input argument count 
 *  @param argv: standard input argument values
 */
void argsCheck(int argc, char* argv[]){
    long val;
    char *next;
	if(argc!=5){ //test for number of arguments and floats/non-number characters
        fprintf(stderr,"%s",helpMessage);
        exit(1);
    }
    for (int i = 1; i < argc; i++) { //process all arguments one-by-one
        val = strtol (argv[i], &next, 10); //get value of arguments, stopping when NoN encountered
        if ((val%1 != 0) || (next == argv[i]) || (*next != '\0')) { // Check for empty string and characters left after conversion
            fprintf(stderr,"%s",helpMessage);
            exit(1);
        }
    }
}

/**
 *  argsLoad
 * -----------------------------------------------
 *  @brief: loads and stores arguments from standard input into pre-prepared global variables
 *  
 *  @param argv: standard input argument values
 */
void argsLoad(char* argv[]){
    NO = atof(argv[1]);
	NH = atof(argv[2]);
	TI = atof(argv[3]);
	TB = atof(argv[4]);
    if ((NO <= 0 || NO > 1000)|| //0 < NO <= 1000
        (NH <= 0 || NH > 1000)  || //0 < NH <= 1000
        (TI < 0 || TI > 1000)  || //0 <= TE <= 1000
        (TB < 0 || TB > 1000))    //0 <= TB <= 1000
	{
		fprintf(stderr,"%s",helpMessage); /// print help
		exit(1);
	}
}

/**
 *  freeShmem
 * -----------------------------------------------
 *  @brief: frees shared memory
 *  
 *  @param shmem: pointer to the shared memory structure
 *  @param id: identification number of the shared memory segment to be cleared
 */
void freeShmem(sharedMem *shmem, int id){
    shmdt(shmem); //detaches shared memory
    shmctl(id, IPC_RMID, NULL); //sets it to be deleted
}

/**
 *  freeSem
 * -----------------------------------------------
 *  @brief: frees semaphores, mutexes, condition variables and close file
 *  
 *  @param shmem: pointer to the shared memory structure
 */
void freeSem(sharedMem *shmem){
    sem_destroy(&shmem->semPrint);
    pthread_mutexattr_destroy(&shmem->psharedm);
    pthread_condattr_destroy(&shmem->psharedc);
    pthread_mutex_destroy(&shmem->mutWaitH);
    pthread_mutex_destroy(&shmem->mutWaitH2O);
    pthread_cond_destroy(&shmem->condWaitH);
    pthread_cond_destroy(&shmem->condWaitH2O);

#ifdef WRITE_TO_FILE
    fclose(fp);
#endif
}

/**
 *  init
 * -----------------------------------------------
 *  @brief: initializes semaphores, condition variables and counters in shared memory to wanted values
 *  
 *  @param shmem: pointer to the shared memory structure 
 *  @param id: identification number of the shared memory segment to be cleared
 */
void init(sharedMem* shmem, int id){
    bool err = false;

    memset(shmem, 0, sizeof(sharedMem));

    if (sem_init(&shmem->semPrint, 1, 1) < 0) {err = true;}

    pthread_mutexattr_init(&shmem->psharedm);
    pthread_mutexattr_setpshared(&shmem->psharedm, PTHREAD_PROCESS_SHARED);  // make mutexes shareable between processes
    pthread_condattr_init(&shmem->psharedc);
    pthread_condattr_setpshared(&shmem->psharedc, PTHREAD_PROCESS_SHARED);  // make condition variables shareable between processes

    if (pthread_mutex_init(&shmem->mutWaitH, &shmem->psharedm)) {err = true;}
    if (pthread_mutex_init(&shmem->mutWaitH2O, &shmem->psharedm)) {err = true;}
    if (pthread_cond_init(&shmem->condWaitH, &shmem->psharedc)) {err = true;}
    if (pthread_cond_init(&shmem->condWaitH2O, &shmem->psharedc)) {err = true;}

    if (err == true){
        fprintf(stderr,"Error initializing semaphores, mutexes and/or condition variables\n");
        freeSem(shmem);  // free semaphores, mutexes, condition variables and close file
        freeShmem(shmem,id);
        exit(1);
    }
}

/**
 *  OxygenFunc
 * -----------------------------------------------
 *  @brief: function for oxygen child processes
 *  
 *  @param shmem: pointer to the shared memory structure 
 */
void OxygenFunc(sharedMem* shmem){
    
    while (NH - shmem->usedH >= 2 && NO - shmem->usedO >= 1 && !shmem->error) {

        if (shmem->countO < NO) {
            sem_wait(&(shmem->semPrint));
                fprintf(fp,"%d: O %d: started\n", ++shmem->action, shmem->countO + 1); // ++ is locked by semPrint semaphore
                fflush(fp); //forces immediate writing
            sem_post(&(shmem->semPrint));
    
            if (shmem->error) { break; }
            usleep(rand() % (TI + 1) * 1000); //interval <0,TI>, simulates Oxygen atom generation
    
            sem_wait(&(shmem->semPrint));
                fprintf(fp,"%d: O %d: going to queue\n", ++shmem->action, ++shmem->countO); // ++ is locked by semPrint semaphore
                fflush(fp); //forces immediate writing
            sem_post(&(shmem->semPrint));
        }

        if (shmem->countH - shmem->usedH >= 2 && shmem->countO - shmem->usedO >= 1) {
            sem_wait(&(shmem->semPrint));
                shmem->creating = true;  // molecule creation started
                fprintf(fp,"%d: O %d: creating molecule %d\n", ++shmem->action, ++shmem->usedO, ++shmem->countH2O); // ++ is locked by semPrint semaphore
                fflush(fp); //forces immediate writing
            sem_post(&(shmem->semPrint));

            if (shmem->error) { break; }
            usleep(rand() % (TB + 1) * 1000); //interval <0,TI>, simulates H2O molecule generation

            if (shmem->error) { break; }
            if (!shmem->finished) {
                pthread_mutex_lock(&shmem->mutWaitH);
                while (!shmem->readyH) /*usleep(10000);//*/ { pthread_cond_wait(&shmem->condWaitH, &shmem->mutWaitH); }  // waiting for Hydrogen process
                shmem->readyH = false;
                pthread_mutex_unlock(&shmem->mutWaitH);
            }
        
            sem_wait(&(shmem->semPrint));
                shmem->creating = false;
                fprintf(fp,"%d: O %d: molecule %d created\n", ++shmem->action, shmem->usedO, shmem->countH2O); // ++ is locked by semPrint semaphore
                fflush(fp); //forces immediate writing
            sem_post(&(shmem->semPrint));

            shmem->readyH2O = true;
            pthread_cond_broadcast(&shmem->condWaitH2O);  // H2O molecule is created
        }

        if (!shmem->finishedO && shmem->countO >= NO) {
            sem_wait(&(shmem->semPrint));
                fprintf(fp,"%d: O %d: not enough O\n", ++shmem->action, shmem->countO); // ++ is locked by semPrint semaphore
                fflush(fp); //forces immediate writing
            sem_post(&(shmem->semPrint));
            shmem->finishedO = true;
        }
    }

    shmem->finished = true;
    shmem->readyH2O = true;
    pthread_cond_broadcast(&shmem->condWaitH2O);
}

/**
 *  HydrogenFunc
 * -----------------------------------------------
 *  @brief: function for Hydrogen child processes
 *  
 *  @param shmem: pointer to the shared memory structure 
 */
void HydrogenFunc(sharedMem* shmem){
    
    while (NH - shmem->usedH >= 2 && NO - shmem->usedO >= 1 && !shmem->error) {

        if (shmem->countH < NH) {
            sem_wait(&(shmem->semPrint));
                fprintf(fp,"%d: H %d: started\n", ++shmem->action, shmem->countH + 1); // ++ is locked by semPrint semaphore
                fflush(fp); //forces immediate writing
            sem_post(&(shmem->semPrint));
    
            if (shmem->error) { break; }
            usleep(rand() % (TI + 1) * 1000); //interval <0,TI>, simulates Oxygen atom generation
    
            sem_wait(&(shmem->semPrint));
                fprintf(fp,"%d: H %d: going to queue\n", ++shmem->action, ++shmem->countH); // ++ is locked by semPrint semaphore
                fflush(fp); //forces immediate writing
            sem_post(&(shmem->semPrint));
        }

        if (shmem->creating) {
            sem_wait(&(shmem->semPrint));
                fprintf(fp,"%d: H %d: creating molecule %d\n", ++shmem->action, ++shmem->usedH, shmem->countH2O); // ++ is locked by semPrint semaphore
                fprintf(fp,"%d: H %d: creating molecule %d\n", ++shmem->action, ++shmem->usedH, shmem->countH2O); // ++ is locked by semPrint semaphore
                fflush(fp); //forces immediate writing
            sem_post(&(shmem->semPrint));

            shmem->readyH = true;
            pthread_cond_broadcast(&shmem->condWaitH);  // Hydrogen process is ready

            if (shmem->error) { break; }
            if (!shmem->finished) {
                pthread_mutex_lock(&shmem->mutWaitH2O);
                while (!shmem->readyH2O) /*usleep(10000);//*/ { pthread_cond_wait(&shmem->condWaitH2O, &shmem->mutWaitH2O); }  // waiting for H2O molecule creation
                shmem->readyH2O = false;
                pthread_mutex_unlock(&shmem->mutWaitH2O);
            }
        
            sem_wait(&(shmem->semPrint));
                fprintf(fp,"%d: H %d: molecule %d created\n", ++shmem->action, shmem->usedH - 1, shmem->countH2O); // ++ is locked by semPrint semaphore
                fprintf(fp,"%d: H %d: molecule %d created\n", ++shmem->action, shmem->usedH, shmem->countH2O); // ++ is locked by semPrint semaphore
                fflush(fp); //forces immediate writing
            sem_post(&(shmem->semPrint));
        }

        if (!shmem->finishedH && shmem->countH >= NH) {
            sem_wait(&(shmem->semPrint));
                fprintf(fp,"%d: H %d: not enough H\n", ++shmem->action, shmem->countH); // ++ is locked by semPrint semaphore
                fflush(fp); //forces immediate writing
            sem_post(&(shmem->semPrint));
            shmem->finishedH = true;
        }
    }

    shmem->finished = true;
    shmem->readyH = true;
    pthread_cond_broadcast(&shmem->condWaitH);
}

/********************************
 *                              *
 *            MAIN              *
 *                              *
 *******************************/ 
int main(int argc, char *argv[])
{
/*Storing values from standard input. All parametres need to be given as whole numbers in their 
  respective ranges, else the program prints the help message to standard error output and exits.*/
    argsCheck(argc, argv);
//Values okay, load and save them
    argsLoad(argv);
//Values read successfully, open file 'proj2.out' for writing
#ifdef WRITE_TO_FILE
    fp = fopen("proj2.out", "w");
#endif
//Use current time as seed for random number generator
    srand(time(0));
//Segment which initializes shared memory
    int id;
    id = shmget(shmem_key, sizeof(sharedMem), IPC_CREAT | 0644);
    if (id < 0) {
        fprintf(stderr,"Shared memory error\n");
        exit(1);
    }
    shmem = (sharedMem *)shmat(id, NULL, 0);
//Initialize shared memory structure and create semaphores, mutexes, condition variables
    init(shmem, id);

//Create child processes for Hydrogens and Oxygens
    // Creating Oxygen process
    int idProcO = fork();
    if (idProcO == -1) { //forking error occurred
        fprintf(stderr,"Fork error for Oxygen process\n");
        freeSem(shmem);  // free semaphores, mutexes, condition variables and close file
        freeShmem(shmem,id);
        exit(1);
    } else if (idProcO == 0) {
        // Oxygen process
        OxygenFunc(shmem);
    } else {
        // Creating Hydrogen process
        int idProcH = fork();
        if (idProcH == -1) { //forking error occurred
            fprintf(stderr,"Fork error for Hydrogen process\n");
            shmem->error = true;
            wait(NULL);
            freeSem(shmem);  // free semaphores, mutexes, condition variables and close file
            freeShmem(shmem,id);
            exit(1);
        } else if (idProcH == 0) {
            // Hydrogen process
            HydrogenFunc(shmem);
        } else {
            // Main process
            wait(NULL);
            wait(NULL);
            freeSem(shmem);  // free semaphores, mutexes, condition variables and close file
            freeShmem(shmem,id);
        }
    }
            
    return 0;
}
