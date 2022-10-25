#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <linux/limits.h>
#include "structs.h"
#include "parser.h"


int read_line_csv_world(FILE *file, struct point *point)
{
    int id = 0;
    char name[17] = {0};
    float exposure = 0;
    int status = 0;
    while ((status = fscanf(file, "%d;%[^;];%f\n", &id, name, &exposure)) != EOF){
        if (exposure < 0 || id < 0 || status != 3)
        {
            return EXIT_FAILURE;
        }
        if (!make_point(point, &id, name, &exposure)){
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    return EXIT_SUCCESS;
}

int read_line_csv_agent(FILE *file, struct agent *agent)
{
    int id = 0;
    char route[PATH_MAX]= {0};
    memset(route, 0, sizeof(char) * PATH_MAX);
    int is_vaccinated = 0;
    float immunity = 0;
    int is_infected = 0; // 0 nezaočkovaný
    int status = 0;
    int flag = EXIT_SUCCESS;
    while ((status = fscanf(file, "%d;%[^;];%d;%f;%d", &id, route, &is_vaccinated, &immunity, &is_infected)) != EOF){
        if (status != 5 || id < 0 
        || is_vaccinated < 0 || is_vaccinated > 1 
        || immunity < 0 || immunity > 1 
        || is_infected < 0 || is_infected > 1)
        {
            flag = EXIT_FAILURE;
        }
        if (!make_agent(agent, route, id, is_vaccinated, immunity, is_infected)){
            flag = EXIT_FAILURE;
        }
        if (flag == EXIT_FAILURE){
            return EXIT_FAILURE;
        } else {
            return EXIT_SUCCESS;
        }
    }
    return EXIT_SUCCESS;
}

bool parse_csv_world(FILE *file,
                     struct point_array *points)
{
    int read_status = 0;
    initialize_point_array(points);
    // we are going to load a point array
    while (read_status == 0){
        point_t point;
        initialize_point(&point);
        int status = read_line_csv_world(file, &point);
        if (status == EXIT_SUCCESS && point.name == NULL){ // end of file, no node, success
            read_status = 1;
        } else if (status == EXIT_FAILURE){ // sth was loaded but failure happened
            free(point.name);
            point.name = NULL;
            destroy_points(points);
            return false;
        } else { // everything went good, add point to the array
            if (!add_point(points, &point))
            {
                destroy_points(points);
                return false;
            }
        }
    }
    return true; 
}

bool parse_csv_agents(FILE *file,
                      struct agents_list_t *agents)
{
    int read_status = 0;
    initialize_agents_array(agents);
    // we are going to load a point array
    while (read_status == 0){
        agent_t agent;
        initialize_agent(&agent);
        int status = read_line_csv_agent(file, &agent);
        if (status == EXIT_SUCCESS && agent.route == NULL){ // end of file, no node, success
            read_status = 1;
        } else if (status == EXIT_FAILURE){ // sth was loaded but failure happened
            free(agent.route);
            initialize_agent(&agent);
            return false;
        } else { // everything went good, add point to the array
            if (!add_agent(agents, &agent))
            {
                destroy_agents(agents);
                return false;
            }
        }
    }
    return true;
}

