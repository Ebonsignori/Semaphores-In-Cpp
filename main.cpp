#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <algorithm>
#include <random>
#include <thread>

using namespace std;

// ==============================
// Method Signatures
// ==============================
// Thread Methods
void *producer(void *);
void *consumer(void *);

// Shared producer and consumer logic
bool shared_logic(int buffer_location, string buffer_from);

// Utility Methods
char randomAlphabeticCharacter();
void destroySemaphores(sem_t sems[], int num_of_sems);
bool continueRunningCheck(string current_char_sequence);

// ==============================
// Globals
// ==============================
// Semaphores
sem_t empty, full, mutex;
sem_t sems[3] = {empty, full, mutex};
// Buffer
const int BUFFER_SIZE = 2;
string BUFFER[BUFFER_SIZE];
const int PRODUCT_LENGTH = 3;
// Buffer producer and consumer pointers
int IN = 0;
int OUT = 0;
// User options
string user_input;
int selected_user_option = -1; // Can be either Option: 1 or 2
/*
 *  Options for run_mode can be:
 *  1 = Run indefinitely
 *  2 = Run until character sequence found
 *  3 = Run N times
 */
int run_mode = -1;
int current_iteration = 0;
int run_iterations = 0;
string stop_sequence = "xyz";


const bool IS_LOGGING = true; // Set to true to turn on console logging for debugging purposes

// ==============================
// Regex Globals
// ==============================


// ==============================
// Main Method
// ==============================
/* Entry point. Spawns consumer and producer threads and initializes semaphores */
int main() {
    // Get program print options from user input
    printf("Enter the desired program print option:\n"
           "1: Print results from producer before product is loaded into buffer.\n"
           "2: Print results from consumer after product is consumed from buffer.\n"
           "Your print option selection: ");
    bool invalid_input = true;
    while (invalid_input) {
        try {
            // Read in user input
            getline(cin, user_input);
            selected_user_option = stoi(user_input);
            user_input = "";
            printf("\n");
        } catch (...) {
            printf("Critical invalid user input. Exiting program...\n");
            exit(1); // Exit thread with unsuccessful 1 code on reading input error
        }
        if (selected_user_option != 1 && selected_user_option != 2) {
            printf("Invalid option. Please enter either 1 or 2.\n"
                   "Your print option selection: ");
        } else {
            printf("Print option %d selected.\n\n", selected_user_option);
            invalid_input = false;
        }
    }
    // Get program running options from user input
    printf("Enter the desired program runtime option:\n"
           "1: Run indefinitely (until cancelled with interrupt signal).\n"
           "2: Run until character sequence \"xyz\" is found.\n"
           "3: Run N times (iterations)\n"
           "Your runtime selection: ");
    invalid_input = true;
    while (invalid_input) {
        try {
            // Read in user input
            getline(cin, user_input);
            run_mode = stoi(user_input);
            user_input = "";
            printf("\n");
        } catch (...) {
            printf("Critical invalid user input. Exiting program...\n");
            exit(1); // Exit thread with unsuccessful 1 code on reading input error
        }
        if (run_mode != 1 && run_mode != 2 && run_mode != 3) {
            printf("Invalid option. Please enter either 1, 2, or 3.\n"
                   "Your runtime selection: ");
        } else {
            printf("Runtime option %d selected.\n\n", run_mode);
            invalid_input = false;
        }
    }
    // If run_mode is 3, get the number of times to run the program from user input
    if (run_mode == 3) {
        printf("Enter the number of N iterations to perform:\n"
               "Your N iterations selection: ");
        invalid_input = true;
        while (invalid_input) {
            try {
                // Read in user input
                getline(cin, user_input);
                run_iterations = stoi(user_input);
                user_input = "";
                printf("\n");
            } catch (...) {
                printf("Critical invalid user input. Exiting program...\n");
                exit(1); // Exit thread with unsuccessful 1 code on reading input error
            }
            if (run_iterations <= 0) {
                printf("Invalid option. Please enter a number of times to run that is greater than zero.\n"
                       "Your N iterations selection: ");
            } else {
                printf("%d iterations selected.\n\n", run_iterations);
                invalid_input = false;
            }
        }
    }

    if (IS_LOGGING) {
        printf("User Input selections: \n"
               "Print Option (1 or 2): %d\n"
               "Runtime Option (1, 2, or 3): %d\n",
               selected_user_option, run_mode);
        if (run_mode == 3) {
            printf("N iterations: %d\n", run_iterations);
        }

    }

    pthread_t producer_thread, consumer_thread;
    ulong producer_retval, consumer_retval;

    // Returns from pthread methods
    int producer_thread_created, consumer_thread_created;
    int producer_thread_joined, consumer_thread_joined;

    // Initialize Semaphores
    int empty_sem_result = sem_init(&empty, 0, BUFFER_SIZE);
    int full_sem_result = sem_init(&full, 0, 0);
    int mutex_sem_result = sem_init(&mutex, 0, 1);

    // Create producer and consumer threads
    producer_thread_created = pthread_create(&producer_thread, nullptr, producer, nullptr);
    consumer_thread_created = pthread_create(&consumer_thread, nullptr, consumer, nullptr);

    // If thread is created successfully
    if (producer_thread_created == 0 && consumer_thread_created == 0) {
        if (IS_LOGGING) {
            printf("Consumer and Producer threads spawned successfully.\n");
        }
    } else if (producer_thread_created == 1 || consumer_thread_created == 1) {
        printf("Error spawning threads from main module thread.\n");
        exit(1); // Exit with unsuccessful error code
    }

    // Join sifter thread with parent thread. Waits for results of sifter thread before continuing sequential execution
    producer_thread_joined = pthread_join(consumer_thread, (void **)&consumer_retval);
    consumer_thread_joined = pthread_join(producer_thread, (void **)&producer_retval);

    // ------------------------------
    // Handle Results Passed Through From Threads
    // ------------------------------
    // If both threads join successfully
    if (producer_thread_joined == 0 && consumer_thread_joined == 0) {
        if (IS_LOGGING) {
            printf("Consumer and Producer threads joined successfully.\n");
        }

        // Destroy Semaphores
        destroySemaphores(sems, 3);

        // Handle thread return (0 = exit successfully)
        if (consumer_retval == 0 && producer_retval == 0) {
            printf("Program quiting...");
            exit(0); // Exit with success code 0
        // 1 = uncaught error
        } else {
            printf("Something went wrong during program execution.\n"
                   "Please contact an admin with details of what you were doing at the time of crash.");
            // Destroy Semaphores
            destroySemaphores(sems, 3);
            exit(1); // Exit with error code 1
        }
    // If error joining threads, exit with error
    } else {
        printf("Error joining thread(s) with main module thread.\n");
        // Destroy Semaphores
        destroySemaphores(sems, 3);
        exit(1); // Exit with unsuccessful error code
    }
}

