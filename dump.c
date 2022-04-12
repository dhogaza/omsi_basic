// Dump "compiled" (and I use that word loosely)  OMSI Basic files
// in os8pip's image output format (same as simh dsk format).

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <libgen.h>
#include <sys/file.h>

typedef unsigned short pdp8_word_t;

int fd;

pdp8_word_t length;
pdp8_word_t  count = 1;
pdp8_word_t statement_count;
pdp8_word_t statement_length;

pdp8_word_t read_word()
{
    pdp8_word_t  data;
    if (read(fd, &data, 2) == 0) {
        perror("read:");
        exit(1);
    }
    return data;
}

pdp8_word_t read_counted()
{
    if (count++ == length) {
        printf("read too many words\n");
        exit(1);
    }
    return read_word();
}

pdp8_word_t read_stmt_counted()
{
    if (statement_count++ == statement_length) {
        printf("read too many statement words\n");
        exit(1);
    }
    return read_counted();
}

void plit_micro_opcode(pdp8_word_t data)
{
    printf(" \"");
    pdp8_word_t cnt = 0;

    // A bit weird but the logic is copied from the runtime code.
    // Depends in a subtle way on mark bit being set.

    pdp8_word_t w1, w2;
    for (;;) {
        char ch;

        w1 = read_stmt_counted();
        if ((ch = w1 & 0177) == 0) {
            break;
        }
        printf("%c", ch);

        w2 = read_stmt_counted();

        if ((ch = w2 & 0177) == 0) {
            break;
        }
        printf("%c", ch);

        if ((ch = ((w1 & 03400) >> 4) | (w2 >> 8)) == 0) {
            break;
        };
        printf("%c", ch);
    }
    printf("\"");
}

void normal_micro_opcode(pdp8_word_t data)
{
}

typedef void (*handle_micro_opcode_t)(pdp8_word_t);
typedef struct {
    char* micro_opcode;
    handle_micro_opcode_t handle_micro_opcode;
} micro_opcode_map_t;

micro_opcode_map_t micro_opcode_map[] = {
    {"FLADD", &normal_micro_opcode},
    {"FLSUB", &normal_micro_opcode},
    {"FLMUL", &normal_micro_opcode},
    {"FLDIV", &normal_micro_opcode},
    {"FLPOW", &normal_micro_opcode},
    {"=", &normal_micro_opcode},
    {"<>", &normal_micro_opcode},
    {"<", &normal_micro_opcode},
    {">", &normal_micro_opcode},
    {"<=", &normal_micro_opcode},
    {">=", &normal_micro_opcode},
    {"EXIT", &normal_micro_opcode},
    {"EXIT STRING", &normal_micro_opcode},
    {"FNEG", &normal_micro_opcode},
    {"FSQT", &normal_micro_opcode},
    {"FITR", &normal_micro_opcode},
    {"FSIN", &normal_micro_opcode},
    {"FCOS", &normal_micro_opcode},
    {"FTAN", &normal_micro_opcode},
    {"FABS", &normal_micro_opcode},
    {"FRAN", &normal_micro_opcode},
    {"FEXP", &normal_micro_opcode},
    {"FLOG", &normal_micro_opcode},
    {"FSGN", &normal_micro_opcode},
    {"FATN", &normal_micro_opcode},
    {"CHR", &normal_micro_opcode},
    {"PI", &normal_micro_opcode},
    {"LEN", &normal_micro_opcode},
    {"TABFN", &normal_micro_opcode},
    {"POS", &normal_micro_opcode},
    {"ASCII", &normal_micro_opcode},
    {"MID", &normal_micro_opcode},
    {"PLIT", &plit_micro_opcode},
    {"LINKER", &normal_micro_opcode},
    {"SETFIL", &normal_micro_opcode},
    {"SETQUO", &normal_micro_opcode},
    {"EVAL1", &normal_micro_opcode}
};

void micro_opcode(pdp8_word_t data)
{
    printf("%s", micro_opcode_map[data & 077].micro_opcode);
    micro_opcode_map[data & 077].handle_micro_opcode(data);
}

void normal_opcode(pdp8_word_t data)
{
    printf("address: %03o", data);
}

typedef void (*handle_opcode_t)(pdp8_word_t);
typedef struct {
    char* opcode;
    handle_opcode_t handle_opcode;
} opcode_map_t;

opcode_map_t opcode_map[] = {
    {"LDOP", &normal_opcode},
    {"LIT", &normal_opcode},
    {"LDOPSB", &normal_opcode},
    {"UNSET", &normal_opcode},
    {"LADDR", &normal_opcode},
    {"MICRO", &micro_opcode},
    {"LSADDR", &normal_opcode},
    {"PACSET", &normal_opcode}
};

