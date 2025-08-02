#include <iostream>
#include <vector>
#include <cmath>
#include <bitset>
#include <algorithm>
#include <fstream>
//#include <sstream>
#include <string>

#include "config.h" //this includes necessary constants

using namespace std;
// class to define the structure and space for main memory
class mainMemeory {
public:
    vector<bool> memoryBlock;
    //implement the memory using vector and take boolean value to represent free and filled frames
    mainMemeory(size_t totalPages) : memoryBlock(totalPages, true) {}
    // function to allocate new frame to new request
    size_t allocatePage() {
        for (size_t i = 0; i < memoryBlock.size(); ++i) {
            if (memoryBlock[i]) {
                memoryBlock[i] = false;
                return i;
            }
        }
        return -1;
    }
    // function to find out available memory at any point of time
    size_t availableMemory() {
        return count(memoryBlock.begin(), memoryBlock.end(), true) * PAGE_SIZE;
    }
};
// task class to manage the request fro tasks
class Task {
private:
    mainMemeory& mainMemoryManager;
    size_t totalMemoryAllocated;
    size_t pageRequested = 0;
    int pageHit = 0;
    size_t pageMiss = 0;
    int lengthOfVirtualAddress = 0;
    int lengthOfOffsetAddress = 0;
    int lengthOfInnerLevelAddress = 0;
    int lengthOfLastLevelAddress = 0;
    vector<vector<vector<size_t>>> pageTable; //implement multilevel page table using vector of 3D
    
public:
    unsigned long long virtualMemorySize;
    int lengthOfOffset, pageTableEntrySize, numberOfEntryPerPage, numberOfPageAtFirstLevel, numberOfPageAtSecondLevel;
    string task_id;
    
    Task(const string& id, mainMemeory& mem_mgr)
        : task_id(id), mainMemoryManager(mem_mgr), totalMemoryAllocated(0) {
        virtualMemorySize = static_cast<unsigned long long>(VIRTUAL_MEMORY_PAGES) * PAGE_SIZE;
        lengthOfOffset = PAGE_SIZE;
        pageTableEntrySize = PAGE_TABLE_ENTERY_SIZE / 8;
        numberOfEntryPerPage = PAGE_SIZE / pageTableEntrySize;
        numberOfPageAtFirstLevel = VIRTUAL_MEMORY_PAGES / numberOfEntryPerPage;
        numberOfPageAtSecondLevel = numberOfPageAtFirstLevel / numberOfEntryPerPage;
        //cout<<"virtual memory size: "<<virtualMemorySize <<"\nsize of offset address:"<<lengthOfOffset<<"\nsize of each page table entry: "<<pageTableEntrySize
        //<<"\nnumber of entery per page at each level: "<<numberOfEntryPerPage<<"\nnumber of pages at first level: "<<numberOfPageAtFirstLevel
        //<<"\nnumber of pages at second level: "<<numberOfPageAtSecondLevel;
        
        // Now initialize pageTable with the correct sizes for all levels
        pageTable.resize(numberOfPageAtSecondLevel, 
            vector<vector<size_t>>(numberOfEntryPerPage, 
                vector<size_t>(numberOfEntryPerPage, -2)));

        lengthOfVirtualAddress =  log2(virtualMemorySize);
        lengthOfOffsetAddress = log2(lengthOfOffset);
        lengthOfInnerLevelAddress =  log2(numberOfEntryPerPage);
        lengthOfLastLevelAddress = lengthOfVirtualAddress - (lengthOfOffsetAddress + lengthOfInnerLevelAddress + lengthOfInnerLevelAddress);
        // cout<<"\n\nvirtual memory address length: "<<lengthOfVirtualAddress <<"\nlength of offset address:"<<lengthOfOffsetAddress<<"\nbits needed at first level: "<<lengthOfInnerLevelAddress
        // <<"\nbits required at second level: "<<lengthOfInnerLevelAddress<<"\nbits required at last level: "<<lengthOfLastLevelAddress;
    }
    //function to convert binary address to decimal value
    int binaryToDecimal(const bitset<48>& binary, int startIndex, int endIndex) {
        //cout<<"\naddress: "<<binary;
        //cout<<"\nstarting index: "<<startIndex<<"\nend index: "<<endIndex;
        int decimal = 0;
        for (int i = startIndex, j=0; i <= endIndex; ++i ,++j) {
            if (binary[i]) {
                decimal += (1 << j);
            }
            //cout<<"\n"<<decimal;
        }
        return decimal;
    }

