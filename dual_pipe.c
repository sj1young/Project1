/**************************************************************/
/* CS/COE 1541
   compile with gcc -o five_stage five_stage.c
   and execute using
   ./five_stage  /afs/cs.pitt.edu/courses/1541/short_traces/sample.tr	0
***************************************************************/

#include <stdio.h>
#include<stdlib.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <string.h>
#include "new_cpu.h"

int checkForALU(struct instruction *PCregister, struct instruction *ALU_ID_EX){
  printf("ALUCheck@ = %d\n",PCregister->PC);

  if(PCregister->type==ti_RTYPE){
    printf("found r-type");
    *ALU_ID_EX = *PCregister;
    PCregister->type=30;
    return 1;
  }
  if(PCregister->type==ti_ITYPE){
    printf("found i-type");
    *ALU_ID_EX = *PCregister;
    PCregister->type=30;
    return 1;
  }
  if(PCregister->type==ti_BRANCH){
    *ALU_ID_EX = *PCregister;
    PCregister->type=30;
    return 1;
  }
  if(PCregister->type==ti_SPECIAL){
    *ALU_ID_EX = *PCregister;
    PCregister->type=30;
    return 1;
  }
  if(PCregister->type==(ti_JTYPE||ti_JRTYPE)){
    *ALU_ID_EX = *PCregister;
    PCregister->type=30;
    return 1;
  }
  if(PCregister->type==(ti_NOP)){
    //printf("No-Op@ %d/n",PCregister->PC);
    *ALU_ID_EX = *PCregister;
    PCregister->type=30;
    return 1;
  }
  if(PCregister->type==30){
    printf("This shouldn't happen\n");
    return 1;
  }

  return 0;
}

int checkForLW(struct instruction *PCregister, struct instruction *LW_ID_EX){
printf("LWCheck@ %d\n\n\n\n\n\n",PCregister->PC);
  if(PCregister->type==ti_LOAD){
    *LW_ID_EX = *PCregister;
    PCregister->type=30;
    return 1;
  }
  if(PCregister->type==(ti_STORE)){
    *LW_ID_EX = *PCregister;
    PCregister->type=30;
    return 1;
  }
  if(PCregister->type==(ti_NOP)){
    *LW_ID_EX = *PCregister;
    PCregister->type=30;
    return 1;
  }
  return 0;
}

