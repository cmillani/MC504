//
//  main.c
//  MC504_t1
//  
//  Created by
//  Carlos Eduardo Millani RA: 145643
//  Letícia Gonçalves RA: 146906
//  on 4/13/15.
//
//

/***********************************************************************
                          Resumo do programa
************************************************************************
O programa itera pela string com os caracteres 'n', 'e', 'w', e 's' passados
como argumento, cria e inicializa uma variavel do tipo noFila, que contem
todas informacoes do bat, e inicializa uma thread para cada letra. Existe uma 
variavel do tipo cabecaFila para cada fila de threads, para facilitar insercao 
e controle.

Cada thread ao iniciar verifica se ela e a primeira da fila, caso positivo ela
entra no cruzamento (imprime na tela), altera as bitmasks que o batman le para
mandar sinais para os bat seguirem e espera o sinal do bat, se negativo, o bat
espera um sinal do primeiro bat (que sera enviado quando esse for se tornar
o primeiro da fila).

Ao receber o sinal do batman o bat imprime que esta saindo da fila e atualiza
as variaveis da fila.

O loop do batman consiste em travar o cruzamento, para garantir a consistencia
dos dados que ele vai analizar, ler as bitmasks para saber quais sentidos tem 
bat no cruzamento e quais estao dispostos a ceder a vez. Baseado nessas duas
mascaras ele envia sinal para o escolhido e imprime uma mensagem caso algum
bat tenha cedido a vez ou caso tenha ocorrido um impasse.

Foi entendido como impasse a presenca de mais de um bat que nao quer ceder a 
vez no cruzamento. O criterio para ceder a vez e a fila ultrapassar um tamanho
MAX_SIZE definido no codigo.

***********************************************************************/

//----------------------------------------------------------------------
//                      Bibliotecas
//----------------------------------------------------------------------
#include <stdio.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
//----------------------------------------------------------------------
//                      Defines
//----------------------------------------------------------------------
//Relativos aos acessos as bitmasks
#define NORTH 1 << 0
#define SOUTH 1 << 1
#define EAST 1 << 2
#define WEST 1 << 3
//Tamanho maximo que determina quando passa a vez
#define MAX_SIZE 2
//----------------------------------------------------------------------
//                Declaração e inicialização de variáveis
//----------------------------------------------------------------------
//Controle de impasses
uint8_t want_mask = 0;
uint8_t give_mask = 0;


//Variaveis do batman
pthread_mutex_t batmanMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t batmanCond = PTHREAD_COND_INITIALIZER;
uint32_t newData = 0;
uint32_t threadData = 0;

char calee = 0;

//Variaveis de thread
pthread_mutex_t cruzamento = PTHREAD_MUTEX_INITIALIZER;
pthread_t batman;

pthread_cond_t testecond = PTHREAD_COND_INITIALIZER;

//Estruturas de dados
typedef struct noFila {
    pthread_t bat;
	pthread_cond_t condition;
    struct noFila *prox;
    uint8_t id;
} Fila;

typedef struct whatToFree {
	Fila *no;
	struct whatToFree *prox;
} whatToFree;

typedef struct cabecaFila {
	bool shouldContinue;
    Fila *primeiroNo;
    Fila *ultimoNo;
    int tamanho;
    pthread_mutex_t alteraFila;
	pthread_mutex_t fim_da_fila;
}filaCabeca;

filaCabeca NorteFila, LesteFila, SulFila, OesteFila;
whatToFree nodesToFree, *nodesToFreeEnd;
//----------------------------------------------------------------------
//                      Declaracao de funcoes
//----------------------------------------------------------------------
void insereFila(filaCabeca *fila, uint8_t ultimoId);
void *batMan(void * tid);
void *northBat(void * tid);
void *southBat(void * tid);
void *westBat(void * tid);
void *eastBat(void * tid);
//-----------------------------------------------------------------------
//                          Main
//-----------------------------------------------------------------------