void eval()
{
    pdp8_word_t op;
    pdp8_word_t opcode;
    pdp8_word_t data;
    do {
        op = read_stmt_counted();
        opcode = op >> 9;
        data = op & 0777;
        printf("op: %04o opcode: %s ", op, opcode_map[opcode].opcode);
        opcode_map[opcode].handle_opcode(data);
        printf("\n");
    } while (opcode != 5 || (data & 0177) != 013 && (data & 0177) != 014);
}

void normal_stmt()
{
    printf("\n");
}

void chain_stmt()
{
    printf("\nFILENAME:\n");
    eval();
    if (statement_count < statement_length) {
        printf("LINE NUMBER:\n");
        eval();
    }
}

void expr_stmt()
{
    printf("\nEXPRESSION:\n");
    eval();
}

void print_stmt()
{
    printf("\nEXPRESSIONS:\n");
    eval();
}

void printnum_stmt()
{
    printf("\nFILE NUMBER:\n");
    eval();
    printf("EXPRESSIONS:\n");
    eval();
}

void input_stmt()
{
    printf("\nVARIABLES:\n");
    eval();
}

void inputnum_stmt()
{
    printf("\nFILE NUMBER:\n");
    eval();
    printf("VARIABLES:\n");
    eval();
}

void dimnum_stmt()
{
    printf("\nFILE NUMBER:\n");
    eval();
    read_stmt_counted(); // used internally by runtime
    printf("BLOCKS REQUIRED: %d\n", read_stmt_counted());
}


void open_stmt()
{
    read_stmt_counted() == 07777 ? printf(" BINARY") : printf(" TEXT");
    printf(" FILE\n");
    printf("STRING EXPRESSION:\n");
    eval();
    printf("FILE NUMBER EXPRESSION:\n");
    eval();
}

void on_stmt()
{
    printf("\nEXPRESSION:\n");
    eval();
    while (statement_count < statement_length) {
        printf("TARGET ADDRESS: %06o\n", read_stmt_counted());
    }
}

void go_stmt()
{
    printf("\nTARGET ADDRESS: %06o\n", read_stmt_counted());
}

void for_stmt()
{
    printf("\nCONTROL VARIABLE ADDRESS: %06o\n", read_stmt_counted());
    printf("INITIAL, LIMIT, STEP:\n");
    eval();
    unsigned filler_count = 0;
    pdp8_word_t next_word;
    while ((next_word = read_stmt_counted()) == 0) {
        filler_count++;
    }
    printf("%d WORDS RESERVED\n", filler_count);
    printf("FOR EXIT ADDRESS: %06o\n", next_word);
}

void next_stmt()
{
    printf("\nCONTROL VARIABLE ADDRESS: %06o\n", read_stmt_counted());
    printf("LIMIT VARIABLE ADDRESS: %06o\n", read_stmt_counted());
}

typedef void (*handle_stmt_t)();

typedef struct {
    char* stmt;
    handle_stmt_t handle_stmt;
} stmt_map_t;

stmt_map_t stmt_map[] = {
    {"LET", &expr_stmt},
    {"PRINT #", &printnum_stmt},
    {"PRINT", &print_stmt},
    {"DATA", &normal_stmt},
    {"INPUT #", &inputnum_stmt},
    {"INPUT", &input_stmt},
    {"STOP", &normal_stmt},
    {"END", &normal_stmt},
    {"REM", &normal_stmt},
    {"DIM #N", &dimnum_stmt},
    {"DIM", &normal_stmt},
    {"RETURN", &normal_stmt},
    {"RANDOMIZE", &normal_stmt},
    {"OPEN", &open_stmt},
    {"CLOSE", &expr_stmt},
    {"KILL", &expr_stmt},
    {"CHAIN", &chain_stmt},
    {"RESTORE", &normal_stmt},
    {"READ", &input_stmt},
    {"GOTO", &go_stmt},
    {"GOSUB", &go_stmt},
    {"FOR", &for_stmt},
    {"NEXT", &next_stmt},
    {"IF", &expr_stmt},
    {"ON GOTO", &on_stmt},
    {"ON GOSUB", &on_stmt}
};

int main(int argc, char **argv)
{

    assert(argc==2);

    if ((fd = open(argv[1], O_RDONLY)) == -1) {
       perror("open");
       exit(1);
    }

    lseek(fd, 18*2, SEEK_SET);

    if (read_word() != 07777) {
        printf("first word was not 07777\n");
        exit(1);
    }

    length = read_word();
    printf("Compiled code length: %d\n", length);

    do {
        pdp8_word_t lineno = read_counted();
        pdp8_word_t statement_word = read_counted();
        pdp8_word_t statement = statement_word & 037;

        statement_count = 1;
        statement_length = statement_word >> 5;

        printf("\naddress: %06o line number: %d statement: %s length: %d", count + 16,
                lineno, stmt_map[statement].stmt, statement_length);
        stmt_map[statement].handle_stmt();

        for (int i = statement_count; i < statement_length; i++) {
            pdp8_word_t t = read_stmt_counted();
            printf("%06o\n", t);
        }
    } while (count < length);
}
