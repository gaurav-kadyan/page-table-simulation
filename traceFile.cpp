#include <iostream>
#include <fstream>
#include <random>
#include <pthread.h>
#include<vector>
#include "config.h"

using namespace std;
// Function to generate a random aligned address
void generate_aligned_address(int arr[], uint64_t start_address, uint64_t end_address){
    uint64_t random_offset = (rand() % 4096) * PAGE_SIZE; // Generate a random offset in multiples of PAGE_SIZE
    if(start_address + random_offset > end_address){
        generate_aligned_address(arr, start_address, end_address);
    }
    int sizeOfPage = generate_size();
    if(start_address + random_offset + sizeOfPage > end_address){
        generate_aligned_address(arr, start_address, end_address);
    }
    arr[0] = start_address + random_offset;
    arr[1] = sizeOfPage;
}

// Function to generate a random size in KB
int generate_size() {
    int size = rand() % (16*PAGE_SIZE);
    return size;
}
void generateTrace(int threadId){
    ofstream traceFile("output Files/traceFile.txt");
    if(traceFile.is_open()){
        int arr[2];
        generate_aligned_address(arr, START_TEXT_SECTION, START_DATA_SECTION);
        traceFile<<"T"<<threadId<<":"<<"0x"<<arr[0]<<":"<<arr[1]<<"KB"<<"\n";
        generate_aligned_address(arr, START_DATA_SECTION, START_STACK_SECTION);
        traceFile<<"T"<<threadId<<":"<<"0x"<<arr[0]<<":"<<arr[1]<<"KB"<<"\n";
        generate_aligned_address(arr, START_STACK_SECTION, START_HEAP_SECTION);
        traceFile<<"T"<<threadId<<":"<<"0x"<<arr[0]<<":"<<arr[1]<<"KB"<<"\n";
        generate_aligned_address(arr, START_HEAP_SECTION, END_MEMORY_SECTION);
        traceFile<<"T"<<threadId<<":"<<"0x"<<arr[0]<<":"<<arr[1]<<"KB"<<"\n";
    }
}
int main() {
    const int num_threads = 5;  // Number of threads to create
    pthread_t threads[num_threads];  // Array to hold thread identifiers
    std::vector<int> thread_ids(num_threads);  // Vector to hold thread IDs

    // Create multiple threads
    for (int i = 0; i < num_threads; ++i) {
        thread_ids[i] = i + 1;  // Assign a thread ID (starting from 1)
        if (pthread_create(&threads[i], nullptr, generateTrace, thread_ids[i]) != 0) {
            std::cerr << "Error creating thread " << i + 1 << std::endl;
            return 1;
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; ++i) {
        if (pthread_join(threads[i], nullptr) != 0) {
            std::cerr << "Error joining thread " << i + 1 << std::endl;
            return 1;
        }
    }
    return 0;
}
