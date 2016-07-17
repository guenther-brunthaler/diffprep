#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static void die(char const *msg, ...) {
   va_list args;
   va_start(args, msg);
   (void)vfprintf(stderr, msg, args);
   va_end(args);
   (void)fputc('\n', stderr);
   exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
   int reverse= 0;
   int split= 'w';
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
               reverse=1;
               /* Fall through. */
            case 'w': case 'c': case 'x': case 'b':
               split= c;
               break;
            default: bad_option: die("Unsupported option -%c!", c);
         }
         ++argpos;
      }
      end_of_options:
      if (optind != argc) die("To many arguments!");
   }
   return EXIT_SUCCESS;
}
