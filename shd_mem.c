#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define BankAccount 0

void ChildProcess(int[], int);
void MumProcess(int[]);

sem_t *mutex;

int main(int argc, char *argv[]) {
  int ShmID;
  int *ShmPTR;
  pid_t pid;
  int status;

  if (argc != 3) {
    printf("usage:  ref_psdd <parents> <children> (where parent = [1 | 2], "
           "children = [N > 0]\n");
    exit(1);
  }

  ShmID = shmget(IPC_PRIVATE, 1 * sizeof(int),
                 IPC_CREAT | 0666); // shared memory for 1 integer
  if (ShmID < 0) {
    printf("*** shmget error (server) ***\n");
    exit(1);
  }
  printf("Process has received a shared memory of 1 integer...\n");

  ShmPTR = (int *)shmat(ShmID, NULL, 0);
  if (*ShmPTR == -1) {
    printf("*** shmat error (server) ***\n");
    exit(1);
  }

  // create, initialize semaphore
  if ((mutex = sem_open("examplesemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("semaphore initilization");
    exit(1);
  }

  ShmPTR[BankAccount] = 0; // Initializing BankAccount

  printf("Process has attached the shared memory...\n");
  pid = fork();

  if (pid < 0) {
    printf("fork error\n");
    exit(1);
  } else if (pid == 0) {
    ChildProcess(ShmPTR, 1);
    exit(0);
  } else {
    int localBalance;
    int random_deposit;
    int random_decision;
    srandom(getpid());
    while (true) {
      // ParentProcess
      sleep(random() % 6);
      sem_wait(mutex);
      printf("Dear Old Dad: Attempting to Check Balance\n");
      localBalance = ShmPTR[BankAccount];
      random_decision = random(); // decide to deposit or not
      if (random_decision % 2 == 0) {
        if (localBalance < 100) {
          random_deposit = random() % 101; // for 0-100 inclusive
          if (random_deposit % 2 == 0) {
            localBalance += random_deposit;
            printf("Dear old Dad: Deposits $%d / Balance = $%d\n",
                   random_deposit, localBalance);
          } else {
            printf("Dear old Dad: Doesn't have any money to give\n");
          }
        } else {
          printf("Dear old Dad: Thinks student has enough Cash ($%d)\n",
                 localBalance);
        }
      } else {
        printf("Dear old Dad: Last Checking Balance = $%d\n", localBalance);
      }
      ShmPTR[BankAccount] = localBalance;
      sem_post(mutex);
    }
    wait(&status);
    printf("Process has detected the completion of its child...\n");
    shmdt((void *)ShmPTR);
    printf("Process has detached its shared memory...\n");
    shmctl(ShmID, IPC_RMID, NULL);
    printf("Process has removed its shared memory...\n");
    printf("Process exits...\n");
    exit(0);
  }
}

void ChildProcess(int SharedMem[], int nth) {
  int localBalance;
  int random_withdrawal;
  int random_decision;
  srandom(getpid());
  while (true) {
    sleep(random() % 6);
    sem_wait(mutex);
    printf("Poor Student #%d: Attempting to Check Balance\n", nth);
    localBalance = SharedMem[BankAccount];
    random_decision = random(); // decide to withdraw or not
    if (random_decision % 2 == 0) {
      random_withdrawal = random() % 51; // for 0-50 inclusive
      printf("Poor Student needs $%d\n", random_withdrawal);
      if (random_withdrawal <= localBalance) {
        localBalance -= random_withdrawal;
        printf("Poor Student #%d: Withdraws $%d / Balance = $%d\n", nth,
               random_withdrawal, localBalance);
      } else {
        printf("Poor Student #%d: Not Enough Cash ($%d)\n", nth, localBalance);
      }
    } else {
      printf("Poor Student #%d: Last Checking Balance = $%d\n", nth,
             localBalance);
    }
    SharedMem[BankAccount] = localBalance;
    sem_post(mutex);
  }
}