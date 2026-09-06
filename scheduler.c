#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

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

int ler_arq(char *nome){
    FILE * arq;
    arq = fopen(nome,"r");
    if(arq == NULL){
        fprintf(stderr,"Erro: ao abrir arquivo de entrada\n");
        return 0;
    }
    char linha[256];
    if(fgets(linha,256,arq) == NULL){ 
        if (ferror(arq)){
            fprintf(stderr,"Erro: ao ler o arquivo\n");//caso de erro na leitura
        }
        else{
            fprintf(stderr,"Erro: arquivo vazio\n"); //caso nao tenha nada
        }
        fclose(arq);
        return 0;
    }
    if(strchr(linha,'\n') == NULL && !feof(arq)){//strchr procurando a quebra de linha e feof nao foi detectado o fim do arquivo
        int proximo = fgetc(arq);//leu mais 1
        if (proximo != EOF || ferror(arq)) {//conseguiu ler outro ou teve algum erro de leitura
            fprintf(stderr, "Erro: primeira linha muito longa ou ilegivel.\n");
            fclose(arq);
            return 0;
        }
    }
    char *fim;
    errno =0;
    long valor=strtol(linha,&fim,10);//conteudo de linha ,end do final,base deciamal
    if (fim==linha){
        fprintf(stderr, "Erro: tempo total deve ser um numero inteiro.\n");
        fclose(arq);
        return 0;
    }
    fim += strspn(fim, " \t\r\n\v\f");//permitir espacos e quebra de linha dps do numero

    if (*fim!='\0'){
        fprintf(stderr, "Erro: conteudo extra depois do tempo total.\n");
        fclose(arq);
        return 0;
    }

    if (errno==ERANGE || valor>INT_MAX || valor<=0){
        fprintf(stderr, "Erro: tempo total deve estar entre 1 e %d.\n",INT_MAX);
        fclose(arq);
        return 0;
    }
    fclose(arq);
    return 1;
}
int main(int argc, char*argv[]){
    validar_args(argc,argv);
    ler_arq(argv[2]);
    return 0;
}   