// ==============================
// Thread Methods
// ==============================
/* Consumes buffer contents if the buffer is filled
 *
 * Thread returns a uint code that is handled by the main method, with the following meanings:
 *   0 = successful exit (user keyboard interrupt)
 *   1 = uncaught error
 */
void* consumer(void *) {
    // Check if thread should continue iterations
    while (continueRunningCheck(BUFFER[OUT])) {
        // If this thread should wait, wait, otherwise execute critical section
        while (IN == OUT) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        shared_logic(OUT, "Consumer");
        OUT += 1;
    }

    pthread_exit((void *) nullptr); // Exit thread with successful code
}

/* Producers  buffer contents if the buffer is filled
 *
 * Thread returns a uint code that is handled by the main method, with the following meanings:
 *   0 = successful exit (user keyboard interrupt)
 *   1 = uncaught error
 */
void* producer(void*) {
    string product = "";

    // Check if thread should continue iterations
    while (continueRunningCheck(product)) {
        // If this thread should wait, wait, otherwise execute critical section
        while ((IN + 1) % BUFFER_SIZE == OUT ) {
            std::this_thread::sleep_for (std::chrono::seconds(1));
        }

        product = "";
        // Generate a string that is 3 random characters
        for (int i = 0; i < PRODUCT_LENGTH; i++) {
            product += randomAlphabeticCharacter();
        }

        // Load buffer with product
        BUFFER[IN] = product;

        shared_logic(IN, "Producer");

        // Increment IN pointer
        IN = (IN + 1) % BUFFER_SIZE;
    }


    pthread_exit((void *) nullptr); // Exit thread with successful code
}

// ==============================
// Shared Logic
// ==============================
bool shared_logic(int buffer_location, string buffer_from) {
    printf("%s: Buffer contents: %s\n", buffer_from.c_str(), BUFFER[buffer_location].c_str());


    return true; // If section 1 is valid, returns true with values for current_index and section_one_int set correctly
}


// ==============================
// Utility Methods
// ==============================
/* Returns a random alphabetic character */
std::random_device rd;
std::mt19937 rng(rd());
char randomAlphabeticCharacter() {
    // Generate a random integer 0 through 25
    std::uniform_int_distribution<int> uni(0, 25);
    auto random_integer = uni(rng);
    // Return alphabetic character corresponding to integer
    return "abcdefghijklmnopqrstuvwxyz"[random_integer];
}

void destroySemaphores(sem_t sems[], int num_of_sems) {
    for (int i = 0; i < num_of_sems; i++) {
        sem_destroy(&sems[i]);
    }
}

/* Checks if the calling producer or consumer should continue iterating and returns true if they should, false otherwise
 * Runtime conditions are as follows:
 *  1 = Run indefinitely
 *  2 = Run until character sequence found
 *  3 = Run N times
 * */
bool continueRunningCheck(string current_char_sequence) {
    // Skip other checks and return true if option 0
    if (run_mode == 0) {
        return true;
    }

    if (run_mode == 2 && current_char_sequence == stop_sequence) {
        return false;
    }

    if (run_mode == 3) {
        current_iteration++;
        if (current_iteration >= run_iterations) {
            return false;
        }
    }

    return true;
}