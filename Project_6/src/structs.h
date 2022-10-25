#include <getopt.h>


typedef struct point{
    int id;
    char *name;
    float exposure;
    int number_of_infected;
}point_t;

typedef struct point_array{
    struct point **points;
    size_t size;
}point_array_t;

typedef struct agent{
    int id;
    size_t *route;
    size_t number_of_turns; // agent ide po routes stále dookola!!!
    bool is_vaccinated;
    float immunity;
    bool is_infected;
    int current_point;
    bool is_dead;
    bool was_currently_cured;
    bool is_currently_infected;
}agent_t;

typedef struct agents_list_t{
    agent_t **agents;
    size_t a_size;
}agents_list;

typedef struct virus_t{
    float lethality;
    float infectivity;
    float duration;
}virus;

size_t* parse_route(char *path, size_t *number_of_turns);
void create_virus(struct virus_t *virus, float *lethality, float *infectivity, float *duration);
bool add_point(point_array_t *point_array, point_t *point);
void destroy_points(point_array_t *point_array);
void destroy_agents(struct agents_list_t *agent_list);
void initialize_point(point_t *point);
void initialize_agent(agent_t *agent);
void initialize_point_array(point_array_t *point_array);
void initialize_agents_array(agents_list *agents);
bool add_agent(agents_list *agents, agent_t *agent);
bool make_agent(struct agent *agent, char *his_path, int id, int vac, float imm, int inf);
bool make_point(struct point *point, int *id, char *name, float *exposure);
