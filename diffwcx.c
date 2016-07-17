#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

static void die(char const *msg, ...) {
   va_list args;
   va_start(args, msg);
   (void)vfprintf(stderr, msg, args);
   va_end(args);
   (void)fputc('\n', stderr);
   exit(EXIT_FAILURE);
}

static void chk_stdin(void) {
   if (ferror(stdin)) die("Error reading from standard input stream!");
}

static void stdout_error(void) {
   die("Error writing to standard output stream!");
}

static void chk_stdout(void) {
   if (ferror(stdout)) stdout_error();
}

static void ck_printf(char const *format, ...) {
   va_list args;
   va_start(args, format);
   if (vprintf(format, args) < 0) stdout_error();
   va_end(args);
}

static void ck_putc(int c) {
   if (putchar(c) != c) stdout_error();
}

static void perror_exit(int e, char const *msg) {
   errno= e;
   perror(msg);
   exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
   int mode= 'w';
   if (argc > 1) {
      int optind, argpos;
      char *arg= argv[optind= 1];
      for (argpos= 0;; ) {
         int c;
         switch (c= arg[argpos]) {
            case '-':
               if (argpos) goto bad_option;
               switch (arg[++argpos]) {
                  case '-':
                     if (c= arg[argpos + 1]) {
                        die("Unsupported long option %s!", arg);
                     }
                     ++optind; /* "--". */
                     goto end_of_options;
                  case '\0': goto end_of_options; /* "-". */
               }
               /* At first option switch character now. */
               continue;
            case '\0': {
               if (++optind == argc) goto end_of_options;
               arg= argv[optind];
               argpos= 0;
               continue;
            }
         }
         if (!argpos) goto end_of_options;
         switch (c) {
            case 'W': case 'C': case 'X': case 'B':
            case 'w': case 'c': case 'x': case 'b':
               mode= c;
               break;
            default: bad_option: die("Unsupported option -%c!", c);
         }
         ++argpos;
      }
      end_of_options:
      if (optind != argc) die("Too many arguments!");
   }
   switch (mode) {
      int c;
      case 'x': case 'b':
         {
            register int c;
            while ((c= getchar()) != EOF) {
               if (mode == 'b' || c == 0x20) ck_printf("%02X\n", c);
               else ck_printf("%02X %c\n", c, c > 0x20 && c < 0x7f ? c : '.');
            }
         }
         chk_stdin();
         break;
      case 'X': case 'B':
         {
            auto int c;
            int n;
            errno= 0;
            while ((n= scanf("%2x", &c)) == 1) {
               if (putchar(c) == EOF) stdout_error();
               while ((c= getchar()) != '\n') {
                  if (c == EOF) {
                     chk_stdin();
                     goto done;
                  }
               }
            }
            if (n == EOF) {
               int e= errno;
               chk_stdin();
               if (e) perror_exit(e, "Parsing error");
            }
            else bad_syntax: die("Input format syntax error!");
         }
         goto done;
   }
   {
      int const cmd_indent= 'i';
      int const cmd_word= ':';
      int const cmd_newline= 'n';
      int const nl_rplc= ' ';
      /* States:
       * 0: Start of file.
       * 1: Converting adjacent newline charaters into nl_rplc characters.
       * 2: Outputting whitespace at the end of a cmd_word or cmd_indent.
       * 3: Outputing non-whitespace characters of a cmd_word command. */
      int state= 0;
      int cc, nc, count;
      if ((nc= getchar()) == EOF) {
         chk_stdin();
         goto done;
      }
      while ((cc= nc) != EOF) {
         if ((nc= getchar()) == EOF) chk_stdin();
         switch (mode) {
            case 'w': case 'c':
               if (cc == 0x0a) {
                  /* What we consider to be a newline character. */
                  switch (state) {
                     case 2: case 3: ck_putc('\n'); /* Fall through. */
                     case 0: ck_putc(cmd_newline); state= 1; break;
                     default: assert(state == 1); ck_putc(nl_rplc);
                  }
               } else if (cc <= 0x20 || cc == 0x7f) {
                  /* What we consider to be another kind of whitspace
                   * character. */
                  switch (state) {
                     case 1: ck_putc('\n'); /* Fall through. */
                     case 0: ck_putc(cmd_indent); /* Fall through. */
                     case 3: state= 2; /* Fall through. */
                     default: assert(state == 2); ck_putc(cc);
                  }
               } else {
                  /* What we consider to be a normal non-whitespace
                   * character. */
                  switch (state) {
                     case 1: case 2: ck_putc('\n'); /* Fall through. */
                     case 0: ck_putc(cmd_word); state= 3; /* Fall through. */
                     default: assert(state == 3); ck_putc(cc);
                  }
               }
               break;
            default: die("Not yet implemented!");
         }
      }
   }
   done:
   if (fflush(0)) die("Error writing to output stream!");
   return EXIT_SUCCESS;
}
