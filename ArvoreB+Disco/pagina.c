#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pagina.h"


void inicializaRegistro(Registro *rg, int ano, char autor[], char DOI[], char palavraChave[], int id, char revista[], char titulo[]){
    rg->ano = ano;
    rg->id = id;

    strcpy(rg->autor, autor);
    strcpy(rg->DOI, DOI);
    strcpy(rg->palavraChave, palavraChave);
    strcpy(rg->revista, revista);
    strcpy(rg->titulo, titulo);
}


int criaRegistro(int ano, char autor[], char DOI[], char palavraChave[], int id, char revista[], char titulo[]){
    FILE *fp = fopen(TABELAREGISTROS, "ab");


    Registro rg;
    inicializaRegistro(&rg, ano, autor, DOI, palavraChave, id, revista, titulo);

    fseek(fp, 0, SEEK_SET);
    fwrite(&rg, sizeof(Registro), 1, fp);

    int posicaoInsercao = ftell(fp);

    fclose(fp);

    return posicaoInsercao;

}


void criaPagina(){

    FILE *fp = fopen(ARQARVORE, "ab");

    Pagina pg;
    pg.pai = 0;
    pg.proxima = 0;
    pg.m = 0;
    pg.folha = 1;
    pg.ativa = 1;

    for(int i = 0; i < ORDEM * 2 + 3; i++){
        pg.enderecos[i] = -1;
        pg.chaves[i] = -1;
    }


    fwrite(&pg, sizeof(Pagina), 1, fp);

    fclose(fp);
}

void criaArquivo(){
    FILE *fp = fopen(TABELAREGISTROS, "wb");
    fclose(fp);

    fp = fopen(ARQARVORE, "wb");


    Cabecalho cb;
    cb.paginasExcluidas = 0;
    cb.quantidadePaginas = 0;
    cb.raiz = sizeof(Cabecalho);

    fseek(fp, 0, SEEK_SET);

    fwrite(&cb, sizeof(Cabecalho), 1, fp);

    fclose(fp);
}

int buscaRaiz(){
    FILE *fp = fopen(ARQARVORE, "rb");

    Cabecalho cb;

    fseek(fp, 0, SEEK_SET);

    fread(&cb, sizeof(Cabecalho), 1, fp);

    if(!cb.quantidadePaginas)
        return -1;

    fclose(fp);

    return cb.raiz;
}

/*
    raiz = 0: a raiz não mudou
    alterouElemento = 1: inseriu elemento;
                      0: excluiu elemento;
*/
void atualizaCabecalho(int raiz, int alterouElemento){
    FILE *fp = fopen(ARQARVORE, "r+b");

    fseek(fp, 0, SEEK_SET);

    Cabecalho cb;

    fread(&cb, sizeof(Cabecalho), 1, fp);


    if(raiz)
        cb.raiz = raiz;
    if(alterouElemento)
        cb.quantidadePaginas++;
    else
        cb.paginasExcluidas++;

    fseek(fp, 0, SEEK_SET);
    fwrite(&cb, sizeof(Cabecalho), 1, fp);

    fclose(fp);
}


/*
    estado = 1: elemento encontrado
    estado = 0: elemento não encontrado
    g indica a posição na página que o elemento se encontra
*/

