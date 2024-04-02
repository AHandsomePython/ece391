#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

uint32_t ece391_strlen(const uint8_t* s)
{
    uint32_t len;

    for (len = 0; '\0' != *s; s++, len++);
    return len;
}

void ece391_strcpy(uint8_t* dst, const uint8_t* src)
{
    while ('\0' != (*dst++ = *src++));
}

void ece391_fdputs(int32_t fd, const uint8_t* s)
{
    (void)ece391_write (fd, s, ece391_strlen(s));
}

void ece391_fdputc(int32_t fd, int8_t c){
    int8_t str[2] = {c, '\0'};
    (void)ece391_write (fd, str, 1);
}

int32_t ece391_strcmp(const uint8_t* s1, const uint8_t* s2)
{
    while (*s1 == *s2) {
        if (*s1 == '\0')
            return 0;
        s1++;
        s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

int32_t ece391_strncmp(const uint8_t* s1, const uint8_t* s2, uint32_t n)
{
    if (0 == n)
        return 0;
    while (*s1 == *s2) {
        if (*s1 == '\0' || --n == 0)
        return 0;
    s1++;
    s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

/* Convert a number to its ASCII representation, with base "radix" */
uint8_t* ece391_itoa(uint32_t value, uint8_t* buf, int32_t radix)
{
        static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        uint8_t *newbuf = buf;
        int32_t i;
        uint32_t newval = value;

        /* Special case for zero */
        if(value == 0) {
                buf[0]='0';
                buf[1]='\0';
                return buf;
        }

        /* Go through the number one place value at a time, and add the
         * correct digit to "newbuf".  We actually add characters to the
         * ASCII string from lowest place value to highest, which is the
         * opposite of how the number should be printed.  We'll reverse the
         * characters later. */
        while (newval > 0) {
                i = newval % radix;
                *newbuf = lookup[i];
                newbuf++;
                newval /= radix;
        }

        /* Add a terminating NULL */
        *newbuf = '\0';

        /* Reverse the string and return */
        return ece391_strrev(buf);
}

/* In-place string reversal */
uint8_t* ece391_strrev(uint8_t* s)
{
    register uint8_t tmp;
    register int32_t beg = 0;
    register int32_t end = ece391_strlen(s) - 1;

    if (end <= 0) {
        return s;
    }

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
   }

   return s;
}



/*
show char on screen
inputLscreen, str
output:1 for success
side effect:change video memory
*/
// This function modified the printf function in lib.c
int32_t ece391_printf(int8_t *format, ...) {
    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            ece391_fdputc(1,'%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                uint8_t conv_buf[64];
                                if (alternate == 0) {
                                    ece391_itoa(*((uint32_t *)esp), conv_buf, 16);
                                    ece391_fdputs(1,conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    ece391_itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = ece391_strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    ece391_fdputs(1,&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                uint8_t conv_buf[36];
                                ece391_itoa(*((uint32_t *)esp), conv_buf, 10);
                                ece391_fdputs(1,conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                uint8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    ece391_itoa(-value, &conv_buf[1], 10);
                                } else {
                                    ece391_itoa(value, conv_buf, 10);
                                }
                                ece391_fdputs(1,conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            ece391_fdputc(1,(uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            ece391_fdputs(1,*((uint8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                ece391_fdputc(1,*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}
