#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "structs.h"


void create_virus(struct virus_t *virus, float *lethality, float *infectivity, float *duration)
{
    virus->lethality = *lethality;
    virus->infectivity = *infectivity;
    virus->duration = *duration;
}

bool add_point(point_array_t *point_array, point_t *point)
{
    point_t **temp_point = NULL;
    if (point_array->size == 0)
    {
        temp_point = malloc(sizeof(point_t*));
        point_array->size = 1;
    } else {
        point_array->size++;
        temp_point = realloc(point_array->points, sizeof(point_t *) * point_array->size);
    }

    if (temp_point == NULL) {
        return false;
    }
    point_array->points = temp_point;

    point_t *new_point = malloc(sizeof(point_t));
    if (new_point == NULL) {
        point_array->size--;
        return false;
    }
    new_point->id = point->id;
    new_point->exposure = point->exposure;
    new_point->number_of_infected = point->number_of_infected;
    new_point->name = malloc(sizeof(char) * (strlen(point->name) + 1));
    strcpy(new_point->name, point->name);

    point_array->points[point_array->size - 1] = new_point;
    free(point->name);
    initialize_point(point);
    return true;
}

void initialize_point(point_t *point)
{
    memset(point, 0, sizeof(point_t));
}

void initialize_agent(agent_t *agent)
{
    agent->route = NULL;
    agent->immunity = 0;
    agent->is_infected = 0;
    agent->is_vaccinated = 0;
    agent->id = 0;
    agent->number_of_turns = 0;
    agent->current_point = -1;
    agent->is_dead = false;
    agent->is_currently_infected = false;
    agent->was_currently_cured = false;
}

void initialize_point_array(point_array_t *point_array)
{
    point_array->points = NULL;
    point_array->size = 0;
}

void initialize_agents_array(agents_list *agents)
{
    agents->a_size = 0;
    agents->agents = NULL;
}

void destroy_points(point_array_t *point_array)
{
    for (size_t index = 0; index < point_array->size; index++)
    {
        free(point_array->points[index]->name);
        point_array->points[index]->name = NULL;
        free(point_array->points[index]);
        point_array->points[index] = NULL;
    }
    free(point_array->points);
    point_array->size = 0;
    point_array->points = NULL;
}


void destroy_agents(struct agents_list_t *agent_list)
{
    for (size_t index = 0; index < agent_list->a_size; index++)
    {
        free(agent_list->agents[index]->route);
        free(agent_list->agents[index]);
    }
    free(agent_list->agents);
    agent_list->agents = NULL;
}


bool add_agent(agents_list *agents, agent_t *agent)
{
    agent_t **temp_agent = NULL;
    if (agents->a_size == 0)
    {
        temp_agent = malloc(sizeof(agent_t));
        agents->a_size = 1;
    } else {
        agents->a_size++;
        temp_agent = realloc(agents->agents, sizeof(agent_t *) * agents->a_size);
    }

    if (temp_agent == NULL) {
        return false;
    }
    agents->agents = temp_agent;

    agent_t *new_agent = malloc(sizeof(agent_t));
    if (new_agent == NULL) {
        agents->a_size--;
        return false;
    }
    new_agent->id = agent->id;
    new_agent->immunity = agent->immunity;
    new_agent->is_infected = agent->is_infected;
    new_agent->is_vaccinated = agent->is_vaccinated;
    new_agent->is_dead = agent->is_dead;
    new_agent->number_of_turns = agent->number_of_turns;
    new_agent->route = malloc(sizeof(size_t) * agent->number_of_turns);
    memcpy(new_agent->route, agent->route, sizeof(size_t) * agent->number_of_turns);

    agents->agents[agents->a_size - 1] = new_agent;
    /*for (int index = 0; index < agent->number_of_turns; index++){
        free(*agent->route[index]);
    }*/
    free(agent->route);
    initialize_agent(agent);
    return true;
}


size_t* parse_route(char *path, size_t *number_of_turns) {
    char *token;
    token = strtok(path, "-"); 
    size_t *route = malloc(sizeof(size_t));
    if (route == NULL) {
        return false;
    }

    while (token != NULL) {
        size_t current_id = strtoul(token, NULL, 10); // check str instead of NULL for syntax errors
        *number_of_turns = *number_of_turns + 1;
        size_t *temp = realloc(route, sizeof(size_t) * (*number_of_turns));     
        if (temp == NULL) {
            free(route);
            return NULL;
        }
        route = temp;
        route[*number_of_turns - 1] = current_id;
        path = path + strlen(token) + 1;
        token = strtok(path, "-");    
    }
    return route;
}

bool make_agent(struct agent *agent, char *his_path, int id, int vac, float imm, int inf)
{
    if (agent == NULL)
    {
        agent = malloc(sizeof(agent_t));
    }
    
    agent->id = id;
    agent->immunity = imm;
    agent->is_infected = inf;
    agent->is_vaccinated = vac;
    agent->number_of_turns = 0;
    agent->route = parse_route(his_path, &agent->number_of_turns);
    if (agent->route == NULL) {
        free(agent);
        return false;
    }

    return true;
}

bool make_point(struct point *point, int *id, char *name, float *exposure)
{
    if (point == NULL)
    {
        point = malloc(sizeof(struct point));
    }
    if (point != NULL)
    {
        point->name = malloc(sizeof(char) * (strlen(name) + 1));
    } else {
        return false;
    }
    if (point->name == NULL){
        free(point);
        point = NULL;
        return false;
    }
    strcpy(point->name, name);
    point->id = *id;
    point->exposure = *exposure;
    return true;
}
