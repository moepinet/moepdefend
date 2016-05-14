#ifndef _ARGS_H_
#define _ARGS_H_

struct argp_state;

error_t parse_opt(int key, char *arg, struct argp_state *state);

#endif//_ARGS_H_
