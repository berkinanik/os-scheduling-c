/*
Berkin ANIK
EE442 - Operating Systems HW1

solution to synchronization problem using only semaphores
to compile the program:
gcc -o output2.o main2.c -lpthread -lm
*/

// include the header files
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// type define atom struct
typedef struct atom {
  int atomID;
  char atomTYPE; // C, H, O or N
} atom;

// enums for molecule types
enum molecule_type {
  H2O = 1,
  CO2 = 2,
  NO2 = 3,
  NH3 = 4,
};

// type define information struct
typedef struct Information {
  int moleculeTYPE;
} Information;

// atom count for generating atom id
int atomCount = 0;

// function to generate a new atom with given atom type
atom *generateAtom(char atomTYPE) {
  atomCount++;
  atom *newAtom = (atom *)malloc(sizeof(atom));
  newAtom->atomID = atomCount;
  newAtom->atomTYPE = atomTYPE;
  return newAtom;
}

// function to generate random exponential distribution
double generateExponentialDistribution(double lambda) {
  double u = (double)rand() / (double)RAND_MAX;
  return -log(1 - u) / lambda;
}

// function to generate a random atom type of C, H, N or O and return it
char generateRandomAtomType() {
  int random = rand() % 4;
  switch (random) {
  case 0:
    return 'C';
  case 1:
    return 'H';
  case 2:
    return 'N';
  case 3:
    return 'O';
  default:
    return 'C';
  }
}

char *getMoleculeName(int moleculeType) {
  switch (moleculeType) {
  case CO2:
    return "CO2";
    break;
  case NO2:
    return "NO2";
    break;
  case NH3:
    return "NH3";
    break;
  case H2O:
    return "H2O";
    break;
  default:
    return "";
    break;
  }
}

// initialize semaphores for each atom type
sem_t C_semaphore;
sem_t H_semaphore;
sem_t N_semaphore;
sem_t O_semaphore;

// initialize semaphores for each molecule type
sem_t CO2_semaphore;
sem_t NO2_semaphore;
sem_t NH3_semaphore;
sem_t H2O_semaphore;

// initialize a semaphore for the information
sem_t info_semaphore;

// initialize a semaphore for the atom count
sem_t atom_count_semaphore;

Information *info = NULL;

// global default variables for command-line arguments
int NUM_M = 40; // number of atom type
int GENERATION_RATE = 100;
// counters for global number of atoms
int NUM_C = 0;
int NUM_H = 0;
int NUM_N = 0;
int NUM_O = 0;

// integers to keep track of exited status of molecule threads
int CO2_exited = 0;
int NO2_exited = 0;
int NH3_exited = 0;
int H2O_exited = 0;

// function to generate an atom with given type for each atom type thread
void *atomThreadFunc(void *arg) {
  // sleep for a random time
  double duration = generateExponentialDistribution(GENERATION_RATE);
  sleep(duration);
  atom *newAtom = (atom *)malloc(sizeof(atom));
  char atomTYPE = *(char *)arg;
  sem_wait(&atom_count_semaphore);
  switch (atomTYPE) {
  case 'C':
    // increase the semaphore count for C atom
    sem_post(&C_semaphore);
    newAtom = generateAtom('C');
    break;
  case 'H':
    // increase the semaphore count for H atom
    sem_post(&H_semaphore);
    newAtom = generateAtom('H');
    break;
  case 'N':
    // increase the semaphore count for N atom
    sem_post(&N_semaphore);
    newAtom = generateAtom('N');
    break;
  case 'O':
    // increase the semaphore count for O atom
    sem_post(&O_semaphore);
    newAtom = generateAtom('O');
    break;
  default:
    break;
  }
  sem_post(&atom_count_semaphore);
  // print the atom
  printf("%c with ID: %d is created.\n", newAtom->atomTYPE, newAtom->atomID);
  // free the atom
  free(newAtom);
  pthread_exit(0);
}

// function to reset counts and release unsed atom semaphores
void resetAndRelease(int *c_count, int *h_count, int *n_count, int *o_count,
                     int moleculeCreated) {
  // if molecule is not created release semaphores
  if (moleculeCreated == 0) {
    for (int i = 0; i < *c_count; i++) {
      sem_post(&C_semaphore);
    }
    for (int i = 0; i < *h_count; i++) {
      sem_post(&H_semaphore);
    }
    for (int i = 0; i < *n_count; i++) {
      sem_post(&N_semaphore);
    }
    for (int i = 0; i < *o_count; i++) {
      sem_post(&O_semaphore);
    }
  }
  *c_count = 0;
  *h_count = 0;
  *n_count = 0;
  *o_count = 0;
}

