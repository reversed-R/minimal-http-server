#ifndef HEADER_PARSER_H
#define HEADER_PARSER_H

int parse_header_line(const char *header_line, char *header_type,
                      char *params_str);

#endif