int main(int argc, char **argv)
{
  struct instruction *tr_entry1,*tr_entry2;
  struct instruction PCregister[2], IF_ID, ALU_ID_EX, LW_ID_EX,ALU_EX_EMPTY, LW_EX_MEM,ALU_EMPTY_WB, LW_MEM_WB;
  size_t size;
  char *trace_file_name;
  int trace_view_on = 0;
  int flush_counter = 5; //5 stage pipeline, so we have to execute 4 instructions once trace is done
  int ALUfilled = 0;
  int LWfilled = 0;
  int firstInsSent = 0;
  tr_entry1=0;
  tr_entry2=0;
  PCregister[0].type=0;
  PCregister[1].type=0;


  unsigned int cycle_number = 0;

  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }

  trace_file_name = argv[1];
  if (argc == 3) trace_view_on = atoi(argv[2]) ;

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();
  printf("Entering Loop\n\n\n\n");

  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  while(1) {
    printf("Check Buffers\n");
    if(tr_entry1==0){
      size = trace_get_item(&tr_entry1); /* put the instruction into a buffer */
      //printf("Getting something for 1\n");
    }
    if(tr_entry2==0){
      size = trace_get_item(&tr_entry2);
      //printf("Getting something for 2\n");
    }


    if (!size && flush_counter==0) {       /* no more instructions to simulate */
      printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      break;
    }
    else{              /* move the pipeline forward */
      cycle_number++;

      /* move instructions one stage ahead */
      LW_MEM_WB = LW_EX_MEM;
      LW_EX_MEM = LW_ID_EX;

      ALU_EMPTY_WB = ALU_EX_EMPTY;
      ALU_EX_EMPTY = ALU_ID_EX;


      /* Check first instruction in buffer to see if it can go*/
      ALUfilled = checkForALU(&PCregister[0],&ALU_ID_EX);
      if(ALUfilled){
        LWfilled = checkForLW(&PCregister[1],&LW_ID_EX);
      }
      else{
        ALUfilled = checkForALU(&PCregister[1],&ALU_ID_EX);
        LWfilled = checkForLW(&PCregister[0],&LW_ID_EX);
      }
      /*If one of the pipes isn't filled it throws in a no-op*/
      //if(ALUfilled==0)
        //ALU_ID_EX.type=ti_NOP;

      //if(LWfilled==0)
        //LW_ID_EX.type=ti_NOP;

        if(!size){    /* if no more instructions in trace, reduce flush_counter */
          flush_counter--;
        }
        /*Checks which PC Registers were used, and loads the tr_entry
        items in order*/
      if(PCregister[0].type==30 && PCregister[1].type==30)
      {
        memcpy(&PCregister[0], tr_entry1 , sizeof(PCregister[0]));
        memcpy(&PCregister[1], tr_entry2 , sizeof(PCregister[0]));
        printf("Copied to both reg\n");
        tr_entry1=0;
        tr_entry2=0;
      }
      else if(PCregister[0].type==30)
      {
        PCregister[0]=PCregister[1];
        memcpy(&PCregister[1], tr_entry1 , sizeof(PCregister[0]));
        tr_entry1=tr_entry2;
        tr_entry2=0;
        printf("Copied to first reg %d\n",PCregister[1].type);
      }
      else if(PCregister[1].type==30)
      {
        memcpy(&PCregister[1], tr_entry1 , sizeof(PCregister[0]));
        tr_entry1=tr_entry2;
        tr_entry2=0;
        printf("Copied to second reg\n");
      }
      printf("PC: %d %d,\t%d %d\n",PCregister[0].PC,PCregister[0].type,PCregister[1].PC,PCregister[1].type);
      printf("ALU: %d %d,\t%d %d,\t%d %d\n",ALU_ID_EX.PC,ALU_ID_EX.type,ALU_EX_EMPTY.PC,ALU_EX_EMPTY.type,ALU_EMPTY_WB.PC,ALU_EMPTY_WB.type);
      printf("LW: %d %d,\t%d %d,\t%d %d\n",LW_ID_EX.PC,LW_ID_EX.type,LW_EX_MEM.PC,LW_EX_MEM.type,LW_MEM_WB.PC,LW_MEM_WB.type);

      //printf("==============================================================================\n");
    }


    if (trace_view_on && cycle_number>=5) {/* print the instruction exiting the pipeline if trace_view_on=1 */
      switch(LW_MEM_WB.type) {
        case ti_NOP:
          printf("[cycle %d] NOP:\n",cycle_number) ;
          break;
        case ti_LOAD:
          printf("[cycle %d] LOAD:",cycle_number) ;
		  printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", LW_MEM_WB.PC, LW_MEM_WB.sReg_a, LW_MEM_WB.dReg, LW_MEM_WB.Addr);
          break;
        case ti_STORE:
          printf("[cycle %d] STORE:",cycle_number) ;
		  printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", LW_MEM_WB.PC, LW_MEM_WB.sReg_a, LW_MEM_WB.sReg_b, LW_MEM_WB.Addr);
          break;
        case ti_SPECIAL:
          printf("[cycle %d] SPECIAL:\n",cycle_number) ;
          break;
      }
    }
    if (trace_view_on && cycle_number>=5) {/* print the instruction exiting the pipeline if trace_view_on=1 */
      switch(ALU_EMPTY_WB.type) {
        case ti_NOP:
          printf("[cycle %d] NOP:\n",cycle_number) ;
          break;
        case ti_RTYPE: /* registers are translated for printing by subtracting offset  */
          printf("[cycle %d] RTYPE:",cycle_number) ;
      printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", ALU_EMPTY_WB.PC, ALU_EMPTY_WB.sReg_a, ALU_EMPTY_WB.sReg_b, ALU_EMPTY_WB.dReg);
          break;
        case ti_ITYPE:
          printf("[cycle %d] ITYPE:",cycle_number) ;
      printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", ALU_EMPTY_WB.PC, ALU_EMPTY_WB.sReg_a, ALU_EMPTY_WB.dReg, ALU_EMPTY_WB.Addr);
          break;
        case ti_BRANCH:
          printf("[cycle %d] BRANCH:",cycle_number) ;
      printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", ALU_EMPTY_WB.PC, ALU_EMPTY_WB.sReg_a, ALU_EMPTY_WB.sReg_b, ALU_EMPTY_WB.Addr);
          break;
        case ti_JTYPE:
          printf("[cycle %d] JTYPE:",cycle_number) ;
      printf(" (PC: %d)(addr: %d)\n", ALU_EMPTY_WB.PC, ALU_EMPTY_WB.Addr);
          break;
        case ti_SPECIAL:
          printf("[cycle %d] SPECIAL:\n",cycle_number) ;
          break;
        case ti_JRTYPE:
          printf("[cycle %d] JRTYPE:",cycle_number) ;
      printf(" (PC: %d) (sReg_a: %d)(addr: %d)\n", ALU_EMPTY_WB.PC, ALU_EMPTY_WB.dReg, ALU_EMPTY_WB.Addr);
          break;
      }
    }
  }

  trace_uninit();

  exit(0);
}