int buscaB(int buscado, int *estado, int *posicaoPag, int *pagEndereco, int *posicaoPagEndereco){
    FILE *arv = fopen(ARQARVORE, "r+b");

    fseek(arv, 0, SEEK_SET);

    Cabecalho cb;
    Pagina percorrePaginas;
    int raiz = 0, parar = 0;

    fread(&cb, sizeof(Cabecalho), 1, arv);

    if(!cb.quantidadePaginas){ // arvore vázia sem página inicial criada
        *estado = 0;
        *posicaoPag = 1;
    }else{

        fseek(arv, cb.raiz, SEEK_SET);
        raiz = ftell(arv);
        fread(&percorrePaginas, sizeof(Pagina), 1, arv);

        int m = 0, i = 0;

        *estado = 0;

        while(!parar){ // percorrendo as páginas
            i = *posicaoPag = 1;
            m = percorrePaginas.m;

            while(i <= m){ // percorrendo os elementos da página

                if(buscado > percorrePaginas.chaves[i]) // se o elemento que estamos buscando é maior, continuamos percorrendo
                    *posicaoPag = ++i;
                else{ // elemento é menor ou igual ao buscado
                    if(buscado == percorrePaginas.chaves[i]){ // elemento encontrado
                        if(percorrePaginas.folha){
                            parar = 1;
                            *estado = 1;
                        }else{
                            *pagEndereco = raiz;
                            *posicaoPagEndereco = i;
                            fseek(arv, percorrePaginas.enderecos[i], SEEK_SET);
                            raiz = ftell(arv);
                            fread(&percorrePaginas, sizeof(Pagina), 1, arv);
                            i = m + 2;
                        }

                    }else if(percorrePaginas.enderecos[i-1] != -1 && !percorrePaginas.folha){ // elemento é menor que o elemento buscado, descemos na árvore
                        fseek(arv, percorrePaginas.enderecos[i-1], SEEK_SET);
                        raiz = ftell(arv);
                        fread(&percorrePaginas, sizeof(Pagina), 1, arv);
                    }else
                        parar = 1;

                    i = m+2; // força a sairmos do loop
                }
            }

            if(i == m + 1){ // o elemento buscado é maior que todos os elementos da página
                if(percorrePaginas.enderecos[m] != -1 && !percorrePaginas.folha){
                    fseek(arv, percorrePaginas.enderecos[m], SEEK_SET);
                    raiz = ftell(arv);
                    fread(&percorrePaginas, sizeof(Pagina), 1, arv);
                }else
                    parar = 1;
            }
        }

    }

    fclose(arv);

    return raiz;
}

void atualizaPosicao(Pagina *percorre, int posicao){
    int contador = posicao;

    while(contador <= percorre->m){

        // atualizando os endereços de memória
        percorre->chaves[contador] = percorre->chaves[contador + 1];
        if(!percorre->folha)
            percorre->enderecos[contador - 1] = percorre->enderecos[contador];
        else
            percorre->enderecos[contador] = percorre->enderecos[contador + 1];

        contador++;

    }

    percorre->chaves[percorre->m] = -1;
    percorre->enderecos[percorre->m] = -1;

    percorre->m--;

}

int insereOrdenado(Pagina *percorre, int insere, int endereco){
    int posicaoInsercao = percorre->m+1, indice = percorre->m;

    while(indice >= 1 && percorre->chaves[indice] > insere){
        posicaoInsercao = indice;

        // atualizando os endereços de memória
        percorre->enderecos[indice + 1] = percorre->enderecos[indice];
        percorre->chaves[indice + 1] = percorre->chaves[indice];
        indice--;

    }

    percorre->enderecos[posicaoInsercao] = endereco;
    percorre->chaves[posicaoInsercao] = insere;
    percorre->m++;

    return posicaoInsercao;
}


