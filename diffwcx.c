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
   int mode= 'w';
   if (argc > 1) {
      int optind, argpos;
      char *arg= argv[optind= 1];
      for (argpos= 0;; ) {
         switch (arg[argpos]) {
            case '-':
               if (argpos) goto bad_option;
               switch (arg[++argpos]) {
                  case '-':
                     if (arg[argpos + 1]) goto bad_option; /* "--<text>". */
                     ++optind; /* "--". */
                     goto end_of_options;
                  case '\0': goto end_of_options; /* "-". */
               }
               /* At first option switch character now. */
               break;
            case '\0':
               if (++optind == argc) goto end_of_options;
               arg= argv[optind];
               argpos= 0;
               break;
            case 'w': case 'c': case 'x': case 'b':
               if (!argpos) goto end_of_options;
               mode= arg[argpos++];
               break;
            default:
               if (!argpos) goto end_of_options;
               bad_option:
               die("Unsupported option -%c!", (int)(unsigned char)arg[argpos]);
         }
      }
      end_of_options:
      if (optind != argc) die("To many arguments!");
   }
   return EXIT_SUCCESS;
}