    int indexAtFirstLevel, indexAtSecondLevel, indexAtLastLevel;
    //function to find the page table entery for respected request
    int searchPageTable(size_t logicalPageAddress) {
        bitset<48> binary(logicalPageAddress);
        indexAtFirstLevel = binaryToDecimal(binary, 0, lengthOfInnerLevelAddress-1);
        indexAtSecondLevel = binaryToDecimal(binary, lengthOfInnerLevelAddress, (2*lengthOfInnerLevelAddress)-1);
        indexAtLastLevel = binaryToDecimal(binary, (2*lengthOfInnerLevelAddress), 47);
        //cout<<"first level address: "<<indexAtFirstLevel<<"second level address: "<<indexAtSecondLevel<<"last level address: "<<indexAtLastLevel;
        return pageTable[indexAtLastLevel][indexAtSecondLevel][indexAtFirstLevel];
    }
    //function that removes offset bit from our virtual address this helps us to search the page table entry
    size_t rermoveOffset(size_t logicalAddress){
        bitset<48> binary(logicalAddress);
        //cout<<"\naddress: "<<binary;
        return binaryToDecimal(binary, lengthOfOffsetAddress, 47);
    }
    //task request any memory handeled by this function
    void memoryRequest(size_t logicalAddress, size_t requestedMemory) {
        int pageNumbers = requestedMemory/PAGE_SIZE;
        int temp = requestedMemory % PAGE_SIZE;
        if(temp > 0) pageNumbers++; //find out the number of pages requested by task
        logicalAddress = rermoveOffset(logicalAddress);
        for(int i=0; i<pageNumbers; ++i){
            pageRequested += 1;
            size_t logicalPageAddress = logicalAddress + i;
            //cout<<"\nrequested page address: "<<logicalPageAddress;
            int atIndex = searchPageTable(logicalPageAddress);
            //cout<<"value at requested index: "<<atIndex;
            if (atIndex != -2) { //true if page table hit
                pageHit++;
                //cout << "Page table hit\n";
            } else {
                size_t physicalPageAddress = mainMemoryManager.allocatePage(); //allocate new space in main memory
                if (physicalPageAddress != -1) {
                    pageMiss++;
                    pageTable[indexAtLastLevel][indexAtSecondLevel][indexAtFirstLevel] = physicalPageAddress; //fill page table entry in page table
                    totalMemoryAllocated += PAGE_SIZE;
                } else {
                    cout << "No free memory available\n"; //return this message if main memory is full
                }
            }
        }
        //cout<<"\nrequested pages :"<<pageRequested;
    }
    //function to keep track of performane
    void pageTablePerformance(){
        cout<<"\n"<<task_id<<": \nPage Hits: "<<pageHit;
        //cout<<"\nRequested Pages: "<<pageRequested;
        float pageHitRate = pageHit / static_cast<float>(pageRequested);
        cout << "\nPage Hit Rate is: " << pageHitRate << " & Page Miss Rate is: " << 1 - pageHitRate << endl;
    }
    //function for memory required by page table
    void memoryRequiredByPageTable() {
        cout<<"Memory required by Page Table: "<< numberOfEntryPerPage * numberOfPageAtSecondLevel * PAGE_TABLE_ENTERY_SIZE<<" Bytes";
    }

