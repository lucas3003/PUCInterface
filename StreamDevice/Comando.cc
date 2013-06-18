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
	
	if(check) return false;	
	return true;
}

char * Comando :: sendPacket(char* fromStream, size_t * tam)
{
   debug("sendPacket chamado\n");
   debug(fromStream);
	
	char * buf;
    char *rec;
    char * prox = strstr(fromStream, " ");
    
    int i = ateEspaco(fromStream);
    rec = (char *) malloc(i);
    sscanf(fromStream, "%s", rec);
    prox++;
    
   if(strcmp(rec, "LER_VARIAVEL") == 0) comando = LER_VARIAVEL;
   else if(strcmp(rec, "ESCREVER_VARIAVEL") == 0) comando = ESCREVER_VARIAVEL;

   if(comando == LER_VARIAVEL)
   {	    
	    unsigned int checksum = 0;
	    int indice = 0;
	    buf = (char *) malloc(6*sizeof(char));
	    *tam = 6*sizeof(char);
	    
		//Destino
		process(&prox, &rec);
		debug(rec);
		
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
   else if(comando == ESCREVER_VARIAVEL)
   {	   
	   char * destino, * tamanho;	 
	   int i;
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
	   *tam = 5*sizeof(char) + (atoi(tamanho)+1)*sizeof(char);
	   
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
	   
	   char str[30];
	   
	    i = 0;	    
	   	while(i < atoi(tamanho)+5)
		{						
			checksum += buf[i]&255;
			i++;
		}
	   
  		   	sprintf(str, "%u\n", checksum);
			debug(str);
	   
	   buf[atoi(tamanho)+5] = ~(checksum & 255) + 1;
   }
   
   return buf;    
}


char * Comando :: receivedPacket(unsigned char* enderecamento,unsigned char* cabecalho,unsigned char* carga, int tam)
{
	char * result;	
	int i;
	
	double volts;
	
	if(verificaChecksum(enderecamento, cabecalho, carga, tam))		
	{
		if(cabecalho[0] == LEITURA_VARIAVEL)
		{
			int temp = 0; i;
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
		error("Checksum invalido");		
		return "Checksum invalido";
	}	
}