// function to generate a molecule with given type for each molecule type thread
void *moleculeThreadFunc(void *arg) {
  // count each available atom type
  int *c_count = (int *)malloc(sizeof(int));
  int *h_count = (int *)malloc(sizeof(int));
  int *n_count = (int *)malloc(sizeof(int));
  int *o_count = (int *)malloc(sizeof(int));
  *c_count = 0;
  *h_count = 0;
  *n_count = 0;
  *o_count = 0;
  int moleculeCreated = 0;
  int molecule_type = *(int *)arg;
  while (1) {
    switch (molecule_type) {
    case H2O:
      // wait for 2 H atoms
      if (sem_trywait(&H_semaphore) == 0) {
        (*h_count)++;
      }
      if (sem_trywait(&H_semaphore) == 0) {
        (*h_count)++;
      }
      // wait for 1 O atom
      if (sem_trywait(&O_semaphore) == 0) {
        (*o_count)++;
      }
      // check if molecule is ready
      if (*h_count == 2 && *o_count == 1) {
        moleculeCreated = 1;
      }
      // reset atom counts and release unused atom semaphores
      resetAndRelease(c_count, h_count, n_count, o_count, moleculeCreated);
      // molecule is ready increase the semaphore count for H2O
      if (moleculeCreated == 1) {
        // wait for info semaphore
        sem_wait(&info_semaphore);
        // set the molecule type
        info->moleculeTYPE = H2O;
        moleculeCreated = 0;
        sem_post(&H2O_semaphore);
      }
      break;
    case CO2:
      // wait for 2 O atoms
      if (sem_trywait(&O_semaphore) == 0) {
        (*o_count)++;
      }
      if (sem_trywait(&O_semaphore) == 0) {
        (*o_count)++;
      }
      // wait for 1 C atom
      if (sem_trywait(&C_semaphore) == 0) {
        (*c_count)++;
      }
      // check if molecule is ready
      if (*c_count == 1 && *o_count == 2) {
        moleculeCreated = 1;
      }
      // reset atom counts and release unused atom semaphores
      resetAndRelease(c_count, h_count, n_count, o_count, moleculeCreated);
      // molecule is ready increase the semaphore count for CO2
      if (moleculeCreated == 1) {
        // wait for info semaphore
        sem_wait(&info_semaphore);
        // set the molecule type
        info->moleculeTYPE = CO2;
        moleculeCreated = 0;
        sem_post(&CO2_semaphore);
      }
      break;
    case NO2:
      // wait for 2 O atoms
      if (sem_trywait(&O_semaphore) == 0) {
        (*o_count)++;
      }
      if (sem_trywait(&O_semaphore) == 0) {
        (*o_count)++;
      }
      // wait for 1 N atom
      if (sem_trywait(&N_semaphore) == 0) {
        (*n_count)++;
      }
      // check if molecule is ready
      if (*n_count == 1 && *o_count == 2) {
        moleculeCreated = 1;
      }
      // reset atom counts and release unused atom semaphores
      resetAndRelease(c_count, h_count, n_count, o_count, moleculeCreated);
      // molecule is ready increase the semaphore count for NO2
      if (moleculeCreated == 1) {
        // wait for info semaphore
        sem_wait(&info_semaphore);
        // set the molecule type
        info->moleculeTYPE = NO2;
        moleculeCreated = 0;
        sem_post(&NO2_semaphore);
      }
      break;
    case NH3:
      // wait for 3 H atoms
      if (sem_trywait(&H_semaphore) == 0) {
        (*h_count)++;
      }
      if (sem_trywait(&H_semaphore) == 0) {
        (*h_count)++;
      }
      if (sem_trywait(&H_semaphore) == 0) {
        (*h_count)++;
      }
      // wait for 1 N atom
      if (sem_trywait(&N_semaphore) == 0) {
        (*n_count)++;
      }
      // check if molecule is ready
      if (*n_count == 1 && *h_count == 3) {
        moleculeCreated = 1;
      }
      // reset atom counts and release unused atom semaphores
      resetAndRelease(c_count, h_count, n_count, o_count, moleculeCreated);
      // molecule is ready increase the semaphore count for NH3
      if (moleculeCreated == 1) {
        // wait for info semaphore
        sem_wait(&info_semaphore);
        // set the molecule type
        info->moleculeTYPE = NH3;
        moleculeCreated = 0;
        sem_post(&NH3_semaphore);
      }
      break;
    default:
      break;
    }
    // check if the total atom count is maximum
    // if yes thread cannot create more molecules
    // wait for atom count semaphore
    if (atomCount == NUM_M) {
      // break to the loop and exit the thread
      // update molecule exited status
      switch (molecule_type) {
      case H2O:
        H2O_exited = 1;
        break;
      case CO2:
        CO2_exited = 1;
        break;
      case NO2:
        NO2_exited = 1;
        break;
      case NH3:
        NH3_exited = 1;
        break;
      default:
        break;
      }
      break;
    }
  }
  // clean up
  free(c_count);
  free(h_count);
  free(n_count);
  free(o_count);
  pthread_exit(0);
}