int main(int argc, const char * argv[]) {

    //Inicializacao das variaveis
    if(argc<=1){
        exit(EXIT_FAILURE);
    }
    char sentido;
	pthread_t *lastE, *lastW, *lastN, *lastS;
	lastE = NULL;
	lastW = NULL;
	lastN = NULL;
	lastS = NULL;
	nodesToFree.prox = NULL;
	nodesToFreeEnd = &nodesToFree;
	
	NorteFila.ultimoNo = SulFila.ultimoNo = LesteFila.ultimoNo = OesteFila.ultimoNo = NULL;
	NorteFila.primeiroNo = SulFila.primeiroNo = LesteFila.primeiroNo = OesteFila.primeiroNo = NULL;
	NorteFila.tamanho = SulFila.tamanho = LesteFila.tamanho = OesteFila.tamanho = 0;
	NorteFila.shouldContinue = SulFila.shouldContinue = LesteFila.shouldContinue = OesteFila.shouldContinue = true;
	
	pthread_mutex_init(&NorteFila.fim_da_fila, NULL);
	pthread_mutex_init(&SulFila.fim_da_fila, NULL);
	pthread_mutex_init(&LesteFila.fim_da_fila, NULL);
	pthread_mutex_init(&OesteFila.fim_da_fila, NULL);
	
	pthread_mutex_init(&NorteFila.alteraFila, NULL);
	pthread_mutex_init(&SulFila.alteraFila, NULL);
	pthread_mutex_init(&LesteFila.alteraFila, NULL);
	pthread_mutex_init(&OesteFila.alteraFila, NULL);
	
	//Inicia o batman
	pthread_create(&batman, NULL, batMan, 0);
	
    //Itera pelo vetor passado como argumento para o programa e cria as threads para cada BAT
    uint8_t i = 0;
    while ((sentido=argv[1][i])!='\0') {
		i++;
        switch (sentido) {
            case 'n': {
                insereFila(&NorteFila, i);
				pthread_cond_init(&(NorteFila.ultimoNo->condition), NULL);
				lastN = &NorteFila.ultimoNo->bat;
				pthread_create(&(NorteFila.ultimoNo->bat),NULL,northBat,(void *) NorteFila.ultimoNo);
				pthread_mutex_unlock(&NorteFila.fim_da_fila);
                break;
			}
			case 'e': {
				insereFila(&LesteFila, i);
				pthread_cond_init(&(LesteFila.ultimoNo->condition), NULL);
				lastE = &LesteFila.ultimoNo->bat;
				pthread_create(&(LesteFila.ultimoNo->bat),NULL,eastBat,(void *) LesteFila.ultimoNo);
				pthread_mutex_unlock(&LesteFila.fim_da_fila);
				break;
			}
			case 's': {
				insereFila(&SulFila, i);
				pthread_cond_init(&(SulFila.ultimoNo->condition), NULL);
				lastS = &SulFila.ultimoNo->bat;
				pthread_create(&(SulFila.ultimoNo->bat),NULL,southBat,(void *) SulFila.ultimoNo);
				pthread_mutex_unlock(&SulFila.fim_da_fila);
				break;
			}
			case 'w': {
				insereFila(&OesteFila, i);
				pthread_cond_init(&(OesteFila.ultimoNo->condition), NULL);
				lastW = &OesteFila.ultimoNo->bat;
				pthread_create(&(OesteFila.ultimoNo->bat),NULL,westBat,(void *) OesteFila.ultimoNo);
				pthread_mutex_unlock(&OesteFila.fim_da_fila);
				break;
			}
			
            default: { //Caractar nao representa nenhum sentido, erro de entrada
				exit(EXIT_FAILURE);
                break;
			}
        }
    }
    //Espera as ultimas threads criadas retornarem e 
	int *retval;
	if (lastN != NULL)	pthread_join(*lastN, (void*)&retval);
	if (lastE != NULL)	pthread_join(*lastE, (void*)&retval);
	if (lastS != NULL)	pthread_join(*lastS, (void*)&retval);
	if (lastW != NULL)	pthread_join(*lastW, (void*)&retval);
	whatToFree *atual;
	whatToFree *temp;
	atual = nodesToFree.prox;
	while (atual != NULL)
	{
		if (atual->no != NULL) 
		{
			free(atual->no);
		}
		temp = atual;
		atual = atual->prox;
		free (temp);
	}
	//Fim do programa
    return 0;
}

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
	
	pthread_mutex_lock(&cruzamento); //Entra na regiao critica do cruzamento para testar variaveis
	printf("BAT %d N chegou no cruzamento\n", thisBat->id); //Informa que o BAT chegou no cruzamento
	
	if (NorteFila.tamanho > MAX_SIZE)//Ultrapassa limite, cede a vez
	{
		give_mask |= NORTH; //Mascara que indica bats que podem ceder a vez
	}
	want_mask |= NORTH; //Todo bat sempre quer passar
	newData++; //Contador para controle no batman
	threadData++; //Contador para controle interno
	pthread_cond_broadcast(&batmanCond); //Acorda batman, caso ele esteja dormindo
	pthread_cond_wait(&thisBat->condition, &cruzamento); //Espera sinal do batman para sair
	
	printf("BAT %d N saiu no cruzamento\n", thisBat->id);
	threadData--; //Atualiza contador de controle interno
	want_mask &= ~(NORTH);//Atualiza masks
	give_mask &= ~(NORTH);
	pthread_cond_broadcast(&batmanCond); //Acorda o batman para escolher o prox bat
	pthread_mutex_unlock(&cruzamento);//Ja saiu do cruzamento
	
	pthread_mutex_lock(&NorteFila.fim_da_fila); //Altera o estado da fila, utilizado para nao inserir enquanto remove
	NorteFila.tamanho--;
	NorteFila.primeiroNo = thisBat->prox;
	if (thisBat->prox != NULL) //Nao eh o fim da fila
	{
		pthread_cond_broadcast(&thisBat->prox->condition);
	}
	else //Eh o fim da fila
	{
		NorteFila.ultimoNo = NULL;
	}
	pthread_mutex_unlock(&NorteFila.fim_da_fila);
	
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
	pthread_cond_broadcast(&batmanCond);
	pthread_mutex_lock(&cruzamento); //Entra na regiao critica do cruzamento para testar variaveis
	printf("BAT %d S chegou no cruzamento\n", thisBat->id); //Informa que o BAT chegou no cruzamento
	
	if (SulFila.tamanho > MAX_SIZE)//Ultrapassa limite, cede a vez
	{
		give_mask |= SOUTH; //Mascara que indica bats que podem ceder a vez
	}
	want_mask |= SOUTH; //Ele quer de qq jeito, so perciso do ir pra caso ele possa passar a vez
	newData++;
	threadData++;
	pthread_cond_broadcast(&batmanCond);
	pthread_cond_wait(&thisBat->condition, &cruzamento);
	
	printf("BAT %d S saiu no cruzamento\n", thisBat->id);
	threadData--;
	want_mask &= ~(SOUTH);
	give_mask &= ~(SOUTH);
	pthread_cond_broadcast(&batmanCond);
	pthread_mutex_unlock(&cruzamento);//Ja saiu do cruzamento
	
	pthread_mutex_lock(&SulFila.fim_da_fila); //Altera o estado da fila, utilizado para nao inserir enquanto remove
	SulFila.tamanho--;
	SulFila.primeiroNo = thisBat->prox;
	if (thisBat->prox != NULL)
	{
		pthread_cond_broadcast(&thisBat->prox->condition);
	}
	else
	{
		SulFila.ultimoNo = NULL;
		//printf("Ultimo da fila em S consumido\n");
	}
	pthread_mutex_unlock(&SulFila.fim_da_fila);
	
	pthread_mutex_unlock(&SulFila.alteraFila);//Terminou de mexer na fila
	
	return NULL;
}
void *westBat(void * tid)
{
	Fila * thisBat = (Fila *) tid; //Pega referencia do noh desse BAT

	pthread_mutex_lock(&OesteFila.alteraFila); //Mutex para acesso a fila do norte, verifica posicao
	while (OesteFila.primeiroNo != thisBat) //Se nao for o primeiro noh espera ser chamado ->while garante veracidade do caso
	{ 
		pthread_cond_wait(&(thisBat->condition), &OesteFila.alteraFila); //Libera mutex e espera sinal
	}
	pthread_cond_broadcast(&batmanCond);
	pthread_mutex_lock(&cruzamento); //Entra na regiao critica do cruzamento para testar variaveis
	printf("BAT %d W chegou no cruzamento\n", thisBat->id); //Informa que o BAT chegou no cruzamento
	
	if (OesteFila.tamanho > MAX_SIZE)//Ultrapassa limite, cede a vez ------ DESNECESSARIO???
	{
		give_mask |= WEST; //Mascara que indica bats que podem ceder a vez
	}
	want_mask |= WEST; //Ele quer de qq jeito, so perciso do ir pra caso ele possa passar a vez
	newData++;
	threadData++;
	pthread_cond_broadcast(&batmanCond);
	pthread_cond_wait(&thisBat->condition, &cruzamento);
	
	printf("BAT %d W saiu no cruzamento\n", thisBat->id);
	threadData--;
	want_mask &= ~(WEST);
	give_mask &= ~(WEST);
	pthread_cond_broadcast(&batmanCond);
	pthread_mutex_unlock(&cruzamento);//Ja saiu do cruzamento
	
	pthread_mutex_lock(&OesteFila.fim_da_fila); //Altera o estado da fila, utilizado para nao inserir enquanto remove
	OesteFila.tamanho--;
	OesteFila.primeiroNo = thisBat->prox;
	if (thisBat->prox != NULL)
	{
		pthread_cond_broadcast(&thisBat->prox->condition);
	}
	else
	{
		OesteFila.ultimoNo = NULL;
		//printf("Ultimo da fila em S consumido\n");
	}
	pthread_mutex_unlock(&OesteFila.fim_da_fila);
	
	pthread_mutex_unlock(&OesteFila.alteraFila);//Terminou de mexer na fila

	return NULL;
}
void *eastBat(void * tid)
{
	Fila * thisBat = (Fila *) tid;
	pthread_mutex_lock(&LesteFila.alteraFila); //Mutex para acesso a fila do norte, verifica posicao
	while (LesteFila.primeiroNo != thisBat) //Se nao for o primeiro noh espera ser chamado ->while garante veracidade do caso
	{ 
		pthread_cond_wait(&(thisBat->condition), &LesteFila.alteraFila); //Libera mutex e espera sinal
	}
	pthread_cond_broadcast(&batmanCond);
	pthread_mutex_lock(&cruzamento); //Entra na regiao critica do cruzamento para testar variaveis
	printf("BAT %d E chegou no cruzamento\n", thisBat->id); //Informa que o BAT chegou no cruzamento
	
	if (LesteFila.tamanho > MAX_SIZE)//Ultrapassa limite, cede a vez
	{
		give_mask |= EAST; //Mascara que indica bats que podem ceder a vez
	}
	want_mask |= EAST; //Ele quer de qq jeito, so perciso do ir pra caso ele possa passar a vez
	newData++;
	threadData++;
	pthread_cond_broadcast(&batmanCond);
	pthread_cond_wait(&thisBat->condition, &cruzamento);
	
	printf("BAT %d E saiu no cruzamento\n", thisBat->id);
	threadData--;
	want_mask &= ~(EAST);
	give_mask &= ~(EAST);
	pthread_cond_broadcast(&batmanCond);
	pthread_mutex_unlock(&cruzamento);//Ja saiu do cruzamento
	
	pthread_mutex_lock(&LesteFila.fim_da_fila); //Altera o estado da fila, utilizado para nao inserir enquanto remove
	LesteFila.tamanho--;
	LesteFila.primeiroNo = thisBat->prox;
	if (thisBat->prox != NULL)
	{
		pthread_cond_broadcast(&thisBat->prox->condition);
	}
	else
	{
		LesteFila.ultimoNo = NULL;
		//printf("Ultimo da fila em S consumido\n");
	}
	pthread_mutex_unlock(&LesteFila.fim_da_fila);
	
	pthread_mutex_unlock(&LesteFila.alteraFila);//Terminou de mexer na fila
	return NULL;
}

