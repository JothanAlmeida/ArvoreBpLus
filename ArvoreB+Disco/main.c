#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "pagina.h"




void leValores(Registro *rg){
    printf("Informe o id:");
    scanf("%i", &rg->id);
    printf("Informe o ano:");
    scanf("%i", &rg->ano);
    printf("Informe o autor: ");
    scanf("%s", rg->autor);
    printf("Informe o DOI: ");
    scanf("%s", rg->DOI);
    printf("Informe o palavraChave: ");
    scanf("%s", rg->palavraChave);
    printf("Informe o revista: ");
    scanf("%s", rg->revista);
    printf("Informe o titulo: ");
    scanf("%s", rg->titulo);
}

void imprimeRegistro(Registro rg){
    printf("ID : %i\n", rg.id);
    printf("Autor: %s\n", rg.autor);
    printf("Titulo: %s\n", rg.titulo);
    printf("Ano: %i\n", rg.ano);
    printf("DOI: %s\n", rg.DOI);
    printf("Revista: %s\n", rg.revista);
    printf("Palvra chave: %s\n", rg.palavraChave);
}

void limparTerminal(){
    char c;

    printf("Pressione ENTER para continuar...");
    setbuf(stdin, NULL);
    scanf("%c", &c);
    system("clear");
}


void menu(){
    int opc = 0, id = 0, estado = 0, pag = 0, posicaoPag = 0, posicaoPagEndereco = 0, posicaoEndereco = 0;
    FILE *arv, *registros;
    Registro rg;
    Pagina pg;

    do{
        printf("1 - Inserir registro\n2 - Imprimir árvore\n3 - Buscar por id\n4 - Imprimir ordenado partindo de um registro\n5 - Remover Registro\n6 - Sair\n");
        scanf("%i", &opc);

        switch (opc){
        case 1:
            leValores(&rg);
            inserir(rg);
            break;
        case 2:
            imprimeArvore(buscaRaiz());
            break;
        case 3:
            printf("Informe o id desejado: ");
            scanf("%i", &id);

            pag = buscaB(id, &estado, &posicaoPag, &posicaoPagEndereco, &posicaoEndereco);

            if(estado){
                arv = fopen(ARQARVORE, "rb");
                registros = fopen(TABELAREGISTROS, "rb");

                fseek(arv, pag, SEEK_SET);
                fread(&pg, sizeof(Pagina), 1, arv);

                fseek(registros, pg.enderecos[posicaoPag], SEEK_SET);
                fread(&rg, sizeof(Registro), 1, registros);

                imprimeRegistro(rg);

                fclose(registros);
                fclose(arv);
            }else
                printf("O registro não se encontra na árvore.\n");

            break;
        case 4:
            printf("Informe o id desejado: ");
            scanf("%i", &id);

            imprimeOrdenado(id);
            break;
        case 5:
            printf("Informe o id do registro: ");
            scanf("%i", &id);

            remover(id);

            break;
        case 6:
            printf("Saindo...\n");
            break;
        default:
            printf("Opção inválida!\n");
            break;
        }

        limparTerminal();
    }while(opc != 6);
}


int main(){

    criaArquivo();

    menu();
    return 0;
}