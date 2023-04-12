#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "Instruction.h"
#include "Pipeline.h"
#include "DepMap.h"

using std::string;
using std::cout;
using std::endl;

// goes to specific line in file
void gotoLine(std::ifstream& file, int num){
    file.seekg(std::ios::beg);
    for(int i = 0; i < num - 1; ++i){
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
}

// Main simulator function
void Simulation(std::ifstream& ifile, long start_inst, long inst_count, int W) {
    Pipeline P = Pipeline(W, inst_count);
    // instructions in WB retire and leave the pipeline (and the instruction window)
    while (P.inst_done < (unsigned long)inst_count) { 
        P.tick();
    }
    std::cout << "clock cycles: " << P.clock_cycle << std::endl;
    printf("Instruction Types ran : int = %.4f%%, float = %.4f%%, branch = %.4f%%, load = %.4f%%, store = %.4f%%\n\n", 
    ((double)100*P.ITypeCount[0])/inst_count, 
    ((double)100*P.ITypeCount[1])/inst_count, 
    ((double)100*P.ITypeCount[2])/inst_count, 
    ((double)100*P.ITypeCount[3])/inst_count, 
    ((double)100*P.ITypeCount[4])/inst_count);

}

// example: ./proj sample_trace/compute_fp_1 10000000 1000000 2
// example: ./proj compute_fp_1 10000000 1000000 2
int main(int argc, char* argv[]) {
    // input arguments trace file name, starting instruction, instruction count, W
    if(argc == 5){
		char* trace_file_name = argv[1];
        long start_inst = std::stol(argv[2]);
        long inst_count = std::stol(argv[3]);
        int W = atoi(argv[4]);

        DepMap::ifile.open(trace_file_name);
		// Error checks for input variables here, exit if incorrect input
        if (!DepMap::ifile.is_open() || start_inst <= 0 || inst_count < 0 || W < 1 || W > 4) {
            printf("Input Error. Terminating Simulation...\n");
            return 0;
        }

		// Start Simulation
		printf("Simulating with trace_file_name = '%s' , start_inst = %ld, inst_count = %ld, W = %d\n", trace_file_name, start_inst, inst_count, W);
        gotoLine(DepMap::ifile, start_inst);
		Simulation(DepMap::ifile, start_inst, inst_count, W);
        DepMap::ifile.close();
	}
	else printf("Insufficient number of arguments provided!\n");
	return 0;
}