void cisao(int posicaoPag, int interna){

    FILE *arv = fopen(ARQARVORE, "r+b");
    Pagina pag, novaPag, aux, pai;
    int enderecoNovaPag = 0, idPromovido = 0, enderecoPai = 0, posicaoInsercaoPai = 0;


    // criando a nova pagina
    fseek(arv, 0, SEEK_END);
    enderecoNovaPag = ftell(arv);
    criaPagina();
    fread(&novaPag, sizeof(Pagina), 1, arv);


    // buscando a página
    fseek(arv, posicaoPag, SEEK_SET);
    fread(&pag, sizeof(Pagina), 1, arv);


    // separando as celulas da pag, metade das celulas são movidas para a novaPag
    for(int i = ORDEM + 1; i < (ORDEM * 2) + 2; i++){

        novaPag.enderecos[i - ORDEM] = pag.enderecos[i];
        novaPag.chaves[i - ORDEM] = pag.chaves[i];

        if(novaPag.enderecos[i - ORDEM] != -1 && !pag.folha){
            // alterando o pai da página filha
            fseek(arv, novaPag.enderecos[i - ORDEM], SEEK_SET);
            fread(&aux, sizeof(Pagina), 1, arv);

            aux.pai = enderecoNovaPag;

            fseek(arv, novaPag.enderecos[i - ORDEM], SEEK_SET);
            fwrite(&aux, sizeof(Pagina), 1, arv);
        }

        pag.enderecos[i] = -1;
        pag.chaves[i] = -1;
        pag.m--;

        novaPag.m++;
    }


    // encontrando o promovido
    if(novaPag.m > pag.m){
        idPromovido = novaPag.chaves[1];
        if(interna){
            novaPag.folha = 0;
            atualizaPosicao(&novaPag, 1);
        }
    }else{
        //printf("Promovido é o %i\n", pag.chaves[pag.m]);
        idPromovido = pag.chaves[pag.m];
        if(interna){
            pag.chaves[pag.m] = -1;
            pag.m--;
            pag.folha = 0;
        }
    }

    // ajustando os ponteiros
    novaPag.pai = pag.pai;
    novaPag.proxima = pag.proxima;
    pag.proxima = enderecoNovaPag;



    if(!pag.pai){ // não tem pai

        // criando página pai
        fseek(arv, 0, SEEK_END);
        enderecoPai = ftell(arv);
        criaPagina();

        // ajustando os ponteiros
        pag.pai = novaPag.pai = enderecoPai;

        // salvando alterações nas páginas filhas
        fseek(arv, posicaoPag, SEEK_SET);
        fwrite(&pag, sizeof(pag), 1, arv);

        fseek(arv, enderecoNovaPag, SEEK_SET);
        fwrite(&novaPag, sizeof(pag), 1, arv);

        // buscando página pai
        fseek(arv, enderecoPai, SEEK_SET);
        fread(&pai, sizeof(Pagina), 1, arv);

        // inserindo elemento promovido
        posicaoInsercaoPai = insereOrdenado(&pai, idPromovido, enderecoNovaPag);


        // ajustando ponteiros
        pai.enderecos[posicaoInsercaoPai-1] = posicaoPag;
        pai.folha = 0;

        // salvando página pai e atualizando cabeçalho
        fseek(arv, enderecoPai, SEEK_SET);
        fwrite(&pai, sizeof(Pagina), 1, arv);


        atualizaCabecalho(enderecoPai, 1);
        atualizaCabecalho(0, 1);
        fclose(arv);

    }else{
        // salvando alterações nas páginas filhas
        fseek(arv, posicaoPag, SEEK_SET);
        fwrite(&pag, sizeof(pag), 1, arv);

        fseek(arv, enderecoNovaPag, SEEK_SET);
        fwrite(&novaPag, sizeof(pag), 1, arv);

        // buscando pai
        fseek(arv, pag.pai, SEEK_SET);
        fread(&pai, sizeof(Pagina), 1, arv);

        // inserindo elemento Promovido
        posicaoInsercaoPai = insereOrdenado(&pai, idPromovido, enderecoNovaPag);

        // ajustando ponteiros
        pai.enderecos[posicaoInsercaoPai-1] = posicaoPag;

        // salvando página pai e atualizando cabeçalho
        fseek(arv, pag.pai, SEEK_SET);
        fwrite(&pai, sizeof(Pagina), 1, arv);

        fclose(arv);

        if(pai.m > ORDEM * 2)
            cisao(pag.pai, 1);

    }
}

void inserir(Registro insere){
    int estado = 0, posicaoPag = 0, posicaoPercorre = 0, posicaoPagEndereco = 0, posicaoEndereco = 0;
    Pagina percorre;
    FILE *fp = fopen(ARQARVORE, "r+b");


    posicaoPercorre = buscaB(insere.id, &estado, &posicaoPag, &posicaoPagEndereco, &posicaoEndereco);

    if(!posicaoPercorre){
        posicaoPercorre = sizeof(Cabecalho);
        criaPagina();
        atualizaCabecalho(posicaoPercorre, 1);
    }


    fseek(fp, posicaoPercorre, SEEK_SET);
    fread(&percorre, sizeof(Pagina), 1, fp);

    if(estado) // elemento já se encontra na árvore
        printf("Valor informado já se encontra na árvore!\n");
    else{
        // inserindo o valor
        FILE *registros = fopen(TABELAREGISTROS, "ab");
        int enderecoReg = ftell(registros);

        fwrite(&insere, sizeof(Registro), 1, registros);
        fclose(registros);

        insereOrdenado(&percorre, insere.id, enderecoReg);

        fseek(fp, posicaoPercorre, SEEK_SET);
        fwrite(&percorre, sizeof(Pagina), 1, fp);


        fclose(fp);
        fp = fopen(ARQARVORE, "r+b");

        if(percorre.m > ORDEM * 2)
            cisao(posicaoPercorre, 0);

        printf("Valor inserido com sucesso!\n");
    }


    fclose(fp);

}

void imprimePagina(Pagina pagina){
    FILE *tabelaR = fopen(TABELAREGISTROS, "rb");
    Registro rg;


    printf("/ ");
    for(int i = 1; i <= pagina.m; i++){
        if(pagina.chaves[i] != -1){
            printf("%i ", pagina.chaves[i]);
        }
    }
    printf("/\n");

    fclose(tabelaR);
}

