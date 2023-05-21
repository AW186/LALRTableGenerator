#ifndef READER_HPP
#define READER_HPP

#include <stdio.h>

class Reader {
public:
    virtual char getc() = 0;
};

class FileReader: public Reader {
    FILE *file;
public:
    FileReader(const char *filename) {
        file = fopen(filename, "r");
    }
    char getc() override {
        if (!file) return EOF;
        return fgetc(file);
    }
    ~FileReader() {
        if (file) fclose(file);
    }
};

#endif