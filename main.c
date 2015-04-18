//
//  main.c
//  MC504_t1
//
//  Created by Letícia Gonçalves on 4/13/15.
//  Copyright (c) 2015 LazyFox e Cadu. All rights reserved.
//

//TODO ---->>>>Botar mutex nos bits da bitmask, fazer batman, controle dos sinais
//TODO ---->>>>Mutex para fim da fila

//----------------------------------------------------------------------
//                      Bibliotecas
//----------------------------------------------------------------------
#include <stdio.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdlib.h>
//----------------------------------------------------------------------
//                      Defines
//----------------------------------------------------------------------
#define NORTH 1 << 0
#define SOUTH 1 << 1
#define EAST 1 << 2
#define WEST 1 << 3
//----------------------------------------------------------------------
//                Declaração e inicialização de variáveis
//----------------------------------------------------------------------

//Variaveis de thread
pthread_mutex_t cruzamento = PTHREAD_MUTEX_INITIALIZER;
pthread_t batman;

uint8_t busy_mask = 0;
pthread_mutex_t mask_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t testecond = PTHREAD_COND_INITIALIZER;

//Estruturas de dados
typedef struct noFila {
    pthread_t bat;
	pthread_cond_t condition;
    struct noFila *prox;
    uint8_t id;
} Fila;

typedef struct cabecaFila {
    Fila *primeiroNo;
    Fila *ultimoNo;
    int tamanho;
    pthread_mutex_t alteraFila;
	pthread_mutex_t fim_da_fila;
}filaCabeca;

filaCabeca NorteFila, LesteFila, SulFila, OesteFila;
//----------------------------------------------------------------------
//                      Declaracao de funcoes
//----------------------------------------------------------------------
void insereFila(filaCabeca *fila, uint8_t ultimoId);
void *northBat(void * tid);
void *southBat(void * tid);
void *westBat(void * tid);
void *eastBat(void * tid);
//-----------------------------------------------------------------------
//                          Main
//-----------------------------------------------------------------------


int main(int argc, const char * argv[]) {

    //INICIALIZA
    if(argc<=1){
        exit(EXIT_FAILURE);
    }
    char sentido;
	
	NorteFila.ultimoNo = SulFila.ultimoNo = LesteFila.ultimoNo = OesteFila.ultimoNo = NULL;
	NorteFila.primeiroNo = SulFila.primeiroNo = LesteFila.primeiroNo = OesteFila.primeiroNo = NULL;
	NorteFila.tamanho = SulFila.tamanho = LesteFila.tamanho = OesteFila.tamanho = 0;
	
	pthread_mutex_init(&NorteFila.fim_da_fila, NULL);
	pthread_mutex_init(&SulFila.fim_da_fila, NULL);
	pthread_mutex_init(&LesteFila.fim_da_fila, NULL);
	pthread_mutex_init(&OesteFila.fim_da_fila, NULL);
	
	pthread_mutex_init(&NorteFila.alteraFila, NULL);
	pthread_mutex_init(&SulFila.alteraFila, NULL);
	pthread_mutex_init(&LesteFila.alteraFila, NULL);
	pthread_mutex_init(&OesteFila.alteraFila, NULL);
    //RESOLVE PROBLEMA
    uint8_t i = 0;
    while ((sentido=argv[1][i])!='\0') {
		i++;
        switch (sentido) {
			//Depois de inserir inicializar o COND do bat
            case 'n': {
                insereFila(&NorteFila, i);
				pthread_cond_init(&(NorteFila.ultimoNo->condition), NULL);
				pthread_create(&(NorteFila.ultimoNo->bat),NULL,northBat,(void *) NorteFila.ultimoNo);
				busy_mask |= NORTH; /////Condition to wake -> escreve mas nao le
				pthread_mutex_unlock(&NorteFila.fim_da_fila);
				pthread_mutex_unlock(&mask_mutex);
                break;
			}
			case 'e': {
				insereFila(&LesteFila, i);
				pthread_cond_init(&(NorteFila.ultimoNo->condition), NULL);
				pthread_create(&(LesteFila.ultimoNo->bat),NULL,eastBat,(void *) LesteFila.ultimoNo);
				busy_mask |= EAST;
				pthread_mutex_unlock(&LesteFila.fim_da_fila);
				pthread_mutex_unlock(&mask_mutex);
				break;
			}
			case 's': {
				insereFila(&SulFila, i);
				pthread_cond_init(&(NorteFila.ultimoNo->condition), NULL);
				pthread_create(&(SulFila.ultimoNo->bat),NULL,southBat,(void *) SulFila.ultimoNo);
				busy_mask |= SOUTH;
				pthread_mutex_unlock(&SulFila.fim_da_fila);
				pthread_mutex_unlock(&mask_mutex);
				break;
			}
			case 'w': {
				insereFila(&OesteFila, i);
				pthread_cond_init(&(NorteFila.ultimoNo->condition), NULL);
				pthread_create(&(OesteFila.ultimoNo->bat),NULL,westBat,(void *) OesteFila.ultimoNo);
				busy_mask |= WEST;
				pthread_mutex_unlock(&OesteFila.fim_da_fila);
				pthread_mutex_unlock(&mask_mutex);
				break;
			}
			
            default: {
				exit(EXIT_FAILURE);
                break;
			}
        }
    }
    //FINALIZA
	printf("N%dE%dW%dS%d\n",NorteFila.tamanho, LesteFila.tamanho, OesteFila.tamanho, SulFila.tamanho);
	setbuf(stdin,0);
	getchar();
    return 0;
}
//RODA