void imprimeAltura(int altura){
    for(int i = 0; i < altura; i++)
        printf("\t\t");
}

void imprimePaginas(FILE *arv, int posicaoRaiz, int posicao, int altura){
    Pagina raiz;
    fseek(arv, posicaoRaiz, SEEK_SET);
    fread(&raiz, sizeof(Pagina), 1, arv);

    if(posicao <= raiz.m){

        if(raiz.enderecos[posicao] != -1 && !raiz.folha) // caso tenha filho
            imprimePaginas(arv, raiz.enderecos[posicao], 0, altura+1);

        if(posicao == 0 && raiz.ativa){
            imprimeAltura(altura);
            imprimePagina(raiz);
        }

        imprimePaginas(arv, posicaoRaiz, posicao+1, altura);


    }

}

void imprimeArvore(int posicaoRaiz){
    FILE *arv = fopen(ARQARVORE, "rb");

    if(posicaoRaiz != -1)
        imprimePaginas(arv, posicaoRaiz, 0, 0);
    else
        printf("A árvore está vazia.\n");

    fclose(arv);
}


void imprimeOrdenado(int id){
    int estado = 0, posicaoPag = 0, pag = 0, posicaoPagEndereco = 0, posicaoEndereco = 0;
    Pagina pg;
    FILE *arv = fopen(ARQARVORE, "r+b");

    pag = buscaB(id, &estado, &posicaoPag, &posicaoPagEndereco, &posicaoEndereco);

    if(estado){
        do{
            fseek(arv, pag, SEEK_SET);
            fread(&pg, sizeof(Pagina), 1, arv);

            for(int i = posicaoPag; i <= pg.m; i++){
                if(pg.chaves[i] != -1){
                    printf("%i ", pg.chaves[i]);
                }
            }

            pag = pg.proxima;
            posicaoPag = 1;

        }while(pag);
        printf("\n");
    }else
        printf("O ID informado não se encontra na árvore.\n");

    fclose(arv);
}

int buscaSucessor(Pagina pag, int posicaoInicial, int *temSucessor){
    if(pag.chaves[posicaoInicial + 1] != -1)
        return pag.chaves[posicaoInicial + 1];
    else{
        if(pag.proxima){
            FILE *arv = fopen(ARQARVORE, "rb");
            fseek(arv, pag.proxima, SEEK_SET);
            fread(&pag, sizeof(Pagina), 1, arv);
            fclose(arv);
            return pag.chaves[1];
        }

        *temSucessor = 0;
        return 0;
    }
}

int buscaPosicaoPai(Pagina pag){
    FILE *fp = fopen(ARQARVORE, "r+b");
    Pagina pai;
    int indice = 0;

    // buscando a página pai
    fseek(fp, pag.pai, SEEK_SET);
    fread(&pai, sizeof(Pagina), 1, fp);

    while(indice < pai.m && pai.chaves[indice + 1] <= pag.chaves[1])
        indice++;

    fclose(fp);

    return indice;
}

int buscaIrmaAdjacente(Pagina pag, int *indice){
    Pagina irmaDir, irmaEsq, pai;
    int enderecoIrmaDir = 0, enderecoIrmaEsq = 0;
    FILE *fp = fopen(ARQARVORE, "r+b");

    fseek(fp, pag.pai, SEEK_SET);
    fread(&pai, sizeof(Pagina), 1, fp);


    *indice = buscaPosicaoPai(pag);
    //printf("a posição do pai é %i\n", *indice);
    if(*indice){
        fseek(fp, pai.enderecos[*indice - 1], SEEK_SET);
        fread(&irmaEsq, sizeof(Pagina), 1, fp);
        enderecoIrmaEsq = pai.enderecos[*indice - 1];
    }
    if(*indice < pai.m){
        fseek(fp, pai.enderecos[*indice + 1], SEEK_SET);
        fread(&irmaDir, sizeof(Pagina), 1, fp);
        enderecoIrmaDir = pai.enderecos[*indice + 1];
    }

    if(enderecoIrmaDir && enderecoIrmaEsq)
        return irmaDir.m > irmaEsq.m ? enderecoIrmaDir : enderecoIrmaEsq;

    return enderecoIrmaEsq ? enderecoIrmaEsq : enderecoIrmaDir;

    fclose(fp);
}

