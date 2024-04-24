#ifndef ___PAGINA_H
#define ___PAGINA_H

#define ORDEM 2
#define ARQARVORE "arquivoArvore"
#define TABELAREGISTROS "tabelaRegistros"

typedef struct registro{
    int id;
    int ano;
    char autor[200];
    char titulo[200];
    char revista[200];
    char DOI[200];
    char palavraChave[200];
}Registro;

typedef struct cabecalho{
    int quantidadePaginas;
    int paginasExcluidas;
    int raiz;
}Cabecalho;

typedef struct pagina{
    int pai;
    int chaves[ORDEM * 2 + 3];
    int enderecos[ORDEM * 2 + 3];
    int ativa;
    int m;
    int proxima;
    int folha;
}Pagina;



void criaArquivo();

void criaPagina();

void inserir(Registro);

void imprimeArvore(int);

int buscaB(int buscado, int *estado, int *posicaoPag, int *pagEndereco, int *posicaoPagEndereco);

void remover(int remove);

void imprimeOrdenado(int id);

void inicializaRegistro(Registro*, int ano, char autor[], char DOI[], char palavraChave[], int id, char revista[], char titulo[]);

void imprimePagina(Pagina);

int buscaRaiz();


#endif