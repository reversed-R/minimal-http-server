#include <netdb.h>      /* getnameinfo() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h> /* wait4() */
#include <sys/socket.h>   /* socket() */
#include <sys/types.h>    /* socket(), wait4() */
#include <sys/wait.h>     /* wait4() */
#include <unistd.h>

#include "request/request_parser.h"
#include "response/response.h"

#define BUFFERSIZE 1024

void tcp_peeraddr_print(int com);
void sockaddr_print(struct sockaddr *addrp, int addr_len);
int open_accepting_socket(int port);
void main_loop(int accepting_socket);
/* void http_proceed(int sock); */
void proceed_http(int sock);
void delete_zombie(void);

int main(int argc, char *argv[]) {
  printf("[pid:%d] minimal http server started\n", getpid());

  int sock = open_accepting_socket(10000);

  int on = 1;
  if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) != 0) {
    printf("could NOT setsockopt\n");
  }

  main_loop(sock);
  close(sock);

  return (0);
}

int open_accepting_socket(int port) {
  struct sockaddr_in addr; // インターネット接続用アドレス
  socklen_t addr_size;
  int sock;
  /* sock = socket(PF_INET, SOCK_STREAM, 0); // TCPソケット */
  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("accepting socket"), exit(1);
  }

  memset(&addr, 0, sizeof(addr));    // ゼロクリア
  addr.sin_family = AF_INET;         // インターネット
  addr.sin_addr.s_addr = INADDR_ANY; // ワイルドカード
  addr.sin_port = htons(port);       // ネットワーク順に変換
  addr_size = sizeof(addr);

  if (bind(sock, (struct sockaddr *)&addr, addr_size) < 0) {
    perror("bind accepting socket"), exit(1);
  }
  if (listen(sock, 5) < 0) {
    perror("listen"), exit(1);
  }

  return (sock);
}

void main_loop(int accepting_socket) {
  while (1) {
    int sock;
    pid_t child_pid;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size;
    client_addr_size = sizeof(client_addr);

    delete_zombie();

    sock = accept(accepting_socket, (struct sockaddr *)&client_addr,
                  &client_addr_size);
    if (sock < 0) {
      perror("accept");
      exit(1);
    }

    tcp_peeraddr_print(sock);
    if ((child_pid = fork()) > 0) /* parent */
    {
      close(sock);
    } else if (child_pid == 0) /* child */
    {
      close(accepting_socket);
      /* http_proceed(sock); */
      proceed_http(sock);
      exit(0);
    } else {
      perror("fork");
      exit(-1);
    }
  }
}

/* void http_proceed(int sock) { */
/*   printf("[pid:%d, fd:%d] ----> \n", getpid(), sock); */
/**/
/*   request rq = parse_request(sock); */
/*   // print_request(rq); */
/**/
/*   switch (rq.method) { */
/*   case HTTP_METHOD_GET: */
/*     respond_for_GET(sock, rq.uri); */
/*     break; */
/*   } */
/**/
/*   close(sock); */
/**/
/*   printf("<----[pid:%d, fd:%d]\n", getpid(), sock); */
/*   fflush(stdout); */
/* } */

void proceed_http(int sock) {
  printf("[pid:%d, fd:%d] ----> \n", getpid(), sock);

  request rq;
  int bufsize = 1024;
  char buf[bufsize];
  int head_index = 0;

  /* initialization of request rq */
  Headers headers;
  connection_h connection;
  content_length_h content_length;
  headers.connection = &connection;
  headers.connection->value = HTTP_CONNECTION_CLOSE;
  headers.content_length = &content_length;
  headers.content_length->length = 0;
  rq.known_headers = &headers;

  rq.headers = NULL;
  rq.entity_body = NULL;
  rq.uri = NULL;

  int r;
  while ((r = read_request(sock, buf, bufsize, &head_index, &rq)) > 0) {
    /* re initialzation  */
    headers.content_length->length = 0;

    printf("read_request returns %d\n", r);
    switch (rq.method) {
    case HTTP_METHOD_GET:
      respond_for_GET(sock, rq.uri);
      break;
    }

    if (rq.headers != NULL) {
      list_clear(rq.headers, 1);
    }
    free(rq.entity_body);
    free(rq.uri);
  }

  if (r < 0) {
    /* parse error, 400 bad request */
  }

  shutdown(sock, SHUT_RDWR);
  close(sock);

  printf("<----[pid:%d, fd:%d]\n", getpid(), sock);
  fflush(stdout);
}

void tcp_peeraddr_print(int com) {
  struct sockaddr_storage addr;
  int addr_len;
  addr_len = sizeof(addr);
  if (getpeername(com, (struct sockaddr *)&addr, (socklen_t *)&addr_len) < 0) {
    perror("tcp_peeraddr_print");
    return;
  }
  printf("[pid:%d] connection (fd:%d) from ", getpid(), com);
  sockaddr_print((struct sockaddr *)&addr, addr_len);
  printf("\n");
}

void sockaddr_print(struct sockaddr *addrp, int addr_len) {
  char host[BUFFERSIZE];
  char port[BUFFERSIZE];
  if (getnameinfo(addrp, addr_len, host, sizeof(host), port, sizeof(port),
                  NI_NUMERICHOST | NI_NUMERICSERV) < 0)
    return;
  printf("%s:%s", host, port);
}

void delete_zombie() {
  pid_t pid;
  while ((pid = wait4(-1, 0, WNOHANG, 0)) > 0) {
    printf("[pid:%d] process (pid:%d) deleted.\n", getpid(), pid);
    continue;
  }
}
