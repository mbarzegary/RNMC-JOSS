#include <getopt.h>
#include "../core/dispatcher.h"
#include "../core/lattice_simulation.h"
#include "lattice_reaction_network.h"

void print_usage() {
    
    std::cout << "Usage: specify the following in the input file\n"
              << "reaction_database\n"
              << "initial_state_database\n"
              << "number_of_simulations\n"
              << "thread_count\n"
              << "base_seed\n"
              << "step_cutoff\n"
              << "parameters\n";
    
} // print_usage()

/* ---------------------------------------------------------------------- */

void print_usage_LGMC_parameters() {
 
     std::cout << "Usage: specify the following in the input file\n"
              << "lattice constant\n"
              << "box x upper boundary\n"
              << "box y upper boundary\n"
              << "box z upper boundary\n"
              << "Temperature\n"
              << "Electron free energy\n"
              << "Is Add Site (T/F)\n"
              << "Charge transfer style (M/B)\n"
              << "Filename for lattice fill\n";
} // print_usage_LGMC_parameters()

/* ---------------------------------------------------------------------- */

int main(int argc, char **argv) {

    struct option long_options[] = {
        {"reaction_database", required_argument, NULL, 'r'},
        {"initial_state_database", required_argument, NULL, 'i'},
        {"number_of_simulations", required_argument, NULL, 'n'},
        {"base_seed", required_argument, NULL, 'b'},
        {"thread_count", required_argument, NULL, 'c'},
        {"step_cutoff", required_argument, NULL, 's'},
        {"time_cutoff", required_argument, NULL, 't'},
        {"parameters", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, '\0'}
        // last element of options array needs to be filled with zeros
    };

    int c;
    int option_index = 0;

    char *reaction_database = nullptr;
    char *initial_state_database = nullptr;
    int number_of_simulations = 0;
    int base_seed = 0;
    int thread_count = 0;
    char *LGMC_params_file = nullptr;

    Cutoff cutoff = {
        .bound =  { .step =  0 },
        .type_of_cutoff = step_termination
    };

    while ((c = getopt_long_only(
                argc, argv, "hr:i:n:b:c:s:t:p:",
                long_options,
                &option_index)) != -1) {

        switch (c) {

        case 'h': 
            print_usage();
            exit(0);
            break; 

        case 'r':
            reaction_database = optarg;
            break;

        case 'i':
            initial_state_database = optarg;
            break;

        case 'n':
            number_of_simulations = atoi(optarg);
            break;

        case 'b':
            base_seed = atoi(optarg);
            break;

        case 'c':
            thread_count = atoi(optarg);
            break;

        case 's':
            cutoff.bound.step = atoi(optarg);
            cutoff.type_of_cutoff = step_termination;
            break;

        case 't':
            cutoff.bound.time = atof(optarg);
            cutoff.type_of_cutoff = time_termination;
            break;

        case 'p':
            LGMC_params_file = optarg;
            break;

        default:
            // if an unexpected argument is passed, exit
            print_usage();
            exit(EXIT_FAILURE);
            break;

        }

    }                    

    // read in LGMC parameters from file
    std::string LGMC_params_str(LGMC_params_file);
    std::ifstream fin;
    fin.open(LGMC_params_str);

    if(!fin.is_open()) {
        std::cout << "Failed to open file: " << LGMC_params_file << "\n";
        exit(EXIT_FAILURE);
    }

    float latconst;                              
    float boxxhi, boxyhi, boxzhi;                       
    float temperature;
    float g_e;
    bool is_add_site;
    char add_site;
    ChargeTransferStyle charge_transfer_style;
    char ct_style;
    std::string fill_lattice;

    fin >> latconst >> boxxhi >> boxyhi >> boxzhi
    >> temperature >> g_e >> add_site >> ct_style
    >> fill_lattice;

    if(add_site == 'T') {
        is_add_site = true;
    }
    else if (add_site == 'F') {
        is_add_site = false;
    }else {
        std::cout << "Incorrect parameter file argument for add site.\n";
        exit(EXIT_FAILURE);
    }

    if(ct_style == 'M') {
        charge_transfer_style = ChargeTransferStyle::MARCUS;
    }
    else if (ct_style == 'B') {
        charge_transfer_style = ChargeTransferStyle::BUTLER_VOLMER;
    }else {
        std::cout << "Incorrect parameter file argument for charge transfer style.\n";
        exit(EXIT_FAILURE);
    }


    LatticeParameters parameters{.latconst = latconst, 
                            .boxxhi = boxxhi, 
                            .boxyhi = boxyhi,
                            .boxzhi = boxzhi, 
                            .temperature = temperature, 
                            .g_e = g_e, .is_add_sites = is_add_site,
                            .charge_transfer_style = charge_transfer_style,
                            .lattice_fill = fill_lattice};                               


    Dispatcher<LatticeSolver,
    LatticeReactionNetwork,
    LatticeParameters,
    LatticeWriteTrajectoriesSql,
    LatticeReadTrajectoriesSql,
    LatticeWriteStateSql,
    LatticeReadStateSql,
    LatticeWriteCutoffSql,
    LatticeReadCutoffSql, 
    LatticeStateHistoryElement, 
    LatticeTrajectoryHistoryElement, 
    LatticeCutoffHistoryElement, 
    LatticeSimulation, 
    LatticeState>

        dispatcher (
        reaction_database,
        initial_state_database,
        number_of_simulations,
        base_seed,
        thread_count,
        cutoff,
        parameters
        );

    dispatcher.run_dispatcher();
    exit(EXIT_SUCCESS);

}