# Simulador de escalonamento

Trabalho da disciplina de Infraestrutura de Software o programa foi desenvolvido em C e simula tarefas periódicas usando dois algoritmos:

**RATE:** dá prioridade à tarefa com menor período.
**EDF:** dá prioridade à instância com o prazo absoluto mais próximo.

Os dois permitem interromper uma tarefa e retomá-la depois, mantendo o trabalho restante. Em caso de empate, a tarefa que aparece primeiro no arquivo tem prioridade.

## Arquivos

- `scheduler.c`: contém a leitura e validação da entrada, o armazenamento das tarefas, os dois escalonadores e a geração dos resultados.
- `Makefile`: contém os comandos de compilação e limpeza.
- `README.md`: explica como compilar, executar e testar.

## Ambiente

O programa foi desenvolvido em ambiente Linux pelo WSL, usando GCC e Make.

## Como compilar

Na pasta do projeto, execute:

```bash
make
```

Isso gera o executável `scheduler`.

Para apagar o executável:

```bash
make clean
```

## Como executar

Informe o algoritmo e o caminho do arquivo de entrada:

```bash
./scheduler rate entrada.txt
./scheduler edf entrada.txt
```

Os resultados são gravados na pasta de execução:

- `rate_jcmsn.out`
- `edf_jcmsn.out`

Uma nova execução com o mesmo algoritmo sobrescreve o arquivo anterior. Durante uma execução normal, o programa não imprime mensagens no terminal.

## Formato da entrada

A primeira linha informa o tempo total da simulação. Cada linha seguinte descreve uma tarefa:

```text
NOME PERIODO DEADLINE BURST
```

Exemplo de `entrada.txt`:

```text
100
ATT 20 12 8
NAV 50 30 15
```

Nesse exemplo, ATT aparece a cada 20 unidades, precisa de 8 unidades de CPU e tem até 12 unidades após sua chegada para terminar.

Todas as tarefas chegam inicialmente no instante 0. Os números devem ser inteiros positivos e respeitar:

```text
burst<=deadline<=periodo
```

## Como interpretar a saída

O histórico mostra quanto tempo cada tarefa executou continuamente:

- `F`: a instância terminou.
- `H`: a execução foi interrompida.
- `L`: a instância perdeu o prazo.
- `K`: a execução foi encerrada pelo fim da simulação.
- `idle`: a CPU ficou ociosa.

Depois do histórico, aparecem os totais por tarefa:

- `LOST DEADLINES`- instâncias que perderam o prazo.
- `COMPLETE EXECUTION`- instâncias concluídas.
- `KILLED`- instâncias ainda pendentes no encerramento.

Uma instância que perde o prazo tem o trabalho restante descartado. A tarefa volta a concorrer quando chega sua próxima instância.

## Decisões da implementação

A simulação avança uma unidade de tempo por vez, sem usar threads reais ou esperar tempo de relógio.

Uma instância que termina exatamente no prazo é considerada concluída. Na versão atual, uma instância incompleta cujo deadline coincide com o fim da simulação conta como perdida, as demais pendências contam como KILLED.
