#include "structs.h"

#ifndef UTILS
#define UTILS

/**
 * Send message to client
 *
 * @param p player to send msg to
 * @param type type of message
 * @param msg additional message which is added after type
 */
int send_msg(Player *p, MsgType type, char *msg);

/**
 * Invalid message handler -- increases invalid msg count,
 * disconnects player if needed
 *
 * @param p player who sent invalid msg
 */
int invalid_msg(Server *s, Player *p);

/**
 * Find connected player by its file descriptor 
 */
Player *find_connected_player(Server *s, int fd);

/**
 * Check if name is valid game name / nickname
 * @return 1 - valid, 0 - invalid
 */
int is_name_valid(char* name);

/**
 * Transition in state automata
 *
 * @param state current state
 * @param en player event
 */
PState transition(PState state, PEvent en);

#endif
