#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

typedef struct tarefas{
    char nome[50];
    int periodo;
    int deadline;
    int burst;

    int restante;
    long long prazo;
}tarefas;

int validar_args(int argc,char*argv[]){
    if(argc !=3){
        fprintf(stderr,"Erro: quantidade incorreta de argumentos\n");
        return 0;
    }
    if(strcmp(argv[1],"rate") != 0 && strcmp(argv[1],"edf") != 0){
        fprintf(stderr,"Erro: algoritimo deve ser rate ou edf\n");
        return 0;
    }
    return 1;
}

int ler_arq(char *nome,int *tempo_total){
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
    *tempo_total=(int)valor;
    fclose(arq);
    return 1;
}

int ler_tarefa(char *linha, tarefas *tarefa){
    char *campos[4];
    char *parte = strtok(linha," \t\r\n");
    int quantidade = 0;

    while (parte!=NULL && quantidade<4) {
        campos[quantidade++] = parte;
        parte=strtok(NULL, " \t\r\n");
    }

    if(quantidade != 4 || parte!=NULL){
        fprintf(stderr, "Erro: esperado NOME PERIODO DEADLINE BURST.\n");
        return 0;
    }

    if (strlen(campos[0]) >= sizeof(tarefa->nome)) {
        fprintf(stderr, "Erro: nome muito longo.\n");
        return 0;
    }

    int valores[3];

    for (int i=0; i<3;i++) {
        char *fim;
        errno=0;
        long valor=strtol(campos[i+1], &fim,10);

        if (fim == campos[i+1] ||errno == ERANGE || *fim != '\0' || valor <= 0 || valor > INT_MAX){
            fprintf(stderr, "Erro: numero invalido.\n");
            return 0;
        }

        valores[i]=(int)valor;
    }
    if(valores[2]>valores[1] || valores[1]>valores[0]){
        fprintf(stderr, "Erro: tempos inconsistentes.\n");
        return 0;
    }
    strcpy(tarefa->nome,campos[0]);
    tarefa->periodo=valores[0];
    tarefa->deadline=valores[1];
    tarefa->burst=valores[2];
    tarefa->restante=0;
    tarefa->prazo=0;
    return 1;
}
int main(int argc, char*argv[]){
    char linha[] = "ATT 20 12 8";
    tarefas tarefa;
    if (ler_tarefa(linha, &tarefa) == 0) {
        return 1;
    }
    printf("Nome: %s\n",tarefa.nome);
    printf("Periodo: %d\n",tarefa.periodo);
    printf("Deadline: %d\n",tarefa.deadline);
    printf("Burst: %d",tarefa.burst);
    int tempo=0;
    validar_args(argc,argv);
    ler_arq(argv[2],&tempo);
    return 0;
}   