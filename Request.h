#ifndef REQUEST_H_   /* Include guard */
#define REQUEST_H_

#define REQUEST_PUT 1
#define REQUEST_GET 2
#define REQUEST_DIR 3

struct request {
    int  kind;
	int  nbbytes;  /* pour PUT seulement */
    char path[128];
};
/* on alignera la taille de la strucuture sur un multiple de 8 octets (pour tea) en controlant * MAXPATH */

#define ANSWER_OK      0
#define ANSWER_UNKNOWN 1        /* requête inconnue */
#define ANSWER_ERROR   2        /* erreur lors du traitement */

struct answer {
	int  ack;
	int  nbbytes;  /* pour GET seulement */
	int  errnum;   /* significatif ssi != 0 et ack == ANSWER_ERROR */
	int _pad[1];   /* aligne la taille sur un multiple de 8 octests */
};

#endif