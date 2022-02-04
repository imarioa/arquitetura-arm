#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ERROR -1
#define ISVALID 1
#define LASTBITS 9,9
// L1 page types
#define COARSE 1
#define SECTION 2
#define FINE 3
/****************************/
// L2 page types
#define LARGE 1
#define SMALL 2
#define TINY 3
/****************************/
#define L1_OFFSET 2,4



long long int hexTodec(char *hex);
int searchL2(long long int end, int index);

typedef struct 
{
    int hit;   
    int miss;    
    int tag;
} Cache;

Cache memory[256];
long long int * L1;
long long int * L2;

void inicialize(Cache * memory, int size){
    
    for(int i = 0; i < size; i++){
        memory[i].hit = 0;
        memory[i].miss = 0;
        memory[i].tag = -1;
    }
}

void check_cache(long long int end){

    int index = (end & 0x00000FF0) >> 4;
    long int tag = (end & 0xFFFFF000) >> 12;
    
    if(memory[index].tag == tag){
        printf("Cache Hit!\n");
        memory[index].hit++;
    }else{
        printf("Cache MISS!\n");
        memory[index].miss++;
        memory[index].tag = tag;
    }
}

void loadCache(FILE * arq, char * filename, long long int * cache){
    char conteudo[12];
    int i = 0;
    arq = fopen(filename,"rt");

    while (fgets(conteudo,12,arq) != NULL)
    {   
        cache[i] = hexTodec(conteudo);
        i++;
    }
    fclose(arq);
}

int MMU(long long int end){
    int L1_index = (end & 0xFFF00000) >> 20;
    long long int PTE = L1[L1_index];
    int l_bits = (PTE & 0x3);
    int index_L2;
    
    if(l_bits == COARSE){ //Coarse Table
        index_L2 = (end & 0x000FF000) >> 12;
        searchL2(end, index_L2);
    }else if(l_bits == SECTION){//Section entry 
        check_cache((PTE & 0xFFF00000) | (end & 0x000FFFFF));
    }else if(l_bits == FINE){//Fine table
        index_L2 = (end & 0x000FFC00) >> 10;
        searchL2(end, index_L2);
    }else{
        //printf("Page fault!\n");
    }
} 
int searchL2(long long int end, int index){ 
    long long int PTE = L2[index]; 
    long long int l_bits = (PTE & 0x3);
    
    if(l_bits == LARGE){
      check_cache((PTE & 0xFFFF0000) | (end & 0x0000FFFF));
    }else if(l_bits == SMALL){  
      check_cache((PTE & 0xFFFFF000) | (end & 0x00000FFF));
    }else if(l_bits == TINY){
      check_cache((PTE & 0xFFFFFC00) | (end & 0x000003FF));
    }else{
        //printf("Page fault\n");
    }
}
long long int hexTodec(char *hex){
    long long int dec = 0;
    long long int base = 1;
    for(int i = strlen(hex) - 1; i >= 2; i--){
        if(hex[i] >= '0' && hex[i] <= '9'){
            dec += (hex[i] - 48) * base;
            base *= 16;
        }
        else if(hex[i] >= 'a' && hex[i] <= 'f')
        {
            dec += (hex[i] - 87) * base;
            base *= 16;
        }
    }
    return dec;
}
void showCache(){
    long long int last_addr = 0;
    long long int first_addr = 0;
    for(int i = 0; i < 256; i++){
        if(memory[i].tag != -1){
            first_addr = ((memory[i].tag << 12) | (i << 4));
            last_addr = ((memory[i].tag << 12) | (i << 4) | 0xF);
            printf("[0x%08x] [0x%08x - 0x%08x] H(%d) M(%d)\n",memory[i].tag, first_addr, last_addr,memory[i].hit, memory[i].miss);
          
        }
    }
}
void start(FILE * arq, char * filename){
    char conteudo[12];
    arq = fopen(filename,"rt");

    while (fgets(conteudo,12,arq) != NULL)
    {   
        MMU(hexTodec(conteudo));
    }
    fclose(arq);
}

int main(int argc, char *argv[]){
    inicialize(memory,256);
    FILE *arq;

    L1 = malloc(2048);
    loadCache(arq,argv[1],L1);

    L2 = malloc(2048);
    loadCache(arq,argv[2],L2);

    start(arq, argv[3]);
    showCache();
}