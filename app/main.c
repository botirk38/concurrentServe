#include "server.h"

int main(int argc, char **argv) {
  parse_cli_args(argc, argv);
  start_server();
  return 0;
}
