#ifndef TARG_PARSER_H
#define TARG_PARSER_H

//TODO : Create Queue Struct that contains elements representing a target.

// check token and extract target
// if any error free all struct and throw target invalid err
// then try to create a queue elm and push it into the queue, 
// if encontering an error during these phase free all struct and throw mem error
int TargPush(char * token);

#endif /* !_TARG_PARSER_H */