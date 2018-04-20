#ifndef _G_OPT_H
#define _G_OPT_H
// Probability that a particular byte will be dropped from ic7300jig.cxx.
// This will be set to a number between -1 and RAND_MAX inclusive.
extern long int g_opt_p_drop_byte;

// Probability that message dropping will be triggered.
// This will be set to a number between -1 and RAND_MAX inclusive.
extern long int g_opt_p_drop_message;

// Number of messages that will be dropped once triggered.
extern int g_opt_n_drop_message;

// Probability that message hesitation will be triggered.
// This will be set to a number between -1 and RAND_MAX inclusive.
extern long int g_opt_p_hesitate;

// Number of milleseconds to hesitate when triggered.
extern int g_opt_t_hesitate;

// Number of messages that will be hesitated once triggered.
extern int g_opt_n_hesitate;
#endif
