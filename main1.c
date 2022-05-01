/*
Berkin ANIK - 2397123
EE442 - Operating Systems HW1
*/

// include the header files
#include <math.h>
#include <pthread.h>
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

// type define test tube struct
typedef struct mytube {
  int tubeID;
  int tubeTS;       // time stamp (ID of the atom spilled first)
  int moleculeTYPE; // 1:H2O, 2:CO2, 3: NO2, 4: NH3
} mytube;

// enums for molecule types
enum molecule_type {
  EMPTY = 0,
  H2O = 1,
  CO2 = 2,
  NO2 = 3,
  NH3 = 4,
};

// type define information struct
typedef struct Information {
  int tubeID;
  mytube tube;
} Information;

// atom count for generating atom id
int atomCount = 0;
// tube count for genertaiing tube id
int tubeCount = 0;

// function to generate a new atom with given atom type
atom *generateAtom(char atomTYPE) {
  atomCount++;
  atom *newAtom = (atom *)malloc(sizeof(atom));
  newAtom->atomID = atomCount;
  newAtom->atomTYPE = atomTYPE;
  return newAtom;
}

// function to generate an empty test tube
mytube *generateTube() {
  tubeCount++;
  mytube *newTube = (mytube *)malloc(sizeof(mytube));
  newTube->tubeID = tubeCount;
  newTube->tubeTS = 0;
  newTube->moleculeTYPE = EMPTY;
  return newTube;
}

// typedef a linked list for atoms in each test tube
typedef struct atom_in_tube {
  atom atom;
  struct atom_in_tube *next;
} atom_in_tube;

// function to clear the test tube
// reset the time stamp and molecule type
// deallocate the memory of the atoms in the test tube
atom_in_tube *clearTube(mytube **tube, atom_in_tube *head) {
  (*tube)->tubeTS = 0;
  (*tube)->moleculeTYPE = EMPTY;
  atom_in_tube *curr = head;
  while (curr != NULL) {
    atom_in_tube *temp = curr;
    curr = curr->next;
    free(temp);
  }
  return NULL;
}

// function to update test tube molecule type for given two atoms
// 1: H and O -> H2O
// 2: C and O -> CO2
// 3: N and O -> NO2
// 4: N and H -> NH3
void updateTube(mytube **tube, atom atom1, atom atom2) {
  if ((atom1.atomTYPE == 'H' && atom2.atomTYPE == 'O') ||
      (atom1.atomTYPE == 'O' && atom2.atomTYPE == 'H')) {
    (*tube)->moleculeTYPE = H2O;
  } else if ((atom1.atomTYPE == 'C' && atom2.atomTYPE == 'O') ||
             (atom1.atomTYPE == 'O' && atom2.atomTYPE == 'C')) {
    (*tube)->moleculeTYPE = CO2;
  } else if ((atom1.atomTYPE == 'N' && atom2.atomTYPE == 'O') ||
             (atom1.atomTYPE == 'O' && atom2.atomTYPE == 'N')) {
    (*tube)->moleculeTYPE = NO2;
  } else if ((atom1.atomTYPE == 'N' && atom2.atomTYPE == 'H') ||
             (atom1.atomTYPE == 'H' && atom2.atomTYPE == 'N')) {
    (*tube)->moleculeTYPE = NH3;
  }
}

// function to check if an atom can react with a given atom
int checkReaction(atom atom1, atom atom2) {
  if ((atom1.atomTYPE == 'C' && atom2.atomTYPE == 'O') ||
      (atom1.atomTYPE == 'O' && atom2.atomTYPE == 'C')) {
    return 1;
  } else if ((atom1.atomTYPE == 'H' && atom2.atomTYPE == 'N') ||
             (atom1.atomTYPE == 'N' && atom2.atomTYPE == 'H')) {
    return 1;
  } else if ((atom1.atomTYPE == 'H' && atom2.atomTYPE == 'O') ||
             (atom1.atomTYPE == 'O' && atom2.atomTYPE == 'H')) {
    return 1;
  } else if ((atom1.atomTYPE == 'O' && atom2.atomTYPE == 'N') ||
             (atom1.atomTYPE == 'N' && atom2.atomTYPE == 'O')) {
    return 1;
  } else {
    return 0;
  }
}

