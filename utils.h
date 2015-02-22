#ifndef UTILS_H_
#define UTILS_H_

// defines

#define KB 1024
#define MB (1024 * KB)

// number of elements in statically declared array
#define DIM(S_ARR_) (sizeof(S_ARR_) / sizeof(S_ARR_[0]))

#define MIN(A_, B_) ((A_) < (B_) ? (A_) : (B_))
#define MAX(A_, B_) ((A_) > (B_) ? (A_) : (B_))

// read/write u32/16 big/little endian
#define read_u32_be(buf) (unsigned int)(((buf)[0] << 24) + ((buf)[1] << 16) + ((buf)[2] << 8) + ((buf)[3]))
#define read_u32_le(buf) (unsigned int)(((buf)[1] << 24) + ((buf)[0] << 16) + ((buf)[3] << 8) + ((buf)[2]))
#define write_u32_be(buf, val) do { \
   (buf)[0] = ((val) >> 24) & 0xFF; \
   (buf)[1] = ((val) >> 16) & 0xFF; \
   (buf)[2] = ((val) >> 8) & 0xFF; \
   (buf)[3] = (val) & 0xFF; \
} while(0)
#define read_u16_be(buf) (((buf)[0] << 8) + ((buf)[1]))
#define write_u16_be(buf, val) do { \
   (buf)[0] = ((val) >> 8) & 0xFF; \
   (buf)[1] = ((val)) & 0xFF; \
} while(0)

// print nibbles and bytes
#define print_nibble(NIB_) putchar((NIB_) < 10 ? ('0' + (NIB_)) : ('A' + (NIB_) - 0xA))
#define print_byte(BYTE_) do { \
    print_nibble((BYTE_) >> 4); \
    print_nibble((BYTE_) & 0x0F); \
    putchar(' '); \
  } while(0)

// functions

// print buffer as hex bytes
// buf: buffer to read bytes from
// length: length of buffer to print
void print_hex(unsigned char *buf, int length);

// perform byteswapping to convert from v64 to z64 ordering
void swap_bytes(unsigned char *data, long length);

// read entire contents of file into buffer
// returns file size or negative on error
long read_file(const char *file_name, unsigned char **data);

// write buffer to file
// returns number of bytes written out or -1 on failure
long write_file(const char *file_name, unsigned char *data, long length);

#endif // UTILS_H_