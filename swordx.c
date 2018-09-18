#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <mcheck.h>
#include <locale.h>
#include "bintree.h"
#include "linkedlist.h"

#define recursive_flag  (1<<0)
#define follow_flag     (1<<1)
#define alpha_flag      (1<<2)
#define sbo_flag        (1<<3)

// struct to store options
typedef struct args {
    char *explude;
    char *ignore;
    char *log;
    char *output;
    unsigned int min;
} args;

void run(args *option, unsigned int flag);
FILE *getFile(char *path); 
void scanFile(FILE *f, t_node **root, unsigned int flag);
void scanDir(const char *name, t_node **root, unsigned int flag);
bool isWordAlpha(char *word);
int isFlagSet(unsigned int bitoption, unsigned int flag);
void printUsage();
void printHelp();

// if return not equal to zero, flag is set
int isFlagSet(unsigned int bitopt, unsigned int flag) {
    return flag & bitopt;
}

// get file from a specific path
FILE *getFile(char *path) {
    FILE *f = fopen(path, "r");
    if (f == NULL)
        perror("Error reading file");
    return f;
}

void run(args *option, unsigned int flag) {
    t_node **root = createTree();
    scanDir(".", root, flag);
    treePrint(*root);
    if (isFlagSet(sbo_flag, flag) != 0) {
        l_list **head = createList();
        addToList(*root, head);
        sortByOccurrences(head);
        printList(*head);
        destroyList(*head);
        free(head);
    }
    destroyTree(*root);
    free(root);
}

// check if word contains only alphabetic characters
bool isWordAlpha(char *w) {
    do {
        if (isalpha(*w) == 0)
            return false;
    } while (*++w != '\0');
    return true;
}

// Scan file to add word inside the binary tree 
void scanFile(FILE *f, t_node **root, unsigned int flag) {
    char *word;
    int n;
    errno = 0;
    do {
        n = fscanf(f, "%ms", &word);
        if (n == 1) {
            printf("Word: %s\n", word);
            if (isFlagSet(alpha_flag, flag) && !isWordAlpha(word))
                continue;
            addToTree(root, word);
            free(word);
        } else if (errno != 0)
            perror("Error in scanf");
    } while (n != EOF);
    fclose(f);
}

// check every file in directory, if recursive flag is set then
// check the dir recursively
void scanDir(const char *name, t_node **root, unsigned int flag) {
    FILE *f;
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        perror("Problem opening dir");
    
    while ((entry = readdir(dir)) != NULL) {
                char path[1024];
                if (entry->d_type == DT_DIR && isFlagSet(recursive_flag, flag) != 0) {
                    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                        sprintf(path, "%s/%s", name, entry->d_name);
                        printf("dir path: %s\n", path);
                        scanDir(path, root, flag);
                    }
                } else if (entry->d_type == DT_REG) {
                    sprintf(path, "%s/%s", name, entry->d_name);
                    printf("file path: %s\n",  entry->d_name);
                    f = getFile(path);
                    scanFile(f, root, flag);
                }
        }
    closedir(dir);
}


int main (int argc, char **argv) {
    int opt = 0;
    int long_index = 0;
    struct args *option = (struct args *) malloc(sizeof(struct args));
    unsigned int flag = 0; /* byte flag */
    setlocale(LC_ALL, "");
    
    static struct option long_options[] =
	{
        {"help",                no_argument,        0,  'h'},
        {"recursive",           no_argument,        0,  'r'},
        {"follow",              no_argument,        0,  'f'},
        {"sortbyoccurrency",    no_argument,        0,  's'},
        {"alpha",               no_argument,        0,  'a'},
        {"explude",             required_argument,  0,  'e'},
        {"min",                 required_argument,  0,  'm'},
        {"ignore",              required_argument,  0,  'i'},
        {"log",                 required_argument,  0,  'l'},
        {"output",              required_argument,  0,  'o'},
        {0, 0, 0, 0}
	};
	
	while ((opt = getopt_long(argc, argv, "hrfsae:m:i:l:o:", long_options, &long_index)) != -1) {
		switch (opt) {
			case 'h': printHelp();
				break;
			case 'r': flag |= recursive_flag;
				break;
            case 'f': flag |= follow_flag;
                break;
            case 'a': flag |= alpha_flag;
                break;
            case 's': flag |= sbo_flag;
                break;
            case 'e': option->explude = strdup(optarg);
                break;
            case 'm': option->min = atoi(optarg);
                break; 
            case 'i': option->ignore = strdup(optarg);
                break; 
            case 'l': option->log = strdup(optarg);
                break;
            case 'o': option->output = strdup(optarg);
                break;
            default: printUsage();
				exit(EXIT_FAILURE);
			}
	}
    // Remaining arguments (inputs)
    if (optind < argc) {
        printf("INPUTS\n");
        while (optind < argc) 
            printf("%s\n", argv[optind++]);
    }
    run(option, flag);
    free(option);
    exit(EXIT_SUCCESS);
}

void printUsage () {
	printf("USAGE: ./swordx [OPTIONS] [INPUTS]");
}

void printHelp () {
    printf("help\n");
}
