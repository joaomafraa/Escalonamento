#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#define login "jcmsn"
typedef struct tarefas{
    char nome[50];
    int periodo;
    int deadline;
    int burst;
    int perdidas;
    int completas;
    int killed;

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
    tarefa->perdidas=0;
    tarefa->completas=0;
    tarefa->killed=0;
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

void verificar_deadlines(lista_tarefas *lista, int tempo){
    for(size_t i=0;i<lista->quantidade;i++){
        tarefas *tarefa=&lista->itens[i];
        //se ainda falta trabalho e o prazo chegou perdeu
        if(tarefa->restante>0 && tempo>=tarefa->prazo){
            tarefa->restante = 0;//descarta o trabalho pendente
            tarefa->perdidas++;//conta a perda dessa rodada
        }
    }
}
tarefas *escolher_edf(lista_tarefas *lista){//mesma funcao somente usando agora a logica do prazo
    tarefas *escolhida=NULL;

    for (size_t i=0; i<lista->quantidade; i++){
        tarefas *atual = &lista->itens[i];
        //Considera somente quem tem trabalho restante
        if (atual->restante>0){
            //Escolhe a primeira pronta ou uma com periodo menor
            if(escolhida==NULL || atual->prazo<escolhida->prazo){
                escolhida=atual;
            }
        }
    }
    return escolhida;
}
int escrever_trecho(FILE *saida, tarefas *tarefa,int duracao, char motivo){
    //nn registra trechos vazio
    if(duracao<=0){
        return 1;
    }
    int resultado;
    if(tarefa==NULL){
        //sem tarefa CPU fica ociosa
        resultado=fprintf(saida, "idle for %d units\n", duracao);
    }else{
        resultado=fprintf(saida, "[%s] for %d units - %c\n",tarefa->nome, duracao,motivo);
    }
    //fprintf retorna um valor negativo quando a escrita falha.
    if(resultado<0){
        fprintf(stderr, "Erro ao escrever o trecho.\n");
        return 0;
    }
    return 1;
}
int simular(lista_tarefas *lista,int tempo_total,char algoritimo[],FILE* saida){
    tarefas *anterior =NULL;
    int duracao=0;
    char *titulo;

    if(strcmp(algoritimo, "rate") == 0){
        titulo = "RATE";
    }else{
    titulo = "EDF";
    }if(fprintf(saida, "EXECUTION BY %s\n\n", titulo)<0){
        fprintf(stderr, "Erro ao escrever\n");
        return 0;
    }
    for(int tempo =0;tempo<tempo_total;tempo++){
        if(anterior!=NULL && duracao>0 &&tempo >= anterior->prazo){
            if (!escrever_trecho(saida, anterior, duracao, 'L')){
                return 0;
            }

            anterior = NULL;
            duracao = 0;
        }
        //descartamos primeiro rodadas que perderam o prazo
        verificar_deadlines(lista,tempo);
        //preparacao de novas rodadas
        liberar_tarefas(lista,tempo);
        //escolher a nova tarefa pelo rate
        tarefas *escolhida;
        if(strcmp(algoritimo,"rate")==0){
        escolhida=escolher_rate(lista);
        }else{
        escolhida=escolher_edf(lista);
        }
         if(escolhida!=anterior && duracao>0){
            if (!escrever_trecho(saida, anterior, duracao, 'H')) {
                return 0;
            }
            duracao = 0;
        }
        anterior = escolhida;
        duracao++;
        //executa uma unidade e se der bom incrementa mais 1 
        if (executar_unidade(escolhida)){
            escolhida->completas++;
            if(!escrever_trecho(saida, escolhida, duracao, 'F')){
                return 0;
            }
            anterior = NULL;
            duracao = 0;
        }
    }
    if(duracao>0){
        char motivo = 'K';
        if (anterior!=NULL && tempo_total >= anterior->prazo){
                motivo = 'L';
        }
        if(!escrever_trecho(saida,anterior,duracao,motivo)){
        return 0;
        }
    }
        //trata as deadlines no instante final pr dps contar as pendencias como killed
    verificar_deadlines(lista, tempo_total);
    for(size_t i=0;i<lista->quantidade;i++){
        if(lista->itens[i].restante > 0){
            lista->itens[i].killed++;
            lista->itens[i].restante = 0;
        }
    }
    return 1;
}
int escrever_resumo(FILE *saida, lista_tarefas *lista) {
    fprintf(saida, "\nLOST DEADLINES\n");

    for (size_t i=0;i<lista->quantidade;i++) {
        fprintf(saida, "[%s] %d\n",lista->itens[i].nome,lista->itens[i].perdidas);}

    fprintf(saida,"\nCOMPLETE EXECUTION\n");

    for (size_t i=0;i<lista->quantidade;i++){
        fprintf(saida, "[%s] %d\n",lista->itens[i].nome,lista->itens[i].completas);}

    fprintf(saida, "\nKILLED\n");

    for (size_t i=0; i<lista->quantidade;i++){
        fprintf(saida, "[%s] %d\n",lista->itens[i].nome,lista->itens[i].killed);
    }
    if(ferror(saida)){
        fprintf(stderr, "Erro ao escrever o resumo.\n");
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int tempo;
    lista_tarefas lista;
    lista.itens=NULL;
    lista.quantidade=0;

    if(validar_args(argc, argv)==0){
        return 1;
    }

    if(ler_arq(argv[2], &tempo, &lista)==0){
        free(lista.itens);
        return 1;
    }
    char* nome_saida;
    if(strcmp(argv[1], "rate") == 0){
        nome_saida = "rate_" login ".out";
    }else{
        nome_saida = "edf_" login ".out";
    }
    FILE * saida=fopen(nome_saida,"w");
    if(saida == NULL){
        perror("Erro ao criar arquivo de saida");
        free(lista.itens);
        return 1;
    }
    int sucesso =simular(&lista,tempo,argv[1],saida);
    if(sucesso){
        sucesso = escrever_resumo(saida, &lista);
    }if(fclose(saida) == EOF){
        perror("Erro ao fechar arquivo de saida");
        sucesso = 0;
    }
    free(lista.itens);
    if(!sucesso) {
        if (remove(nome_saida)!=0) {
            perror("Erro ao remover saida incompleta");
        }
        return 1;
    }
    return 0;
}
