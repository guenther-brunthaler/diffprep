#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

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
      int cc, nc, state= 0;
      if ((nc= getchar()) == EOF) {
         chk_stdin();
         goto done;
      }
      while ((cc= nc) != EOF) {
         if ((nc= getchar()) == EOF) chk_stdin();
         switch (mode) {
            case 'w': case 'c': case 'x': case 'b':
               
               break;
            default: die("Not yet implemented!");
         }
      }
   }
   done:
   if (fflush(0)) die("Error writing to output stream!");
   return EXIT_SUCCESS;
}