void *batMan(void * tid)
{
	while (1)
	{
		char firstToGive = '\0';
		char signalWasTo = '\0';
		int from = 0, to = 0;
		pthread_mutex_lock(&cruzamento);//Pega lock do cruzamento para ter controle de tudo e garantir que a situacao atual nao sera alterada
		while (newData != threadData || newData == 0) pthread_cond_wait(&batmanCond, &cruzamento);
		newData--;
		if (want_mask & NORTH) //Verifica se o de maior prioridade deseja passar
		{
			if (!((give_mask & NORTH) && (want_mask & ~(NORTH)))) //Verifica se o north vai passar a vez e se ele for verifica se mais algum deseja passar
			{ //Entrando no if o north ou nao vai passar a vez ou é o unico que deseja ir
				if ((want_mask & ~(NORTH)) && !(give_mask))//Houve impasse e alguem nao quer passar a vez
				{
					printf("IMPASSE: N ");
					if ((want_mask & EAST) && !(give_mask & EAST)) printf ("e E ");//Se existem outros que querem passar e nao querem ceder a vez, impasse!
					if ((want_mask & SOUTH) && !(give_mask & SOUTH)) printf ("e S ");
					if ((want_mask & WEST) && !(give_mask & WEST)) printf ("e W ");
					printf(", sinalizando N para ir\n");
				}
				//Nao ha necessidade de definir signalWasTo pois N eh o de maior prioridade
				pthread_cond_broadcast(&NorteFila.primeiroNo->condition);
				goto fimDoLoop;
			}
			from = NorteFila.primeiroNo->id;
			firstToGive = 'N';
			give_mask &= ~(NORTH); //Ja cedeu uma vez, o mesmo bat nao cede de novo
			//O north passa a vez aqui
		}
		if (want_mask & EAST) //Chega aqui caso o North nao queira passar ou ele tenha passado a vez
		{
			if (!((give_mask & EAST) && (want_mask & ~(NORTH | EAST)))) 
			{
				if ((want_mask & ~(EAST | NORTH)) && !(give_mask))//Houve impasse e ninguem quer passar a vez
				{
					printf("IMPASSE: E ");
					if ((want_mask & SOUTH) && !(give_mask & SOUTH)) printf ("e S ");
					if ((want_mask & WEST) && !(give_mask & WEST)) printf ("e W ");
					printf(", sinalizando E para ir\n");
				}
				signalWasTo = 'E';
				to = LesteFila.primeiroNo->id;
				pthread_cond_broadcast(&LesteFila.primeiroNo->condition);
				goto fimDoLoop;
			}
			if (firstToGive == '\0') 
			{
				from = LesteFila.primeiroNo->id;
				firstToGive = 'E';
				give_mask &= ~(EAST); //Ja cedeu uma vez, o mesmo bat nao cede de novo
			}
		}
		if (want_mask & SOUTH) //Chega aqui caso o EAST nao queira passar ou ele tenha passado a vez
		{
			if (!((give_mask & SOUTH) && (want_mask & ~(NORTH | EAST | SOUTH)))) 
			{
				if ((want_mask & ~(SOUTH | EAST | NORTH)) && !(give_mask))//Houve impasse e ninguem quer passar a vez
				{
					printf("IMPASSE: S ");
					if ((want_mask & WEST) && !(give_mask & WEST)) printf ("e W ");
					printf(", sinalizando S para ir\n");
				}
				signalWasTo = 'S';
				to = SulFila.primeiroNo->id;
				pthread_cond_broadcast(&SulFila.primeiroNo->condition);
				goto fimDoLoop;
			}
			if (firstToGive == '\0') 
			{
				from = SulFila.primeiroNo->id;
				firstToGive = 'S';
				give_mask &= ~(SOUTH); //Ja cedeu uma vez, o mesmo bat nao cede de novo
			}
		}
		if (want_mask & WEST) //Chegar aqui implica que todos os outros ou nao tem BAT no cruzamento ou passaram a vez
		{
			signalWasTo = 'W';
			to = OesteFila.primeiroNo->id;
			pthread_cond_broadcast(&OesteFila.primeiroNo->condition);
			goto fimDoLoop;
		}
		printf("Nao Entrou em nenhum :(\n");
		fimDoLoop:
		if (firstToGive != '\0')//Alguem cedeu a vez
		{
			printf("BAT %d %c cedeu passagem BAT %d %c\n", from, firstToGive, to,signalWasTo);
			//Printa permissoes cedidas
		}
		pthread_mutex_unlock(&cruzamento);
	}
	//return NULL;
}
//-----------------------------------------------------------------------
//                      Auxiliares
//-----------------------------------------------------------------------

void insereFila(filaCabeca *fila, uint8_t ultimoId) {
    Fila *novo;
	novo = (Fila *)malloc(sizeof(Fila));
	nodesToFreeEnd->prox = malloc(sizeof(whatToFree));
	nodesToFreeEnd = nodesToFreeEnd->prox;
	nodesToFreeEnd->no = novo;
	nodesToFreeEnd->prox = NULL;
	
    novo->prox=NULL;
    novo->id=ultimoId;
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


