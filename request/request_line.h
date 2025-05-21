#define HTTP_METHOD_GET 1
#define HTTP_METHOD_HEAD 2
#define HTTP_METHOD_POST 3

int parse_request_line(char *line, int *method, char *uri, int *version_upper,
                       int *version_lower);

char *get_method_str_by_id(int method);
