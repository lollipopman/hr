#include <errno.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static const char ansi_color_magenta[] = "\x1b[35m";
static const char ansi_color_reset[] = "\x1b[0m";

struct program_options {
  char delimiter[4];
  bool color_output;
  bool user_specified_width;
  int  width;
};

void print_usage() {
  printf("Usage: hr\n");
  printf("  -c Force color output even when stdout is not a terminal\n");
  printf("  -d DELIMITER, specify the delimiter, default '━'\n");
  printf("  -n Disable color output\n");
}

static int parse_argv(int argc, char *argv[], struct program_options *options) {
  char *endptr;
  bool force_color_output = false;
  int option;
  int delimiter_width;
  extern char *optarg;

  while ((option = getopt(argc, argv, "cd:hnw:")) != -1) {
    switch (option) {
    case 'c':
      force_color_output = true;
      break;
    case 'd':
      delimiter_width = mbstowcs(NULL, optarg, 0);
      if (delimiter_width != 1) {
        fprintf(stderr, "Delimiter must be a single column in width your string is %d columns wide\n", delimiter_width);
        return -EINVAL;
      } else {
        strncpy(options->delimiter, optarg, 4);
      }
      break;
    case 'h':
      print_usage();
      return(0);
    case 'n':
      options->color_output = false;
      break;
    case 'w':
      options->width = strtol(optarg, &endptr, 10);
      if (*endptr != '\0') {
        fprintf(stderr, "Unable to convert width to an integer: %s\n", strerror(errno));
        return -1;
      }
      options->user_specified_width = true;
      break;
    default:
      print_usage();
      return -EINVAL;
    }
  }

  if ((!isatty(STDOUT_FILENO)) && !(force_color_output))
      options->color_output = false;

  return 1;
}

static int terminal_width(struct program_options *options) {
  struct winsize ws;
  if (isatty(STDIN_FILENO)) {
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1) {
      fprintf(stderr, "Unable to retrieve terminal width: %s\n", strerror(errno));
      return -1;
    } else {
       options->width = ws.ws_col;
    }
  } else {
    fprintf(stderr, "Not connected to a tty!\n");
    return -1;
  }
  return 0;
}

static void print_hr(struct program_options *options) {
  int column;
  if (options->color_output)
    printf("%s", ansi_color_magenta);
  printf("\n");
  for (column = 0; column < options->width; column++) {
    if (column == 0 || column == (options->width - 1)) {
      printf(" ");
    } else {
      printf("%s", options->delimiter);
    }
  }
  printf("\n\n");
  if (options->color_output)
    printf("%s", ansi_color_reset);
}

int main(int argc, char *argv[]) {
  int r = 0;
  struct program_options options = {
    .delimiter = "━",
    .color_output = true,
    .user_specified_width = false,
    .width = 0
  };

  if (!setlocale(LC_CTYPE, "")) {
    fprintf(stderr, "Unable to set locale, check LC_CTYPE\n");
    r = -1;
    goto finish;
  }

  r = parse_argv(argc, argv, &options);
  if (r <= 0)
    goto finish;

  if (!options.user_specified_width) {
    r = terminal_width(&options);
    if (r == -1)
      goto finish;
  }

  print_hr(&options);

finish:
  return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
