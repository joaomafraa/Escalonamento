#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct tarefas{
    char nome[50];
}tarefas;

int validar_args(int argc,char*argv[]){
    if(argc !=3){
        fprintf(stderr,"Erro: argumentos insuficientes\n");
        return 0;
    }
    if(strcmp(argv[1],"rate") != 0 && strcmp(argv[1],"edf") != 0){
        fprintf(stderr,"Erro: algoritimo deve ser rate ou edf\n");
        return 0;
    }
    return 1;
}
int main(int argc, char*argv[]){
    validar_args(argc,argv);
    return 0;
}   