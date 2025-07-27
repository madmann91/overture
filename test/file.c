#include <overture/test.h>
#include <overture/file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TEST(file) {
    static const char* file_name = "the_file.txt";
    static const char* contents = "Hello world!";

    FILE* file = fopen(file_name, "wb");
    REQUIRE(file);
    fputs(contents, file);
    fclose(file);

    REQUIRE(file_exists(file_name));

    size_t file_size;
    char* buf = read_file(file_name, &file_size);
    REQUIRE(file_size == strlen(contents));
    REQUIRE(strcmp(buf, contents) == 0);
    free(buf);
}