// main info thread function
void *infoThreadFunc(void *arg) {
  // variables for atom counts
  int c_count = 0;
  int h_count = 0;
  int n_count = 0;
  int o_count = 0;
  while (1) {
    // check if any molecule semaphore is available
    // if yes, print info of created molecule and increase the info semaphore
    if (sem_trywait(&CO2_semaphore) == 0 || sem_trywait(&H2O_semaphore) == 0 ||
        sem_trywait(&NO2_semaphore) == 0 || sem_trywait(&NH3_semaphore) == 0) {
      // print info
      printf("%s is generated.\n", getMoleculeName(info->moleculeTYPE));
      switch (info->moleculeTYPE) {
      case H2O:
        h_count++;
        h_count++;
        o_count++;
        break;
      case CO2:
        c_count++;
        o_count++;
        o_count++;
        break;
      case NO2:
        n_count++;
        o_count++;
        o_count++;
        break;
      case NH3:
        n_count++;
        h_count++;
        h_count++;
        h_count++;
        break;
      default:
        break;
      }
      // increase the info semaphore
      sem_post(&info_semaphore);
    } else if (H2O_exited == 1 && CO2_exited == 1 && NO2_exited == 1 &&
               NH3_exited == 1) {
      // if all molecules are created, break the loop
      printf("All possible molecules are created.\n");
      printf("Number of atoms remaining for each atom: C: %d, H: %d, N: %d, O: "
             "%d\n",
             NUM_M / 4 - c_count, NUM_M / 4 - h_count, NUM_M / 4 - n_count,
             NUM_M / 4 - o_count);
      break;
    }
  }
  // clean up and exit the the app
  free(info);
  exit(0);
}

