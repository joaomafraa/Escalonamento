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
typedef struct lista_tarefas{
    tarefas *itens;
    size_t quantidade;
}lista_tarefas;

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

int adicionar_tarefas(lista_tarefas *lista,tarefas tarefa){
    if(lista->quantidade>=(size_t)-1/sizeof(tarefas)){
        fprintf(stderr,"Erro: quantidade de tarefas muito grande.\n");
        return 0;
    }
    tarefas* novo=realloc(lista->itens,(lista->quantidade+1) * sizeof(tarefas));
    if (novo==NULL){
        fprintf(stderr,"Erro: memoria insuficiente.\n");
        return 0;
    }
    lista->itens=novo;
    lista->itens[lista->quantidade]=tarefa;
    lista->quantidade=lista->quantidade+1;
    return 1;
}
int ler_arq(char *nome,int *tempo_total,lista_tarefas *lista){
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

    int nlinha=1;
    while (fgets(linha, sizeof(linha),arq) != NULL) {
        nlinha++;
        if(strchr(linha,'\n') == NULL){
            int proximo = fgetc(arq);
            if (proximo != EOF || ferror(arq)) {
            fprintf(stderr, "Erro: linha %d muito longa ou ilegivel.\n",nlinha);
            fclose(arq);
            return 0;
            }
        }

        tarefas tarefa;

        if(ler_tarefa(linha, &tarefa)==0){
            fprintf(stderr, "Erro na linha %d.\n",nlinha);
            fclose(arq);
            return 0;
        }
        if(adicionar_tarefas(lista, tarefa) == 0){
        fclose(arq);
        return 0;
}
    }

    if(ferror(arq)){
        fprintf(stderr, "Erro ao ler o arquivo.\n");
        fclose(arq);
        return 0;
    }
    fclose(arq);
    return 1;
}

void liberar_tarefas(lista_tarefas *lista,int tempo){
    //percorre todas as tarefas do vetor
    for(size_t i=0;i<lista->quantidade;i++){
        //aponta para a tarefa numero i
        tarefas *tarefa=&lista->itens[i];
        //se o resto for zero eh um instate
        if(tempo%tarefa->periodo==0){
            //uma nova rodada começa com todo trabalho a fazer
            tarefa->restante=tarefa->burst;
            //calcula o instate para terminar a rodada
            tarefa->prazo=(long long)tempo+tarefa->deadline;
        }
    }
}
tarefas *escolher_rate(lista_tarefas *lista){
    tarefas *escolhida=NULL;

    for (size_t i=0; i<lista->quantidade; i++){
        tarefas *atual = &lista->itens[i];
        //Considera somente quem tem trabalho restante
        if (atual->restante>0){
            //Escolhe a primeira pronta ou uma com periodo menor
            if(escolhida==NULL || atual->periodo<escolhida->periodo){
                escolhida=atual;
            }
        }
    }
    return escolhida;
}
int executar_unidade(tarefas *tarefa) {
    //Sem tarefa ou sem trabalho pendente não executa
    if(tarefa==NULL || tarefa->restante<=0){
        return 0;
    }
    //Executa uma unidade de trabalho
    tarefa->restante--;
    //Retorna 1 se a tarefa acabou de terminar
    if(tarefa->restante == 0){
        return 1;
    }else return 0;//se todas as tarefas terminarem retorna 1 senao 0
}

int main(int argc, char*argv[]){
    int tempo;
    lista_tarefas lista;
    lista.itens = NULL;
    lista.quantidade=0;

    if(validar_args(argc, argv)==0){
        return 1;
    }

    if(ler_arq(argv[2], &tempo,&lista)==0){
        free(lista.itens); 
        return 1;
    }
    for (size_t i=0; i<lista.quantidade;i++) {
        printf("%s %d %d %d\n",lista.itens[i].nome,lista.itens[i].periodo,lista.itens[i].deadline,lista.itens[i].burst);
    }
    liberar_tarefas(&lista, 0);
    for (size_t i = 0; i < lista.quantidade; i++) {
    printf("%s: restante=%d, prazo=%lld\n",lista.itens[i].nome,lista.itens[i].restante,lista.itens[i].prazo);}
    tarefas *escolhida = escolher_rate(&lista);

    if(escolhida!=NULL) {
        printf("Escolhida pelo RATE: %s\n",escolhida->nome);
    }else{
        printf("Nenhuma tarefa pronta.\n");
    }
    printf("antes %s, restante=%d\n",escolhida->nome, escolhida->restante);
    int terminou=executar_unidade(escolhida);
    
    printf("depois: %s,restante=%d\n",escolhida->nome,escolhida->restante);
    printf("terminou %d\n",terminou);
    free(lista.itens);
    printf("Tempo total: %d\n", tempo);
    return 0;
}
