#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "structs.h"
#include "parser.h"
#include <getopt.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define END_AGENTS_DEAD 1
#define END_AGENTS_CURED 2
#define END_MAX_STEPS 3

static struct option long_options[] = {
        {"lethality", required_argument, NULL, 'l'},
        {"infectivity", required_argument, NULL, 'i'},
        {"duration", required_argument, NULL, 'd'},
        {"vaccine-modifier", required_argument, NULL, 'v'},
        {"max-steps", required_argument, NULL, 'm'},
        {"random-seed", required_argument, NULL, 'r'},
        {"verbose", no_argument, NULL, 'w'},
        {0, 0, 0, 0}
};

void error_message(const char *message)
{
    fprintf(stderr, "%s", message);
}

int parse_commands(int argc, char *argv[],
                   float *lethality, float *infectivity,
                   float *duration, float *modifier,
                   int *max_steps, int *seed, bool *verbose)
{
    int opt = 0;
    char *temp;

    while ((opt = getopt_long_only(argc, argv, "", long_options, NULL)) != -1){
        switch (opt) {
            case 'l': *lethality = strtof(optarg, &temp);
                if (optarg == temp || *temp != '\0') {
                    return EXIT_FAILURE;
                }
                break;
            case 'i': *infectivity = strtof(optarg, &temp);
                if (optarg == temp || *temp != '\0') {
                    return EXIT_FAILURE;
                }
                break;
            case 'd': *duration = strtof(optarg, &temp);
                if (optarg == temp || *temp != '\0') {
                    return EXIT_FAILURE;
                }
                break;
            case 'v': *modifier = strtof(optarg, &temp);
                if (optarg == temp || *temp != '\0') {
                    return EXIT_FAILURE;
                }
                break;
            case 'm': *max_steps = strtol(optarg, &temp, 10);
                if (optarg == temp || *temp != '\0') {
                    return EXIT_FAILURE;
                }
                break;
            case 'r': *seed = strtol(optarg, &temp, 10);
                if (optarg == temp || *temp != '\0') {
                    return EXIT_FAILURE;
                }
                break;
            case 'w': *verbose = true;
                break;
            default:
                return EXIT_FAILURE;
        }
    }
    if (argv[optind + 2] != NULL) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


int check_simulation(agents_list agents, int step, int max_steps) {
    bool agents_dead = true;
    for (size_t i = 0; i < agents.a_size; i++) {
        if (!agents.agents[i]->is_dead) {
            agents_dead = false;
            break;
        }
    }   

    if (agents_dead) {
        return END_AGENTS_DEAD;
    }


    bool agents_cured = true;
    for (size_t i = 0; i < agents.a_size; i++) {
        if (agents.agents[i]->is_infected) {
            agents_cured = false;
            break;
        }
    }

    if (agents_cured) {
        return END_AGENTS_CURED;
    }
 

    if (max_steps < step && max_steps != -1) {
        return END_MAX_STEPS;
    }

    return 0;
}

void print_header(const int result)
{
    
    switch (result)
    {
    case END_AGENTS_CURED:
        printf("Virus is extinct.\n");
        break;
    case END_AGENTS_DEAD:
        printf("Population is extinct.\n");
        break;
    case END_MAX_STEPS:
        printf("Step limit expired.\n");
        break;
    default:
        break;
    }
}

void print_statistics(const int total_infections, const int total_deaths, const int survivors)
{
    printf("Statistics:\n");
    printf("\tTotal infections: %d\n", total_infections);
    printf("\tTotal deaths: %d\n", total_deaths);
    printf("\tNumber of survivors: %d\n", survivors);
}

void print_location_steps(const point_t *place, const bool multiple_infected, const int *max_steps)
{
    printf("Most infectious location:\n");
    if (multiple_infected) {
        printf("\tMultiple\n");
    } else {
        printf("\t- %s: %d infections\n", place->name, place->number_of_infected);
    }
    printf("Simulation terminated after %d steps.\n", *max_steps);
}

bool play_game(struct virus_t *virus, const float *vaccine_modifier,
        const int *max_steps, const int *seed, const bool *verbose,
        FILE *world, FILE *agents)
{
    if (virus == NULL && *vaccine_modifier == 1000 && *max_steps == 1000 && *seed == 1000 && !*verbose){
        return false;
    }

    struct point_array point_array;
    if (!parse_csv_world(world, &point_array))
    {
        error_message("not able to parse world.csv\n");
        return false;
    }
    struct agents_list_t agents_array;
    if (!parse_csv_agents(agents, &agents_array))
    {
        error_message("not able to parse agents.csv\n");
        return false;
    }
    size_t i, j;
    struct agent *temp;
    for(i = 0; i < agents_array.a_size - 1; i++){
        for(j = i+1; j < agents_array.a_size; j++){
            if (agents_array.agents[i]->id > agents_array.agents[j]->id){
                temp = agents_array.agents[i];
                agents_array.agents[i] = agents_array.agents[j];
                agents_array.agents[j] = temp;
            }
            else if (agents_array.agents[i]->id == agents_array.agents[j]->id){
                destroy_points(&point_array);
                destroy_agents(&agents_array);
                error_message("same id of agents\n");
                return EXIT_FAILURE;
            }
        }
    }
    struct point *temp_p;
    for(i = 0; i < point_array.size - 1; i++){
        for(j = i+1; j < point_array.size; j++){
            if (point_array.points[i]->id > point_array.points[j]->id){
                temp_p = point_array.points[j];
                point_array.points[i] = point_array.points[j];
                point_array.points[j] = temp_p;
            }
            else if (point_array.points[i]->id == point_array.points[j]->id){
                destroy_points(&point_array);
                destroy_agents(&agents_array);
                error_message("same id of points\n");
                return EXIT_FAILURE;
            }
        }
    }

    printf("Random seed: %d\n", *seed);
    size_t step = 1;
    int state = 0;
    int total_infections = 0;
    while (!(state = check_simulation(agents_array, step, *max_steps))) {
        for (size_t i = 0; i < agents_array.a_size; i++) {
            agent_t *agent = agents_array.agents[i]; 
            size_t agent_round_index = step % agent->number_of_turns;
            agents_array.agents[i]->current_point = agent->route[agent_round_index];
        }

        if (*verbose) {
            printf("\n*** STEP %ld ***\n", step);
        }
        
        for (size_t point_index = 0; point_index < point_array.size; point_index++) 
        {
            point_t *current_point = point_array.points[point_index];
            for (size_t agent_index = 0; agent_index < agents_array.a_size; agent_index++) 
            {
                agent_t *current_agent = agents_array.agents[agent_index];
                if (current_point->id == current_agent->current_point && current_agent->is_infected) 
                {
                    double roll = (double) rand() / RAND_MAX;

                    bool dead = roll * (current_agent->is_vaccinated ? *vaccine_modifier : 1.0) < virus->lethality;
                    
                    if (dead) {
                        current_agent->is_dead = true;
                        current_agent->is_infected = false;
                        if (*verbose){
                            printf("Agent %d has died at %s.\n", current_agent->id, current_point->name);
                        }
                        continue;
                    } 

                    roll = (double) rand() / RAND_MAX;
                    bool cured = roll > virus->duration;

                    if (cured) {
                        current_agent->is_infected = false;
                        current_agent->was_currently_cured = true;
                        if (*verbose){
                            printf("Agent %d has recovered at %s.\n", current_agent->id, current_point->name);
                        }
                    }

                }
            }
        }

        for (size_t point_index = 0; point_index < point_array.size; point_index++) 
        {
            point_t *current_point = point_array.points[point_index];
            size_t number_of_infected = 0;
            agent_t *infected_agents_array[agents_array.a_size];
            for (size_t agent_index = 0; agent_index < agents_array.a_size; agent_index++) 
            {
                agent_t *current_agent = agents_array.agents[agent_index];
                if (current_point->id == current_agent->current_point && current_agent->is_infected) {
                    infected_agents_array[number_of_infected] = current_agent;
                    number_of_infected++;

                }  
            }
            if (number_of_infected == 0) {
                continue;
            } 
            for (size_t infected_index = 0; infected_index < number_of_infected; infected_index++) {
                for (size_t agent_index = 0; agent_index < agents_array.a_size; agent_index++) 
                {
                    agent_t *current_agent = agents_array.agents[agent_index];


                    if (current_agent->current_point != current_point->id) {
                        continue;
                    }

                    if (current_agent->is_infected 
                    || current_agent->is_dead) {
                        continue;
                    }


     
                    double roll = (double) rand() / RAND_MAX;
                    bool infected = current_point->exposure * roll * virus->infectivity > current_agent->immunity * (current_agent->is_vaccinated ? *vaccine_modifier : 1.0);
                    current_agent->is_infected = infected;
                    if (infected) {
                        if (*verbose) {
                            printf("Agent %d has infected agent %d at %s.\n",
                                    infected_agents_array[infected_index]->id, 
                                    current_agent->id, current_point->name);
                        }
                        total_infections++;
                        current_point->number_of_infected++;
                    }
                }
            }
        }
    
        for (size_t agent_index = 0; agent_index < agents_array.a_size; agent_index++) 
        {
            agent_t *current_agent = agents_array.agents[agent_index];
            current_agent->was_currently_cured = false;
        }
        step++;
    }
    int total_deaths = 0;
    int survivors = agents_array.a_size;
    for (size_t index = 0; index < agents_array.a_size; index++)
    {
        if (agents_array.agents[index]->is_dead)
        {
            total_deaths++;
            survivors--;
        }
    }

    point_t *max_point = NULL;
    bool multiple_infected = false;
    for (size_t i = 0; i < point_array.size; i++)
    {
        point_t *current_point = point_array.points[i];
        if (max_point == NULL) {
            max_point = current_point;
            continue;
        }
        
        if (current_point->number_of_infected > max_point->number_of_infected)
        {
            max_point = current_point;
            multiple_infected = false;
        } 

        else if (current_point->number_of_infected == max_point->number_of_infected) 
        {
            multiple_infected = true;
        } 
    }

    
    int final_steps = 0;
    if (state == END_MAX_STEPS){
        final_steps = *max_steps;
    } else {
        final_steps = step - 1;
    }
    
    print_header(state);
    print_statistics(total_infections, total_deaths, survivors);
    print_location_steps(max_point, multiple_infected, &final_steps);

    destroy_points(&point_array);
    destroy_agents(&agents_array);

    return true;
}

int main(int argc, char *argv[])
{   
    if (argc < 3) 
    {
        error_message("wrong number of arguments\n");
        return EXIT_FAILURE;
    }
    FILE *world = fopen(argv[argc - 1], "r");
    if (world == NULL){
        perror(argv[argc - 1]);
        return EXIT_FAILURE;
    }
    FILE *agents = fopen(argv[argc - 2], "r");
    if (agents == NULL){
        perror(argv[argc - 2]);
        return EXIT_FAILURE;
    }

    float lethality = 0.5;
    float infectivity = 0.5;
    float duration = 0.5;
    float modifier = 1.2;
    int max_steps = -1;
    srand(time(NULL));
    int seed = rand();
    bool verbose = false;
    int status = parse_commands(argc, argv, &lethality, &infectivity, &duration,
                                &modifier, &max_steps, &seed, &verbose);
    if (status == EXIT_FAILURE || lethality < 0 || lethality > 1 
    || infectivity < 0 || infectivity > 1 
    || duration < 0 || duration > 1 
    || modifier < 0){
        error_message("wrong arguments\n");
        fclose(world);
        fclose(agents);
        return EXIT_FAILURE;
    }
    srand(seed);
    virus virus;
    create_virus(&virus, &lethality, &infectivity, &duration);

    if (!play_game(&virus, &modifier, &max_steps, &seed, &verbose, world, agents))
    {
        fclose(world);
        fclose(agents);
        return EXIT_FAILURE;
    }
    fclose(world);
    fclose(agents);

    return EXIT_SUCCESS;
}