// main function
int main(int argc, char *argv[]) {
  // take input from command line using getopt
  int opt;
  while ((opt = getopt(argc, argv, "mg")) != -1) {
    switch (opt) {
    case 'm':
      NUM_M = atoi(argv[2]);
      break;

    case 'g':
      GENERATION_RATE = atoi(argv[4]);
      break;
    }
  }

  // print information about number of atoms and generation rate
  printf("GENERATION_RATE: %d\n", GENERATION_RATE);
  printf("TOTAL_ATOM_COUNT: %d\n", NUM_M);

  // create the info variable
  info = (Information *)malloc(sizeof(Information));
  info->moleculeTYPE = 0;

  // initialize semaphores for atoms from 0
  sem_init(&C_semaphore, 0, 0);
  sem_init(&H_semaphore, 0, 0);
  sem_init(&N_semaphore, 0, 0);
  sem_init(&O_semaphore, 0, 0);

  // initialize semaphores for molecules from 0
  sem_init(&H2O_semaphore, 0, 0);
  sem_init(&CO2_semaphore, 0, 0);
  sem_init(&NO2_semaphore, 0, 0);
  sem_init(&NH3_semaphore, 0, 0);

  // initialize the semaphore for the info semaphore from 1
  sem_init(&info_semaphore, 0, 1);

  // initialize the semaphore for the atom count semaphore from 1
  sem_init(&atom_count_semaphore, 0, 1);

  // create threads for total number of atoms
  pthread_t *atomThreads = (pthread_t *)malloc(sizeof(pthread_t) * NUM_M);

  // create threads for each molecule type
  pthread_t H20_thread;
  pthread_t CO2_thread;
  pthread_t NO2_thread;
  pthread_t NH3_thread;

  // create molecule argument for each thread
  int *H20_arg = (int *)malloc(sizeof(int));
  *H20_arg = H2O;
  int *CO2_arg = (int *)malloc(sizeof(int));
  *CO2_arg = CO2;
  int *NO2_arg = (int *)malloc(sizeof(int));
  *NO2_arg = NO2;
  int *NH3_arg = (int *)malloc(sizeof(int));
  *NH3_arg = NH3;

  // create threads for each molecule type
  pthread_create(&H20_thread, NULL, &moleculeThreadFunc, H20_arg);
  pthread_create(&CO2_thread, NULL, &moleculeThreadFunc, CO2_arg);
  pthread_create(&NO2_thread, NULL, &moleculeThreadFunc, NO2_arg);
  pthread_create(&NH3_thread, NULL, &moleculeThreadFunc, NH3_arg);

  // create atom type argument for each thread
  char *C_arg = (char *)malloc(sizeof(char));
  *C_arg = 'C';
  char *H_arg = (char *)malloc(sizeof(char));
  *H_arg = 'H';
  char *N_arg = (char *)malloc(sizeof(char));
  *N_arg = 'N';
  char *O_arg = (char *)malloc(sizeof(char));
  *O_arg = 'O';

  // create the main info thread
  pthread_t infoThread;

  // create the main info thread
  pthread_create(&infoThread, NULL, &infoThreadFunc, NULL);

  // create atom threads
  for (int i = 0; i < NUM_M; i++) {
    // get a random atom type
    char atomType = generateRandomAtomType();
    int createdSuccessfully = 0;
    while (createdSuccessfully == 0) {
      switch (atomType) {
      case 'C':
        if (NUM_C < NUM_M / 4) {
          NUM_C++;
          createdSuccessfully = 1;
          pthread_create(&atomThreads[i], NULL, &atomThreadFunc, C_arg);
        }
        break;
      case 'H':
        if (NUM_H < NUM_M / 4) {
          NUM_H++;
          createdSuccessfully = 1;
          pthread_create(&atomThreads[i], NULL, &atomThreadFunc, H_arg);
        }
        break;
      case 'O':
        if (NUM_O < NUM_M / 4) {
          NUM_O++;
          createdSuccessfully = 1;
          pthread_create(&atomThreads[i], NULL, &atomThreadFunc, O_arg);
        }
        break;
      case 'N':
        if (NUM_N < NUM_M / 4) {
          NUM_N++;
          createdSuccessfully = 1;
          pthread_create(&atomThreads[i], NULL, &atomThreadFunc, N_arg);
        }
        break;
      default:
        break;
      }
      if (createdSuccessfully == 0) {
        atomType = generateRandomAtomType();
      }
    }
  }

  // join the main info thread
  pthread_join(infoThread, NULL);

  // join all atoms threads
  for (int i = 0; i < NUM_M; i++) {
    if (pthread_join(atomThreads[i], NULL) != 0) {
      printf("Error joining atom thread\n");
      exit(1);
    }
  }

  // join all molecule threads
  if (pthread_join(H20_thread, NULL) != 0) {
    printf("Error joining H20 thread\n");
    exit(1);
  }
  if (pthread_join(CO2_thread, NULL) != 0) {
    printf("Error joining CO2 thread\n");
    exit(1);
  }
  if (pthread_join(NO2_thread, NULL) != 0) {
    printf("Error joining NO2 thread\n");
    exit(1);
  }
  if (pthread_join(NH3_thread, NULL) != 0) {
    printf("Error joining NH3 thread\n");
    exit(1);
  }

  // destroy all semaphores
  sem_destroy(&C_semaphore);
  sem_destroy(&H_semaphore);
  sem_destroy(&N_semaphore);
  sem_destroy(&O_semaphore);
  sem_destroy(&H2O_semaphore);
  sem_destroy(&CO2_semaphore);
  sem_destroy(&NO2_semaphore);
  sem_destroy(&NH3_semaphore);
  sem_destroy(&info_semaphore);
  sem_destroy(&atom_count_semaphore);
}
