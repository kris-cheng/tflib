#ifndef _FUNCS_H_
#define _FUNCS_H_

/* config.c */
bool initialize(const char *filename, bool console);
void cleanup();
void adjust_peers_limit(void);

/* main.c */

/* console.c */
void run_console();

#endif
