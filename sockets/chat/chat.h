struct Chat;

struct Chat *create_chat(int max_size, short port);
void run_chat(struct Chat *chat);
void free_chat(struct Chat *chat);
