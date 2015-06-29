#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static const char ansi_color_magenta[] = "\x1b[35m";
static const char ansi_color_reset[] = "\x1b[0m";

void print_usage() {
  printf("Usage: hr\n");
  printf("  -c Force color output even when stdout is a pipe\n");
  printf("  -d DELIMITER, default '━'\n");
  printf("  -n Disable color output\n");
}

main(int argc, char *argv[]) {
  extern char *optarg;
  char delimiter[] = "━";
  bool color_output = true;
  bool force_color_output = false;
  int column;
  int option;
  struct winsize ws;

  while ((option = getopt(argc, argv, "cd:hn")) != -1) {
    switch (option) {
    case 'c':
      force_color_output = true;
      break;
    case 'd':
      strncpy(delimiter, optarg, 4);
      break;
    case 'h':
      print_usage();
      exit(EXIT_SUCCESS);
    case 'n':
      color_output = false;
      break;
    default:
      print_usage();
      exit(EXIT_FAILURE);
    }
  }

  if ((!isatty(STDOUT_FILENO)) && !(force_color_output)) {
      color_output = false;
  }

  if (isatty(STDIN_FILENO)) {
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1) {
      fprintf(stderr, "Unable to retrieve terminal width: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  } else {
    fprintf(stderr, "Not connected to a tty!\n");
    exit(EXIT_FAILURE);
  }
  if (color_output) {
    printf("%s", ansi_color_magenta);
  }
  printf("\n");
  for (column = 0; column < ws.ws_col; column++) {
    if (column == 0 || column == (ws.ws_col - 1)) {
      printf(" ");
    } else {
      printf("%s", delimiter);
    }
  }
  printf("\n\n");
  if (color_output) {
    printf("%s", ansi_color_reset);
  }
}
