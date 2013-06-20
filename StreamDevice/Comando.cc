#include <string.h>
#include <stdlib.h>
#include <cstring>
#include <Comando.h>
#include "StreamError.h"
#include <string.h>
#include <stdint.h>

/*
 * Prox: String que tem todo o comando vindo do StreamDevice. Porem, essa string vai sendo "picotada" durante os processos,
 * para nao haver duplicacao de processamento.
 * 
 * Processed: String que tera a string que vai desde o primeiro caractere de prox ate o primeiro espaco
*/
void Comando :: process(char **prox, char **processed)
{
    int i = ateEspaco(*prox);
    *processed = (char *) malloc(i);
    sscanf(*prox, "%s", *processed);
    *prox = strstr(*prox, " ")+1;		    
}

int Comando :: ateEspaco(char * str)
{
	int i = 0;
	
    while(1)
    {
		if(str[i] == ' ') break;
		i++;
	}	
	
	return i;
}

//Observacao: O checksum esta vindo junto com a carga
bool Comando :: verificaChecksum(unsigned char * enderecamento, unsigned char * cabecalho, unsigned char * carga, int tam)
{
	int i;
	unsigned char conteudo_total = 0, check;
	
	
	//Como o enderecamento e o cabecalho possuem o mesmo tamanho, da para realizar em um so for
	for(i = 0; i < 2; i++)
	{	
		conteudo_total += enderecamento[i];
		conteudo_total += cabecalho[i];
	}
	
	
	for(i = 0; i < tam; i++)
		conteudo_total += carga[i];	
	
	check = conteudo_total + carga[tam];
	
	debug("Checksum correto: %d", 256-conteudo_total);
	
	if(check) return false;	
	return true;
}


char * Comando :: sendPacket(char* fromStream, size_t * tam)
{
   debug("sendPacket chamado\n");
   debug(fromStream);
	
    char *rec;
    prox = strstr(fromStream, " ");
    
    rec = (char *) malloc(ateEspaco(fromStream));
    sscanf(fromStream, "%s", rec);
    prox++;
    
    debug("\nCOMANDO ENVIADO: t%st\n\n", rec);
    
   if(strcmp(rec, "LER_VARIAVEL") == 0) 
   {
	   comando = LER_VARIAVEL;
	   *tam = 6*sizeof(char);
	   lerVariavel();
   }
   else if(strcmp(rec, "ESCREVER_VARIAVEL") == 0) 
   {	  
	   comando = ESCREVER_VARIAVEL;
	   *tam = escreverVariavel();
   }
   else if(strcmp(rec, "TRANSMITIR_BLOCO_CURVA") == 0)
   {	   
	   comando = TRANSMITIR_BLOCO_CURVA;
	   *tam = transmitirBlocoCurva();
   }
   
   return buf;    
}


char * Comando :: receivedPacket(unsigned char* enderecamento,unsigned char* cabecalho,unsigned char* carga, int tam)
{
	char * result;	
	int i;
	
	double volts;
	
	//TIRAR
	debug("%d\n",enderecamento[0]);
	debug("%d\n",enderecamento[1]);
	debug("%d\n",cabecalho[0]);
	debug("%d\n",cabecalho[1]);
	debug("%d\n",carga[0]);
	debug("%d\n",carga[1]);
	debug("%d\n",carga[2]);
	debug("%d\n",carga[3]);
			
	//
	
	if(verificaChecksum(enderecamento, cabecalho, carga, tam))		
	{
		if(cabecalho[0] == LEITURA_VARIAVEL)
		{
			
			int temp = 0;
			unsigned int conteudo = 0;
			char str[30];
			
								
			for(i = 0; i < tam; i ++)
			{
				conteudo = conteudo << 8;
				conteudo += carga[i];
			}			

			volts = ((20*conteudo)/262143.0)-10;

			sprintf(str, "%d\n", conteudo);
			debug(str);			
			
			sprintf(str, "%f\n", volts);
			
			result = (char *) malloc(sizeof(str) + sizeof("LEITURA_VARIAVEL"));			
			sprintf(result, "LEITURA_VARIAVEL %f", volts);
						
			return result;
		}
		else if(cabecalho[0] == OK_COMMAND)
		{
			return "OK";
		}
	}
	else
	{
		error("Checksum %d invalido", carga[tam]);		
		return "Checksum invalido";
	}	
}

int Comando :: escreverVariavel()
{
	   char * destino, * tamanho, * rec;	 
	   int i, result;
	   int checksum = 0;
	   double tensao;
	   unsigned int conteudo;	  
	  
	   //Destino
	   process(&prox, &rec);	   	   
	   destino = rec;	   
	   
	   //Tamanho
	   process(&prox, &rec);
	   tamanho = rec;
	   
	   buf = (char *) malloc(5*sizeof(char) + (atoi(tamanho)+1)*sizeof(char));
	   result = 5*sizeof(char) + (atoi(tamanho)+1)*sizeof(char);
	   
	   buf[0] = atoi(destino);
	   buf[1] = 0;
	   buf[2] = ESCREVER_VARIAVEL;
	   buf[3] = atoi(tamanho)+1;
	   
	   //Id da variavel
	   process(&prox, &rec);
	   buf[4] = atoi(rec);
	   
	   //Valor
	   process(&prox, &rec);
	   tensao = atof(rec);
	   conteudo = (unsigned int) ((tensao+10)*262143)/20.0;

	   for(i = atoi(tamanho)+4; i > 4; i--)
	   {		   
		   buf[i] = conteudo & 255;		   
		   conteudo = conteudo >> 8;		
	   }
	   
	    i = 0;	    
	   	while(i < atoi(tamanho)+5)
		{						
			checksum += buf[i]&255;
			i++;
		}
	   
	   buf[atoi(tamanho)+5] = ~(checksum & 255) + 1;
	   	
	   return result;
}

void Comando :: lerVariavel()
{
		char * rec;
	    unsigned int checksum = 0;
	    int indice = 0;
	    buf = (char *) malloc(6*sizeof(char));	    
	    
		//Destino
		process(&prox, &rec);
		
		buf[0] = atoi(rec);		
		buf[1] = 0;
		buf[2] = LER_VARIAVEL;
		buf[3] = 1;
		
		//Id variavel
		process(&prox, &rec);
		debug(rec);
					
		buf[4] = atoi(rec);		
		
		while(indice <= 4)
		{
			checksum += buf[indice];
			indice++;			
		}
		
		buf[5] = ~(checksum & 0xFF) + 1;	
}

int Comando :: transmitirBlocoCurva()
{
	char * rec;
	buf = (char *) malloc(6*sizeof(char));
	
	//Destino
	process(&prox, &rec);
	
	buf[0] = atoi(rec);
	buf[1] = 0;
	buf[2] = TRANSMITIR_BLOCO_CURVA;
	buf[3] = 2;
	
	//Id curva
	process(&prox, &rec);	
	buf[4] = atoi(rec);
	
	//Offset do bloco
		
	process(&prox, &rec);	
	buf[5] = atoi(rec);
	
	return 6; //2 do enderecamento, 2 do cabecalho e 2 da carga
}
