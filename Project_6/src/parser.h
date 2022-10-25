int read_line_csv_world(FILE *file, struct point *point);
int read_line_csv_agent(FILE *file, struct agent *agent);
bool parse_csv_world(FILE *file,
                     struct point_array *points);
bool parse_csv_agents(FILE *file,
                      struct agents_list_t *agents);
