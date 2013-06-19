#ifndef COMANDO_H_
#define COMANDO_H_

enum COMANDOS
{   
	LER_VARIAVEL      = 0x10,
	LEITURA_VARIAVEL  = 0x11,
	ESCREVER_VARIAVEL = 0x20,
	OK_COMMAND        = 0xE0		
};

class Comando
{	
	int comando;		
	char * buf;
	char * prox;
	
	void process(char **prox, char **processed);
	void lerVariavel();
	int escreverVariavel();
	int ateEspaco(char * str);
	bool verificaChecksum(unsigned char * enderecamento,unsigned char * cabecalho,unsigned char * carga, int tam);
	
public:
	char * sendPacket(char* fromStream, size_t * tam);
	char * receivedPacket(unsigned char* enderecamento,unsigned char* cabecalho,unsigned char* carga, int tam);
};

#endif