void redistribuicao(int enderecoPag, int enderecoIrma, int indicePai){
    FILE *fp = fopen(ARQARVORE, "r+b");
    Pagina pai, irma, maior, pag, aux;
    int indice = 0;

    // buscando pag
    fseek(fp, enderecoPag, SEEK_SET);
    fread(&pag, sizeof(Pagina), 1, fp);

    // buscando irma
    fseek(fp, enderecoIrma, SEEK_SET);
    fread(&irma, sizeof(Pagina), 1, fp);

    // buscando pai
    fseek(fp, pag.pai, SEEK_SET);
    fread(&pai, sizeof(Pagina), 1, fp);


    if(pai.enderecos[indicePai + 1] == enderecoIrma){ // filha a esq
        indicePai++;

        if(!irma.folha && irma.enderecos[indice] != -1){
            fseek(fp, irma.enderecos[indice], SEEK_SET);
            fread(&aux, sizeof(Pagina), 1, fp);

            aux.pai = enderecoPag;

            fseek(fp, irma.enderecos[indice], SEEK_SET);
            fwrite(&aux, sizeof(Pagina), 1, fp);
        }

        indice = 1;

        pai.chaves[indicePai] = irma.chaves[indice + 1];
        irma.enderecos[0] = irma.enderecos[indice];
    }else{ // filha a dir
        indice = irma.m;

        if(!irma.folha && irma.enderecos[indice] != -1){
            fseek(fp, irma.enderecos[indice], SEEK_SET);
            fread(&aux, sizeof(Pagina), 1, fp);

            aux.pai = enderecoPag;

            fseek(fp, irma.enderecos[indice], SEEK_SET);
            fwrite(&aux, sizeof(Pagina), 1, fp);
        }

        pai.chaves[indicePai] = irma.chaves[indice];
    }

    // inserindo nova chave
    insereOrdenado(&pag, irma.chaves[indice], irma.enderecos[indice]);

    atualizaPosicao(&irma, indice); // atualizando posição da irmã


    // salvando os valores
    fseek(fp, enderecoPag, SEEK_SET);
    fwrite(&pag, sizeof(Pagina), 1, fp);

    fseek(fp, enderecoIrma, SEEK_SET);
    fwrite(&irma, sizeof(Pagina), 1, fp);

    fseek(fp, pag.pai, SEEK_SET);
    fwrite(&pai, sizeof(Pagina), 1, fp);

    fclose(fp);
}

