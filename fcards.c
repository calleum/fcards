#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "json.h"
#include "json-builder.h"

static void print_depth_shift(int depth)
{
        int j;
        for (j=0; j < depth; j++) {
                printf(" ");
        }
}

static void process_value(json_value* value, int depth);

static void process_object(json_value* value, int depth)
{
        int length, x;
        if (value == NULL) {
                return;
        }
        length = value->u.object.length;
        for (x = 0; x < length; x++) {
                print_depth_shift(depth);
                process_value(value->u.object.values[x].value, depth+1);
        }
}

static void process_array(json_value* value, int depth)
{
		char* line = NULL;
		size_t len = 0;
        int length, x;
        if (value == NULL) {
                return;
        }
        length = value->u.array.length;
        printf("Question\n\n");
        for (x = 0; x < length; x++) {
			if (x == 1){
				printf("Press enter to see the answer:\n");
				getline(&line, &len, stdin);
				if (strcmp(line, "q") == 0)
					exit(0);
			}

                process_value(value->u.array.values[x], depth);
        }
}

static void process_value(json_value* value, int depth)
{
        int j;
        if (value == NULL) {
                return;
        }
        if (value->type != json_object) {
                print_depth_shift(depth);
        }
        switch (value->type) {
                case json_none:
                        printf("none\n");
                        break;
                case json_object:
                        process_object(value, depth+1);
                        break;
                case json_array:
                        process_array(value, depth+1);
                        break;
                case json_string:
                        printf("\t %s\n\n", value->u.string.ptr);
                        break;
        }
}

int main(int argc, char** argv)
{
		char* filename;
		char* arg;
        FILE *fp;
        struct stat filestatus;
        int file_size;
        char* file_contents;
        json_char* json;
        json_value* value;
        json_value* arr = json_array_new(0);
		json_value * objs = json_object_new(0);


        if (argc != 3) {
                fprintf(stderr, "%s read <file_json>\n", argv[0]);
                fprintf(stderr, "%s add  <file_json>\n", argv[0]);
                return 1;
        }

		arg = argv[1];
		int add = strcmp(arg, "add");
        filename = argv[2];

        printf("\n--------------------------------\n\n");

		int k;
		if(add ==0) {
			k = 0;
			while(k < 2){
				if (k == 0)
					printf("\nPlease enter the question side of the flashcard:\n\n");
				else if (k==1)
					printf("\nPlease enter the answer:\n\n");

				char *line = NULL;
				size_t len = 0;
				ssize_t lineSize = 0;
				lineSize = getline(&line, &len, stdin);
				line[strcspn(line, "\n")] = 0;
				json_array_push(arr, json_string_new(line));
				free(line);
				k++;
			}
			// json_object_push(objs,"question set", arr);
			char * buf = malloc(json_measure(arr));
			json_serialize(buf, arr);
			fp = fopen(filename, "a");

			if (fp == NULL) exit(0);

			if (fwrite(buf,sizeof(buf), sizeof(buf), fp) != sizeof(buf)){
				fprintf(stderr, "Unable t read content of %s\n", filename);
				fclose(fp);
				free(buf);
				return 1;
			}
			fclose(fp);
			free(buf);
			return 0;
		}


        if ( stat(filename, &filestatus) != 0) {
                fprintf(stderr, "File %s not found\n", filename);
                return 1;
        }
        file_size = filestatus.st_size;
        file_contents = (char*)malloc(filestatus.st_size);
        if ( file_contents == NULL) {
                fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
                return 1;
        }

        fp = fopen(filename, "rt");
        if (fp == NULL) {
                fprintf(stderr, "Unabe to open %s\n", filename);
                fclose(fp);
                free(file_contents);
                return 1;
        }
        if ( fread(file_contents, file_size, 1, fp) != 1 ) {
                fprintf(stderr, "Unable t read content of %s\n", filename);
                fclose(fp);
                free(file_contents);
                return 1;
        }
        fclose(fp);



        json = (json_char*)file_contents;

        value = json_parse(json,file_size);

        if (value == NULL) {
                fprintf(stderr, "Unable to parse data\n");
                free(file_contents);
                exit(1);
        }

        process_value(value, 0);

        json_value_free(value);
        free(file_contents);
        return 0;
}
