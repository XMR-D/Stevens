#pragma once

int is_delim(char c);
void free_redirect_globals(void);
int update_redir_globals(char * redirection_targ, char * type);
int is_invalid_redir_state(char c);

/* !HANDLING_REDIRECTIONS_H */
