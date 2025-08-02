#include<iostream>
#include<vector>
#include<map>
#include<string>
#include"config.h"
#include <fstream>
#include<bitset>
//#include <sstream>
#include <string>
#include<cmath>

using namespace std;
//class for main memory implementation
class mainMemeory{
public:
vector <bool> memoryBlock; // main memory implementation using vector
    mainMemeory(size_t totalPages) : memoryBlock(totalPages, true){};
    // function to allocate frame when new request comes
    size_t allocatePage(){
        for(int i=0; i < PHYSICAL_TOTAL_FRAMES; ++i){
            if(memoryBlock[i]){
                memoryBlock[i] = false;
                return i;
            }
        }
        return -1;
    }
    //find number of free frames in main memory at any instance
    size_t availableMemory(){
        size_t availablePages = 0;
        for(int i=0; i < PHYSICAL_TOTAL_FRAMES; ++i){
            if(memoryBlock[i]){
                availablePages += 1;
            }
        }
        return availablePages*PAGE_SIZE;
    }
};
// Task module
class Task{
private: //some private variables to keep track of different records
    mainMemeory & mainMemoryManager;
    map<int,int> pageTable; //page table implementd using map<virtual address, physical addres>
    size_t totalMemoryAllocated;
    size_t pageRequested = 0; //keep track the total pages requested by a task
    size_t pageHit = 0;
    size_t pageMiss = 0;
    size_t lengthOfOffsetAddress =0;
public:
int lengthOfOffset;
    string task_id;
    Task(const std::string& id, mainMemeory& mem_mgr) 
        : task_id(id), mainMemoryManager(mem_mgr), totalMemoryAllocated(0) {
            lengthOfOffset = PAGE_SIZE;
            lengthOfOffsetAddress = log2(lengthOfOffset);
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

    size_t rermoveOffset(size_t logicalAddress){
        bitset<48> binary(logicalAddress);
        //cout<<"\naddress: "<<binary;
        return binaryToDecimal(binary, lengthOfOffsetAddress, 47);
    }
    void memoryRequest(size_t logicalAddress, size_t requestedMemory){
        logicalAddress = rermoveOffset(logicalAddress);
        int pageNumbers = requestedMemory/PAGE_SIZE;
        int temp = requestedMemory % PAGE_SIZE;
        if(temp > 0) pageNumbers++; //find how many pages are requested by a task
        //cout<<"\nno of page requested are: "<<pageNumbers<<", requested memory"<<requestedMemory;
        for(int i=0; i<pageNumbers; ++i){
            pageRequested += 1;
            size_t logicalPageAddress = logicalAddress + i;
            //cout<<logicalPageAddress<<"\n";
            if(pageTable.find(logicalPageAddress) != pageTable.end()){
                pageHit += 1;
                //cout<<"page table hit";
            }else{
                size_t physicalPageAddress = mainMemoryManager.allocatePage();
                if(physicalPageAddress != -1){
                    pageMiss += 1;
                    // cout<<"\n"<<logicalPageAddress;
                    pageTable[logicalPageAddress] = physicalPageAddress;
                    totalMemoryAllocated += PAGE_SIZE;
                }else{
                    cout<<"no free memory available";
                }
            }
        }
    }
    void pageTablePerformance(){
        cout<<"\n"<<task_id<<": \nPage Hits: "<<pageHit;
        cout<<"\nRequested Pages: "<<pageRequested;
        float pageHitRate = pageHit / static_cast<float>(pageRequested);
        cout << "\nPage Hit Rate is: " << pageHitRate << " & Page Miss Rate is: " << 1 - pageHitRate << endl;
    }
    size_t memoryRequiredByPageTable(){
        size_t totalSize = VIRTUAL_MEMORY_PAGES*PAGE_TABLE_ENTERY_SIZE;
        return totalSize;
    }
    void printPageTable(){
        cout << task_id << " Page Table:\n";
        for (const auto& entry : pageTable) {
            std::cout << "Logical address: " << std::hex << entry.first << " -> Physical page: " << entry.second << std::dec << "\n";
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
    //cout<<"value of request:" <<value;
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