// function to check if an atom is needed for given moleculeType
// if the atom is needed, return 1
// else if the reaction will be complete in next step, return 2
// else return 0
int checkAtomNeededAndUpdateMoleculeType(atom_in_tube *head, atom newAtom,
                                         mytube *tube) {
  atom_in_tube *current = (atom_in_tube *)malloc(sizeof(atom_in_tube));
  current->atom = head->atom;
  current->next = head->next;
  int existing_c_count = 0;
  int existing_h_count = 0;
  int existing_o_count = 0;
  int existing_n_count = 0;
  while (current != NULL) {
    if (current->atom.atomTYPE == 'C') {
      existing_c_count++;
    } else if (current->atom.atomTYPE == 'H') {
      existing_h_count++;
    } else if (current->atom.atomTYPE == 'O') {
      existing_o_count++;
    } else if (current->atom.atomTYPE == 'N') {
      existing_n_count++;
    }
    if (current->next == NULL) {
      break;
    } else {
      current->atom = current->next->atom;
      current->next = current->next->next;
    }
  }
  switch (tube->moleculeTYPE) {
  case EMPTY:
    if (checkReaction(newAtom, current->atom) == 1) {
      updateTube(&tube, newAtom, current->atom);
      return 1;
    } else {
      return 0;
    }
    break;
  case H2O:
    if (newAtom.atomTYPE == 'H' || newAtom.atomTYPE == 'O') {
      if (newAtom.atomTYPE == 'H' && existing_h_count < 2) {
        if (existing_h_count == 1 && existing_o_count == 1) {
          return 2;
        }
        return 1;
      } else if (newAtom.atomTYPE == 'O' && existing_o_count < 1) {
        if (existing_h_count == 2) {
          return 2;
        }
        return 1;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
    break;
  case CO2:
    if (newAtom.atomTYPE == 'C' || newAtom.atomTYPE == 'O') {
      if (newAtom.atomTYPE == 'C' && existing_c_count < 1) {
        if (existing_c_count == 0 && existing_o_count == 2) {
          return 2;
        }
        return 1;
      } else if (newAtom.atomTYPE == 'O' && existing_o_count < 2) {
        if (existing_c_count == 1 && existing_o_count == 1) {
          return 2;
        }
        return 1;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
    break;
  case NO2:
    if (newAtom.atomTYPE == 'N' || newAtom.atomTYPE == 'O') {
      if (newAtom.atomTYPE == 'N' && existing_n_count < 1) {
        if (existing_n_count == 0 && existing_o_count == 2) {
          return 2;
        }
        return 1;
      } else if (newAtom.atomTYPE == 'O' && existing_o_count < 2) {
        if (existing_n_count == 1 && existing_o_count == 1) {
          return 2;
        }
        return 1;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
    break;
  case NH3:
    if (newAtom.atomTYPE == 'N' || newAtom.atomTYPE == 'H') {
      if (newAtom.atomTYPE == 'N' && existing_n_count < 1) {
        if (existing_n_count == 0 && existing_h_count == 3) {
          return 2;
        }
        return 1;
      } else if (newAtom.atomTYPE == 'H' && existing_h_count < 3) {
        if (existing_n_count == 1 && existing_h_count == 2) {
          return 2;
        }
        return 1;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
    break;
  default:
    return 0;
    break;
  }
  free(current);
};

// function to add an atom to the test tube
int addAtom(mytube **tube, atom newAtom, atom_in_tube **head) {
  // if test tube is empty add new atom to the test tube and return 1
  // else check if the new atom is needed for the test tube
  // if yes add the new atom to the test tube, if molecule type is empty
  // update the molecule type, and return 1
  // else return 0
  if (*head == NULL) {
    atom_in_tube *newAtomInTube = (atom_in_tube *)malloc(sizeof(atom_in_tube));
    newAtomInTube->atom = newAtom;
    newAtomInTube->next = NULL;
    *head = newAtomInTube;
    (*tube)->tubeTS = newAtom.atomID;
    return 1;
  } else {
    int reactionResult =
        checkAtomNeededAndUpdateMoleculeType(*head, newAtom, *tube);
    if (reactionResult == 0) {
      return 0;
    } else {
      atom_in_tube *newAtomInTube =
          (atom_in_tube *)malloc(sizeof(atom_in_tube));
      newAtomInTube->atom = newAtom;
      newAtomInTube->next = *head;
      *head = newAtomInTube;
      return reactionResult;
    }
  }
}

// function to generate random exponential distribution
double generateExponentialDistribution(double lambda) {
  double u = (double)rand() / (double)RAND_MAX;
  return -log(1 - u) / lambda;
}

// function to get the test tube with smallest TS
int *getSmallestTS(mytube *tube1, mytube *tube2, mytube *tube3,
                   int *exceptTubeNo) {
  int *smallestTS = (int *)malloc(sizeof(int));
  if (exceptTubeNo == NULL) {
    if (tube1->tubeTS <= tube2->tubeTS && tube1->tubeTS <= tube3->tubeTS) {
      *smallestTS = 1;
    } else if (tube2->tubeTS <= tube1->tubeTS &&
               tube2->tubeTS <= tube3->tubeTS) {
      *smallestTS = 2;
    } else {
      *smallestTS = 3;
    }
  } else if (*exceptTubeNo == 1) {
    if (tube2->tubeTS <= tube3->tubeTS) {
      *smallestTS = 2;
    } else {
      *smallestTS = 3;
    }
  } else if (*exceptTubeNo == 2) {
    if (tube1->tubeTS <= tube3->tubeTS) {
      *smallestTS = 1;
    } else {
      *smallestTS = 3;
    }
  } else {
    if (tube1->tubeTS <= tube2->tubeTS) {
      *smallestTS = 1;
    } else {
      *smallestTS = 2;
    }
  }
  return smallestTS;
}

// generate a mutex for atom
pthread_mutex_t atomMutex;

// generate a mutex for info
pthread_mutex_t infoMutex;

// number for checking in info is updated
int infoUpdated = 0;

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

// global default variables for command-line arguments
int NUM_C = 20; // number of carbon atoms
int NUM_H = 20; // number of hydrogen atoms
int NUM_O = 20; // number of oxygen atoms
int NUM_N = 20; // number of nitrogen atoms
int GENERATION_RATE = 100;

int lastAtomChecked = 0;

// main function
int main(int argc, char *argv[]) {

  // take input from command line using getopt
  int opt;
  while ((opt = getopt(argc, argv, "chong")) != -1) {
    switch (opt) {
    case 'c':
      NUM_C = atoi(argv[2]);
      break;

    case 'h':
      NUM_H = atoi(argv[4]);
      break;

    case 'o':
      NUM_O = atoi(argv[6]);
      break;

    case 'n':
      NUM_N = atoi(argv[8]);
      break;

    case 'g':
      GENERATION_RATE = atoi(argv[10]);
      break;
    }
  }

  // print information about number of atoms and generation rate
  printf("GENERATION_RATE: %d\n", GENERATION_RATE);
  printf("CARBON ATOMS: %d\n", NUM_C);
  printf("HYDROGEN ATOMS: %d\n", NUM_H);
  printf("OXYGEN ATOMS: %d\n", NUM_O);
  printf("NITROGEN ATOMS: %d\n", NUM_N);

  int totalAtoms = NUM_C + NUM_H + NUM_O + NUM_N;

  // initialize 3 test tubes
  mytube *tube1 = generateTube();
  mytube *tube2 = generateTube();
  mytube *tube3 = generateTube();

  // initialize 3 atom_in_tube pointers
  atom_in_tube *head1 = NULL;
  atom_in_tube *head2 = NULL;
  atom_in_tube *head3 = NULL;

  // create the information struct
  Information *info = (Information *)malloc(sizeof(Information));

  // function to called by each generated atom thread
  void *atomThreadFunc(void *arg) {
    // lock the atom and info mutex
    pthread_mutex_lock(&atomMutex);
    pthread_mutex_lock(&infoMutex);
    atom *newAtom = (atom *)malloc(sizeof(atom));
    newAtom = (atom *)arg;
    // get the test tube with smallest TS
    int *smallestTubeNo = getSmallestTS(tube1, tube2, tube3, NULL);
    int additionResult = 0;
    int trial = 0;
    int firstTried = *smallestTubeNo;
    while (trial < 3) {
      if (*smallestTubeNo == 1) {
        additionResult = addAtom(&tube1, *newAtom, &head1);
      } else if (*smallestTubeNo == 2) {
        additionResult = addAtom(&tube2, *newAtom, &head2);
      } else {
        additionResult = addAtom(&tube3, *newAtom, &head3);
      }
      // if the atom is added successfully
      if (additionResult == 1) {
        break;
      } else if (additionResult == 2) {
        // update the info
        infoUpdated = 1;
        switch (*smallestTubeNo) {
        case 1:
          info->tubeID = 1;
          info->tube = *tube1;
          // clear the test tube
          head1 = clearTube(&tube1, head1);
          break;
        case 2:
          info->tubeID = 2;
          info->tube = *tube2;
          // clear the test tube
          head2 = clearTube(&tube2, head2);
          break;
        case 3:
          info->tubeID = 3;
          info->tube = *tube3;
          // clear the test tube
          head3 = clearTube(&tube3, head3);
          break;
        default:
          break;
        }
        break;
      }
      if (trial == 0) {
        smallestTubeNo = getSmallestTS(tube1, tube2, tube3, smallestTubeNo);
      } else if (trial == 1) {
        if (*smallestTubeNo == 1) {
          if (firstTried == 2) {
            *smallestTubeNo = 3;
          } else {
            *smallestTubeNo = 2;
          }
        } else if (*smallestTubeNo == 2) {
          if (firstTried == 1) {
            *smallestTubeNo = 3;
          } else {
            *smallestTubeNo = 1;
          }
        } else {
          if (firstTried == 1) {
            *smallestTubeNo = 2;
          } else {
            *smallestTubeNo = 1;
          }
        }
      }
      trial++;
    }
    if (newAtom->atomID == totalAtoms) {
      lastAtomChecked = 1;
    }
    // unlock the atom and info mutex
    pthread_mutex_unlock(&atomMutex);
    pthread_mutex_unlock(&infoMutex);
    free(newAtom);
    return NULL;
  };

  // function to called by info thread
  void *infoThreadFunc(void *arg) {
    int *totalCount;
    totalCount = (int *)arg;
    while (1) {
      // lock info mutex
      pthread_mutex_lock(&atomMutex);
      pthread_mutex_lock(&infoMutex);
      if (infoUpdated == 1) {
        // print created molecule type and tube id e.g.: NH3 is created in
        // tube 1.
        printf("%s is created in tube %d.\n",
               getMoleculeName(info->tube.moleculeTYPE), info->tubeID);
        // update infoUpdated
        infoUpdated = 0;
        // unlock info mutex
      }

      if (atomCount == *totalCount && lastAtomChecked == 1) {
        exit(0);
      }

      pthread_mutex_unlock(&atomMutex);
      pthread_mutex_unlock(&infoMutex);
    }
    return NULL;
  }

  // generate atom threads and info thread
  pthread_t atomThreads[totalAtoms];
  pthread_t infoThread;

  // initialize mutexes
  pthread_mutex_init(&atomMutex, NULL);
  pthread_mutex_init(&infoMutex, NULL);

  // create the info thread
  pthread_create(&infoThread, NULL, &infoThreadFunc, &totalAtoms);

  // count created atom types
  int countC = 0;
  int countH = 0;
  int countO = 0;
  int countN = 0;

  // create atom threads
  for (int i = 0; i < totalAtoms; i++) {
    // sleep for a random time
    double duration = generateExponentialDistribution(GENERATION_RATE);
    sleep(duration);
    // get a random atom type
    char atomType = generateRandomAtomType();
    int createdSuccessfully = 0;
    while (createdSuccessfully == 0) {
      switch (atomType) {
      case 'C':
        if (countC < NUM_C) {
          countC++;
          createdSuccessfully = 1;
        }
        break;
      case 'H':
        if (countH < NUM_H) {
          countH++;
          createdSuccessfully = 1;
        }
        break;
      case 'O':
        if (countO < NUM_O) {
          countO++;
          createdSuccessfully = 1;
        }
        break;
      case 'N':
        if (countN < NUM_N) {
          countN++;
          createdSuccessfully = 1;
        }
        break;
      default:
        break;
      }
      if (createdSuccessfully == 0) {
        atomType = generateRandomAtomType();
      }
    }

    // create an atom with the atom type
    atom *newAtom = generateAtom(atomType);
    printf("%c with ID:%d is created.\n", newAtom->atomTYPE, newAtom->atomID);
    // create thread for the atom
    pthread_create(&atomThreads[i], NULL, &atomThreadFunc, newAtom);
  }

  for (int i = 0; i < totalAtoms; i++) {
    if (pthread_join(atomThreads[i], NULL) != 0) {
      printf("Error joining thread\n");
      exit(1);
    }
  }
  pthread_join(infoThread, NULL);

  // destroy mutexes
  pthread_mutex_destroy(&atomMutex);
  pthread_mutex_destroy(&infoMutex);

  return 0;
}