void concatenacao(int enderecoPag, int enderecoIrma, int indicePai, int enderecoMaior){
    FILE *fp = fopen(ARQARVORE, "r+b");
    Pagina pai, irma, aux, pag;
    int chavePromovido = 0, enderecoPromovido = 0;

    // buscando pag
    fseek(fp, enderecoPag, SEEK_SET);
    fread(&pag, sizeof(Pagina), 1, fp);

    // buscando o pai
    fseek(fp, pag.pai, SEEK_SET);
    fread(&pai, sizeof(Pagina), 1, fp);

    // buscando irma
    fseek(fp, enderecoIrma, SEEK_SET);
    fread(&irma, sizeof(Pagina), 1, fp);

    pai.enderecos[indicePai] = enderecoIrma;

    if(pai.enderecos[indicePai + 1] == enderecoIrma){ // filho a esq
        indicePai++;

        chavePromovido = pai.chaves[indicePai];

        irma.enderecos[0] = pag.enderecos[0];

    }else{ // filho a dir
        chavePromovido = pai.chaves[indicePai];

        irma.enderecos[0] = enderecoMaior;

        irma.proxima = pag.proxima;
    }

    enderecoPromovido = pag.enderecos[0];

    // ajustando os ponteiros para os pais
    if(enderecoMaior != -1){
        fseek(fp, enderecoMaior, SEEK_SET);
        fread(&aux, sizeof(Pagina), 1, fp);

        aux.pai = enderecoIrma;

        fseek(fp, enderecoMaior, SEEK_SET);
        fwrite(&aux, sizeof(Pagina), 1, fp);
    }if(!pag.folha && pag.enderecos[0] != -1){
        fseek(fp, pag.enderecos[0], SEEK_SET);
        fread(&aux, sizeof(Pagina), 1, fp);

        aux.pai = enderecoIrma;

        fseek(fp, pag.enderecos[0], SEEK_SET);
        fwrite(&aux, sizeof(Pagina), 1, fp);
    }if(!pag.folha && pag.enderecos[1] != -1){
        fseek(fp, pag.enderecos[1], SEEK_SET);
        fread(&aux, sizeof(Pagina), 1, fp);

        aux.pai = enderecoIrma;

        fseek(fp, pag.enderecos[1], SEEK_SET);
        fwrite(&aux, sizeof(Pagina), 1, fp);
    }

    insereOrdenado(&irma, pag.chaves[1], pag.enderecos[1]);
    if(!irma.folha)
        insereOrdenado(&irma, chavePromovido, enderecoPromovido);

    atualizaPosicao(&pai, indicePai);

    pag.ativa = 0;

    // salvando dados
    fseek(fp, enderecoPag, SEEK_SET);
    fwrite(&pag, sizeof(Pagina), 1, fp);

    fseek(fp, enderecoIrma, SEEK_SET);
    fwrite(&irma, sizeof(Pagina), 1, fp);

    fseek(fp, pag.pai, SEEK_SET);
    fwrite(&pai, sizeof(Pagina), 1, fp);

    // verificando se há violação após concatenação
    if(!pai.pai && pai.m == 0){
        Cabecalho cb;
        fseek(fp, 0, SEEK_SET);
        fread(&cb, sizeof(Cabecalho), 1, fp);

        cb.raiz = enderecoIrma;
        cb.paginasExcluidas += 2;
        cb.quantidadePaginas++;

        irma.pai = 0;
        pai.ativa = 0;


        fseek(fp, 0, SEEK_SET);
        fwrite(&cb, sizeof(Cabecalho), 1, fp);

        fseek(fp, enderecoIrma, SEEK_SET);
        fwrite(&irma, sizeof(Pagina), 1, fp);

        fseek(fp, pag.pai, SEEK_SET);
        fwrite(&pai, sizeof(Pagina), 1, fp);

    }else if(pai.pai && pai.m < ORDEM){ // caso a página não seja raiz mas esteja violando alguma propriedade
        int indice = 0, enderecoTia = 0;
        Pagina tia;

        enderecoTia = buscaIrmaAdjacente(pai, &indice);

        fseek(fp, enderecoTia, SEEK_SET);
        fread(&tia, sizeof(Pagina), 1, fp);

        if(tia.m < ORDEM + 1) // faz concatenação
            concatenacao(irma.pai, enderecoTia, indice, tia.enderecos[0]);
        else
            redistribuicao(irma.pai, enderecoTia, indice);
    }

    fclose(fp);
}

void remover(int remove){
    int estado = 0, posicaoPag = 0, posicaoPercorre = 0, posicaoPagEndereco = 0, pagEndereco = 0, indicePai = 0, enderecoIrma = 0;

    Pagina percorre, aux, irma;
    FILE *fp = fopen(ARQARVORE, "r+b");


    posicaoPercorre = buscaB(remove, &estado, &posicaoPag, &pagEndereco, &posicaoPagEndereco);

    if(estado){
        fseek(fp, posicaoPercorre, SEEK_SET);
        fread(&percorre, sizeof(Pagina), 1, fp);


        if(pagEndereco){
            int temSucessor = 1, sucessor;
            sucessor = buscaSucessor(percorre, posicaoPag, &temSucessor);

            if(temSucessor){ // alterando pelo sucessor
                fseek(fp, pagEndereco, SEEK_SET);
                fread(&aux, sizeof(Pagina), 1, fp);
                aux.chaves[posicaoPagEndereco] = sucessor;
                fseek(fp, pagEndereco, SEEK_SET);
                fwrite(&aux, sizeof(Pagina), 1, fp);
            }
        }
        // removendo o nó
        atualizaPosicao(&percorre, posicaoPag);

        fseek(fp, posicaoPercorre, SEEK_SET);
        fwrite(&percorre, sizeof(Pagina), 1, fp);

        if(percorre.m < ORDEM && percorre.pai){
            enderecoIrma = buscaIrmaAdjacente(percorre, &indicePai);

            fseek(fp, enderecoIrma, SEEK_SET);
            fread(&irma, sizeof(Pagina), 1, fp);

            if(irma.m < ORDEM + 1)
                concatenacao(posicaoPercorre, enderecoIrma, indicePai, -1);
            else
                redistribuicao(posicaoPercorre, enderecoIrma, indicePai);

        }


    }else
        printf("O valor não se registro na árvore!");


    fclose(fp);
}

