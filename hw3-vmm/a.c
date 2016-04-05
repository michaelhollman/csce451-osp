#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define PAGE_SIZE 256
#define TLB_SIZE 16
#define MISS_OR_FAULT -1

int main(int argc, char **argv)
{
    // file names
    char *backing_store_file_name;
    char *addresses_file_name;
    
    // handle arguments
    if (argc != 3)
    {
        printf("Exactly 2 arguments must be provided.\n");
        printf("\t1: the backing store binary file\n");
        printf("\t2: the logical address input list\n");
        exit(1);
    }
    else
    {
        backing_store_file_name = argv[1];
        addresses_file_name = argv[2];
    }
    
    // counters for statistics
    int stats_translation_count = 0;
    int stats_page_fault_count = 0;
    int stats_tlb_hit_count = 0;
    
    // the actual chunk of memory that we'll load data into, 256 frames of 256 bytes
    char physical_memory[65536];
    
    // a counter to keep track of the next frame that should be used
    int physical_memory_next_frame = 0;

    // the TLB, an array of pairs
    // this is conceptually similar to a dictionary from higher languages
    int32_t tlb[TLB_SIZE][2];
    for (int i = 0; i < TLB_SIZE; i++)
    {
        // initialize to faults
        tlb[i][0] = MISS_OR_FAULT;
    }
    
    // used to cycle through tlb indices to accomplish a FIFO update policy
    int tlb_next_idx = 0;
    
    // the page table, just a simple map from page number to frame number
    int32_t page_table[PAGE_SIZE];
    for (int i = 0; i < PAGE_SIZE; i++)
    {
        // initialize to faults
        page_table[i] = MISS_OR_FAULT;
    }    
    
    // open backing_store_file_p now
    FILE *backing_store_file_p = fopen(backing_store_file_name, "r");
    if (backing_store_file_p == NULL)
    {
        printf("Unable to open backing store file.\n");
        exit(1);
    }
    
    // open and read addresses file
    FILE *addresses_file_p = fopen(addresses_file_name, "r");
    if (addresses_file_p == NULL)
    {
        printf("Unable to open address list file.\n");
        exit(1);
    }
    
    // read and iterate over addresses
    char line[7];
    while (fgets(line, 7, addresses_file_p) != NULL)
    {
        stats_translation_count++;

        size_t len = strlen(line);
        // replace ending \n with \0
        if (line[len-1] == '\n')
        {
            line[len-1] = '\0';    
        }
        int32_t address_raw = atoi(line);
        
        // parse logical address into page and offset
        int32_t page_number = (address_raw & 0x0000ff00) >> 8;
        int32_t offset = address_raw & 0x000000ff;
        int32_t frame_number = MISS_OR_FAULT;
        
        // check TLB (linearly)
        bool tlb_hit = false;
        for (int i = 0; i < TLB_SIZE && !tlb_hit; i++)
        {
            if (tlb[i][0] != page_number) 
            {
                continue;    
            }
            // else, hit!
            tlb_hit = true;
            stats_tlb_hit_count++;
            frame_number = tlb[i][1];
        }
        
        // if the TLB missed, check page table
        if (frame_number == MISS_OR_FAULT)
        {
            frame_number = page_table[page_number];
            if (frame_number == MISS_OR_FAULT)
            {
                stats_page_fault_count++;
            }            
        }
        
        // if the page table faulted, load new page into memory
        if (frame_number == MISS_OR_FAULT)
        {
            // seek to the correct location in backing_store_file_p
            int long seek_offset = page_number * PAGE_SIZE;
            int seek_result = fseek(backing_store_file_p, seek_offset, SEEK_SET);
            // sanity check… this is c after all
            if (seek_result != 0) 
            {
                printf("Error seeking within backing store file.");
                fclose(addresses_file_p);
                fclose(backing_store_file_p);
                exit(1);
            }
            
            // read the data straight into where we want it
            frame_number = physical_memory_next_frame++;
            fread(&physical_memory[frame_number * PAGE_SIZE], sizeof(char), PAGE_SIZE, backing_store_file_p);
            
            // update page table
            page_table[page_number] = frame_number;
        }
        
        // update TLB if it had missed
        if (!tlb_hit)
        {
            tlb[tlb_next_idx][0] = page_number;
            tlb[tlb_next_idx][1] = frame_number;
            tlb_next_idx++;
            tlb_next_idx %= TLB_SIZE;   
        }
        
        // reconstruct address
        int32_t address_translated = (frame_number << 8) | offset;
        
        // read value at memory location
        int32_t value = (int32_t) physical_memory[address_translated];
        
        // print out info
        printf("Virtual address: %d Physical address: %d Value: %d\n", address_raw, address_translated, value);
    }
    
    // calculate and print stats
    float stats_page_fault_rate = ((float) stats_page_fault_count) / stats_translation_count;
    float stats_tlb_hit_rate = ((float) stats_tlb_hit_count) / stats_translation_count;
    printf("Number of Translated Addresses = %d\n", stats_translation_count);
    printf("Page Faults = %d\n", stats_page_fault_count);
    printf("Page Fault Rate = %0.3f\n", stats_page_fault_rate);
    printf("TLB Hits = %d\n", stats_tlb_hit_count);
    printf("TLB Hit Rate = %0.3f\n", stats_tlb_hit_rate);    
    
    // cleanup
    fclose(addresses_file_p);
    fclose(backing_store_file_p);
}