    void printPageTable() {
        for (int j = 0; j < numberOfPageAtSecondLevel; ++j) {
            for (int k = 0; k < numberOfPageAtFirstLevel; ++k) {
                for (int l = 0; l < numberOfEntryPerPage; ++l) {
                    if (pageTable[j][k][l] != -1) {
                        cout << "Logical address: " << hex << (j * numberOfPageAtFirstLevel + k) << " -> Physical page: " << pageTable[j][k][l] << dec << "\n";
                    }
                }
            }
        }
    }
};
int convertToSize(const string& sizeString) {
    size_t pos = 0;
    // Find the position where the digits end
    while (pos < sizeString.size() && isdigit(sizeString[pos])) {
        ++pos;
    }
    // Extract numeric part
    string numberPart = sizeString.substr(0, pos);
    // Extract unit part
    string unitPart = sizeString.substr(pos);
    // Convert numeric part to integer
    int value = stoi(numberPart);
    // Convert to kilobytes to bytes
    if (unitPart == "KB") {
        return value*1024;
    } else {
        cerr << "Unknown unit: " << unitPart << endl;
        return -1; // Error code
    }
}
int main() {
    mainMemeory memory_manager(PHYSICAL_TOTAL_FRAMES);
    Task task1("T1",memory_manager);
    Task task2("T2",memory_manager);
    Task task3("T3",memory_manager);
    Task task4("T4",memory_manager);
    Task task5("T5",memory_manager);
    Task task6("T6",memory_manager);
    Task task7("T7",memory_manager);
    Task task8("T8",memory_manager);
    Task task9("T9",memory_manager);
    Task task10("T10",memory_manager);
    std::ifstream infile("input_files/tracefile_4KB_4GB_4GB.txt"); // Open the file
    std::string line;

    while (std::getline(infile, line)) {
        // Example line: T6:0x03083400:10KB
        std::string thread_id, address, size;

        // Find positions of delimiters
        size_t pos1 = line.find(":");
        size_t pos2 = line.find(":", pos1 + 1);

        // Extract thread_id, address, and size
        if (pos1 != std::string::npos && pos2 != std::string::npos) {
            thread_id = line.substr(0, pos1);
            address = line.substr(pos1 + 1, pos2 - pos1 - 1);
            size = line.substr(pos2 + 1);

            // Now you can work with thread_id, address, and size
            unsigned long long baseAddress = std::stoull(address, nullptr, 16);
            int kilobytes = convertToSize(size);
            if(thread_id == "T1"){
                task1.memoryRequest(baseAddress, kilobytes);
            }
            if(thread_id == "T2"){
                task2.memoryRequest(baseAddress, kilobytes);
            }
            if(thread_id == "T3"){
                task3.memoryRequest(baseAddress, kilobytes);
            }
            if(thread_id == "T4"){
                task4.memoryRequest(baseAddress, kilobytes);
            }
            if(thread_id == "T5"){
                task5.memoryRequest(baseAddress, kilobytes);
            }
            if(thread_id == "T6"){
                task6.memoryRequest(baseAddress, kilobytes);
            }
            if(thread_id == "T7"){
                task7.memoryRequest(baseAddress, kilobytes);
            }
            if(thread_id == "T8"){
                task8.memoryRequest(baseAddress, kilobytes);
            }
            if(thread_id == "T9"){
                task9.memoryRequest(baseAddress, kilobytes);
            }
            if(thread_id == "T10"){
                task10.memoryRequest(baseAddress, kilobytes);
            }
        }
    }
    infile.close(); // Close the file
    task1.pageTablePerformance();
    task1.memoryRequiredByPageTable();

    task2.pageTablePerformance();
    task2.memoryRequiredByPageTable();

    task3.pageTablePerformance();
    task3.memoryRequiredByPageTable();

    task4.pageTablePerformance();
    task4.memoryRequiredByPageTable();

    task5.pageTablePerformance();
    task5.memoryRequiredByPageTable();

    task6.pageTablePerformance();
    task6.memoryRequiredByPageTable();

    task7.pageTablePerformance();
    task7.memoryRequiredByPageTable();

    task8.pageTablePerformance();
    task8.memoryRequiredByPageTable();

    task9.pageTablePerformance();
    task9.memoryRequiredByPageTable();

    task10.pageTablePerformance();
    task10.memoryRequiredByPageTable();

    size_t availabeMemory = memory_manager.availableMemory();
    cout<<"\nMemory available in Physical Memory: "<<availabeMemory<<" Bytes";
    return 0;
}