//-----------------------------------------------------------------------
//                      Threads
//-----------------------------------------------------------------------
void *northBat(void * tid)
{
	Fila * thisBat = (Fila *) tid; //Guarda referencia desse bat, com todas informacoes uteis

	pthread_mutex_lock(&NorteFila.alteraFila); //Mutex para acesso a fila do norte, verifica posicao
	
	while (NorteFila.primeiroNo != thisBat) //Se nao for o primeiro noh espera ser chamado ->while garante veracidade do caso
	{ 
		pthread_cond_wait(&(thisBat->condition), &NorteFila.alteraFila); //Libera mutex e espera sinal
	}
	
	printf("BAT %d N chegou no cruzamento\n", thisBat->id); //Informa que o BAT chegou no cruzamento
	
	pthread_mutex_lock(&cruzamento); //Entra na regiao critica do cruzamento para testar variaveis
	
	//if (LIMITE);//Condicao para chamar BATMAN ->>>Implementar!
	
	printf("BAT %d N saiu no cruzamento\n", thisBat->id); 
	pthread_mutex_lock(&NorteFila.fim_da_fila);
	NorteFila.tamanho--;
	NorteFila.primeiroNo = thisBat->prox;
	if (thisBat->prox != NULL)
	{
		pthread_cond_broadcast(&thisBat->prox->condition);
	}
	else//caso onde esse eh o ultimo no da fila
	{
		//lembrar de chamar proxima fila
		pthread_mutex_lock(&mask_mutex);
		busy_mask &= ~NORTH;
		pthread_mutex_unlock(&mask_mutex);
	}
		
	pthread_mutex_unlock(&NorteFila.fim_da_fila);
	
	
	pthread_mutex_unlock(&cruzamento);	
	pthread_mutex_unlock(&NorteFila.alteraFila);//Terminou de mexer na fila
	
	return NULL;
}
void *southBat(void * tid)
{
	Fila * thisBat = (Fila *) tid;
	
	pthread_mutex_lock(&SulFila.alteraFila); //Mutex para acesso a fila do norte, verifica posicao
	
	while (SulFila.primeiroNo != thisBat) //Se nao for o primeiro noh espera ser chamado ->while garante veracidade do caso
	{ 
		pthread_cond_wait(&(thisBat->condition), &SulFila.alteraFila); //Libera mutex e espera sinal
	}
	
	printf("BAT %d S chegou no cruzamento\n", thisBat->id); //Informa que o BAT chegou no cruzamento
	
	//Aqui eu testo se ele deve prosseguir cruzamento, checar bit mask
	check: //gambi para testes
	pthread_mutex_lock(&mask_mutex);
	if (busy_mask & ~(SOUTH | WEST))
	{
		pthread_mutex_unlock(&mask_mutex);
		goto check;
	}
	pthread_mutex_unlock(&mask_mutex);
	
	
	pthread_mutex_lock(&cruzamento); //Entra na regiao critica do cruzamento para testar variaveis
	
	//if (LIMITE);//Condicao para chamar BATMAN ->>>Implementar!
	
	printf("BAT %d S saiu no cruzamento\n", thisBat->id); 
	pthread_mutex_lock(&SulFila.fim_da_fila);
	SulFila.tamanho--;
	SulFila.primeiroNo = thisBat->prox;
	if (thisBat->prox != NULL)
	{
		pthread_cond_broadcast(&thisBat->prox->condition);
	}
	else//caso onde esse eh o ultimo no da fila
	{
		pthread_mutex_lock(&mask_mutex);
		busy_mask &= ~SOUTH;
		pthread_mutex_unlock(&mask_mutex);
	}
		
	pthread_mutex_unlock(&SulFila.fim_da_fila);
	
	
	pthread_mutex_unlock(&cruzamento);	
	pthread_mutex_unlock(&SulFila.alteraFila);//Terminou de mexer na fila

	return NULL;
}
void *westBat(void * tid)
{
	Fila * thisBat = (Fila *) tid; //Pega referencia do noh desse BAT
	

	return NULL;
}
void *eastBat(void * tid)
{
	Fila * thisBat = (Fila *) tid;
	

	return NULL;
}
//-----------------------------------------------------------------------
//                      Auxiliares
//-----------------------------------------------------------------------

void insereFila(filaCabeca *fila, uint8_t ultimoId) {
    Fila *novo;
	novo = (Fila *)malloc(sizeof(Fila));
    novo->prox=NULL;
    novo->id=ultimoId;
	pthread_mutex_lock(&mask_mutex);
	pthread_mutex_lock(&fila->fim_da_fila);
	fila->tamanho++;
	if (fila->ultimoNo != NULL)
	{
	    (fila->ultimoNo)->prox=novo;
	    (fila->ultimoNo)=novo;
	}
	else 
	{
		(fila->ultimoNo)=novo;
		(fila->primeiroNo)=novo;
	}
}


