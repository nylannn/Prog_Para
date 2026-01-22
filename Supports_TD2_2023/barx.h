/*
 * routines de creation et modification d'un rectangle
 */
 
/* crée une fenetre rectangle rouge */ 
void initrec(void);
/* detruit la fenetre rectangle */ 
void detruitrec(void); 
/* écrit la chaîne de caractère string à la position x,y */ 
void drawstr(int x, int y, char *str, int lng);
/* dessine un rectangle aux contours blancs à la position x,y de hauteur hter et largeur larg*/
void drawrec(int x, int y, int larg, int hter);
/* remplit un rectangle avec la couleur col1 de hauteur hter et largeur larg*/
void fillrec(int x, int y, int larg, int hter, char *col1);
/* flush display buffer */ 
void flushdis(void);


// On ne se servira pas de des fonctions ci-dessous
void initwind(char *col);
void setbigrec(char *col);
void setlitrec(char *col);
void ecritrec(char *buf,int lng);
void rectvert(int n);
int attendreclic(void);

