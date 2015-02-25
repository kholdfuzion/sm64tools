#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libmio0.h"
#include "libsm64.h"
#include "utils.h"

#define SM64EXTEND_VERSION "0.2"

// default configuration
static const sm64_config_t default_config = 
{
   NULL, // input filename
   NULL, // extended filename
   64,   // extended size
   32,   // MIO0 padding
   1,    // MIO0 alignment
   0,    // fill old MIO0 blocks
};

static void print_usage(void)
{
   ERROR("Usage: sm64extend [-s SIZE] [-p PADDING] [-a ALIGNMENT] [-f] [-v] FILE [EXT_FILE]\n"
         "\n"
         "sm64extend v" SM64EXTEND_VERSION ": Super Mario 64 ROM extender\n"
         "\n"
         "Optional arguments:\n"
         " -s SIZE      size of the extended ROM in MB (default: %d)\n"
         " -p PADDING   padding to insert between MIO0 blocks in KB (default: %d)\n"
         " -a ALIGNMENT byte boundary to align MIO0 blocks (default: %d)\n"
         " -f           fill old MIO0 blocks with 0x01\n"
         " -v           verbose progress output\n"
         "\n"
         "File arguments:\n"
         " FILE         input ROM file\n"
         " EXT_FILE     output extended ROM file (default: replaces input extension with .ext.z64)\n",
         default_config.ext_size, default_config.padding, default_config.alignment);
   exit(1);
}

// generate an output file name from input name by replacing
// file extension with .ext.z64
static void generate_filename(const char *in_name, char *out_name)
{
   char tmp_name[FILENAME_MAX];
   int len;
   int i;
   strcpy(tmp_name, in_name);
   len = strlen(tmp_name);
   for (i = len - 1; i > 0; i--) {
      if (tmp_name[i] == '.') {
         break;
      }
   }
   if (i <= 0) {
      i = len;
   }
   tmp_name[i] = '\0';
   sprintf(out_name, "%s.ext.z64", tmp_name);
}

// parse command line arguments
static void parse_arguments(int argc, char *argv[], sm64_config_t *config)
{
   int i;
   int file_count = 0;
   if (argc < 2) {
      print_usage();
      exit(1);
   }
   for (i = 1; i < argc; i++) {
      if (argv[i][0] == '-') {
         switch (argv[i][1]) {
            case 'a':
               if (++i >= argc) {
                  print_usage();
               }
               config->alignment = strtoul(argv[i], NULL, 0);
               if (!is_power2(config->alignment)) {
                  ERROR("Error: Alignment must be power of 2\n");
                  exit(2);
               }
               break;
            case 'f':
               config->fill = 1;
               break;
            case 'p':
               if (++i >= argc) {
                  print_usage();
               }
               config->padding = strtoul(argv[i], NULL, 0);
               break;
            case 's':
               if (++i >= argc) {
                  print_usage();
               }
               config->ext_size = strtoul(argv[i], NULL, 0);
               break;
            case 'v':
               g_verbosity = 1;
               break;
            default:
               print_usage();
               break;
         }
      } else {
         switch (file_count) {
            case 0:
               config->in_filename = argv[i];
               break;
            case 1:
               config->ext_filename = argv[i];
               break;
            default: // too many
               print_usage();
               break;
         }
         file_count++;
      }
   }
   if (file_count < 1) {
      print_usage();
   }
}

int main(int argc, char *argv[])
{
   char ext_filename[FILENAME_MAX];
   sm64_config_t config;
   unsigned char *in_buf = NULL;
   unsigned char *out_buf = NULL;
   long in_size;
   long bytes_written;

   // get configuration from arguments
   config = default_config;
   parse_arguments(argc, argv, &config);
   if (config.ext_filename == NULL) {
      config.ext_filename = ext_filename;
      generate_filename(config.in_filename, config.ext_filename);
   }
   config.ext_size *= MB;
   config.padding *= KB;

   // read input file into memory
   in_size = read_file(config.in_filename, &in_buf);
   if (in_size <= 0) {
      ERROR("Error reading input file \"%s\"\n", config.in_filename);
      exit(1);
   } else if (in_size > 8 * MB) {
      // TODO: better checks to see if it has been extended
      ERROR("This ROM is already extended!\n");
      exit(1);
   }

   // TODO: confirm valid SM64
   // if necessary, swap bytes
   if (in_buf[0] == 0x37)  {
      swap_bytes(in_buf, in_size);
   }

   // allocate output memory
   out_buf = malloc(config.ext_size);

   // copy file from input to output
   memcpy(out_buf, in_buf, in_size);

   // fill new space with 0x01
   memset(&out_buf[in_size], 0x01, config.ext_size - in_size);

   // decode SM64 MIO0 files and adjust pointers
   sm64_decompress_mio0(&config, in_buf, in_size, out_buf);

   // update N64 header CRC
   sm64_update_checksums(out_buf);

   // write to output file
   bytes_written = write_file(config.ext_filename, out_buf, config.ext_size);
   if (bytes_written < (long)config.ext_size) {
      ERROR("Error writing bytes to output file \"%s\"\n", config.ext_filename);
      exit(1);
   }

   return 0;
}