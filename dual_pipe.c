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
#include "new.h"

int main(int argc, char **argv)
{
  struct instruction *tr_entry[2];
  struct instruction PCregister[2], IF_ID, ALU_ID_EX, LW_ID_EX,ALU_EX_EMPTY, LW_EX_MEM,ALU_EMPTY_WB, LW_MEM_WB;
  size_t size;
  char *trace_file_name;
  int trace_view_on = 0;
  int flush_counter = 4; //5 stage pipeline, so we have to execute 4 instructions once trace is done
  int ALUfilled = 0;
  int LWfilled = 0;
  int firstInsSent = 0;


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

  while(1) {

    size = trace_get_item(&tr_entry); /* put the instruction into a buffer */

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
      if(PCregister[0].type==ti_RTYPE){
        ALU_ID_EX = PCRegister[0];
        PCregister.type=NULL;
        ALUfilled=1;
      }


      if(!size){    /* if no more instructions in trace, reduce flush_counter */
        flush_counter--;
      }
      else{   /* copy trace entry into IF stage */
        if(PCregister[0].type==NULL)
          memcpy(&PCregister[0], tr_entry[0] , sizeof(PCregister[0]));
        if(PCregister[1].type==NULL)
          memcpy(&PCregister[1], tr_entry[1] , sizeof(PCregister[0]));
      }

      //printf("==============================================================================\n");
    }


    if (trace_view_on && cycle_number>=5) {/* print the instruction exiting the pipeline if trace_view_on=1 */
      switch(MEM_WB.type) {
        case ti_NOP:
          printf("[cycle %d] NOP:\n",cycle_number) ;
          break;
        case ti_RTYPE: /* registers are translated for printing by subtracting offset  */
          printf("[cycle %d] RTYPE:",cycle_number) ;
		  printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", MEM_WB.PC, MEM_WB.sReg_a, MEM_WB.sReg_b, MEM_WB.dReg);
          break;
        case ti_ITYPE:
          printf("[cycle %d] ITYPE:",cycle_number) ;
		  printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", MEM_WB.PC, MEM_WB.sReg_a, MEM_WB.dReg, MEM_WB.Addr);
          break;
        case ti_LOAD:
          printf("[cycle %d] LOAD:",cycle_number) ;
		  printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", MEM_WB.PC, MEM_WB.sReg_a, MEM_WB.dReg, MEM_WB.Addr);
          break;
        case ti_STORE:
          printf("[cycle %d] STORE:",cycle_number) ;
		  printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", MEM_WB.PC, MEM_WB.sReg_a, MEM_WB.sReg_b, MEM_WB.Addr);
          break;
        case ti_BRANCH:
          printf("[cycle %d] BRANCH:",cycle_number) ;
		  printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", MEM_WB.PC, MEM_WB.sReg_a, MEM_WB.sReg_b, MEM_WB.Addr);
          break;
        case ti_JTYPE:
          printf("[cycle %d] JTYPE:",cycle_number) ;
		  printf(" (PC: %d)(addr: %d)\n", MEM_WB.PC, MEM_WB.Addr);
          break;
        case ti_SPECIAL:
          printf("[cycle %d] SPECIAL:\n",cycle_number) ;
          break;
        case ti_JRTYPE:
          printf("[cycle %d] JRTYPE:",cycle_number) ;
		  printf(" (PC: %d) (sReg_a: %d)(addr: %d)\n", MEM_WB.PC, MEM_WB.dReg, MEM_WB.Addr);
          break;
      }
    }
  }

  trace_uninit();

  exit(0);
}
