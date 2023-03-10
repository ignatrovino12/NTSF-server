#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

/* portul folosit */

#define PORT 2769

/* functie de convertire a adresei IP a clientului in sir de caractere */
char * conv_addr (struct sockaddr_in address)
{
  static char str[25];
  char port[7];

  /* adresa IP a clientului */
  strcpy (str, inet_ntoa (address.sin_addr));	
  /* portul utilizat de client */
  bzero (port, 7);
  sprintf (port, ":%d", ntohs (address.sin_port));	
  strcat (str, port);
  return (str);
}

// declarare functii

void da_cu_zarul ( int fd,int &val_zar) ;
void afisare ( int fd ) ;
void resetare_joc(int fd,int ready[5],int &curent,int &nrjucatori,int tabla[41],int baza[5],int final[5][5],bool &exista_castigator,char nume_client[5][30]);
void afisare_reguli( int fd);

/* programul */
int main ()
{
    struct sockaddr_in server;	
    struct sockaddr_in from;
    fd_set readfds;		
    fd_set actfds;	
    struct timeval tv;
    int sd, client;	
    int optval=1; 		
    int fd;		
    int nfds;
    int i,j;
   
    char nume_client[5][30];
    char nume_leaderboard[100][30];
    int nr_castiguri[100];
    int nr_leaderboard=0;
    int curent_castigator=0;

    int ready[5];	// vector ready cu fd-urile clientilor	
    int curent=1,nrjucatori=0;
    bool exist,exista_castigator=0;
    int fd_castigator=0;

    int stadiujoc=0; // stadiujoc 0- trebuie dat cu zarul 1- trebuie mutat/skip 2-end turn
    int mutare=0; //mutare=0 pt MUTA mutare=1 pt selectare pion

    int val_zar;
    int tabla[41];  /*  p1: 1- 11,12,13,14 p2:11- 21,22,23,24 p3: 21- 31,32,33,34  p4: 31- 41,42,43,44*/
    int baza[5]; // numar pioni baza p1-p4 0,1,2,3,4
    int final[5][5]; // numar pioni castigati p1-p4 la 4 castigi
    
    for(i=1;i<=4;i++) {ready[i]=-1;baza[i]=4;}

    for(i=1;i<=4;i++) 
    for(int j=1;j<=4;j++) final[i][j]=0;

    for(i=1;i<=40;i++) tabla[i]=0;

    for(i=1;i<=100;i++) nr_castiguri[i]=0;

    socklen_t len;	
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server] Eroare la socket().\n");
        return errno;
    }
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));
    bzero (&server, sizeof (server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server] Eroare la bind().\n");
        return errno;
    }
    if (listen (sd, 5) == -1)
    {
        perror ("[server] Eroare la listen().\n");
        return errno;
    }
    FD_ZERO (&actfds);	
    FD_SET (sd, &actfds);	
    nfds = sd;
    printf ("[server] Asteptam la portul %d...\n", PORT);
    fflush (stdout);
    while (1)
    {
        bcopy ((char *) &actfds, (char *) &readfds, sizeof (readfds));
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if (select (nfds+1, &readfds, NULL, NULL, &tv) < 0)
	    {
	        perror ("[server] Eroare la select().\n");
	        return errno;
	    }
        if (FD_ISSET (sd, &readfds))
	    {
            len = sizeof (from);
            bzero (&from, sizeof (from));
            client = accept (sd, (struct sockaddr *) &from, &len);
	        if (client < 0)
	        {
	            perror ("[server] Eroare la accept().\n");
	            continue;
	        }
            if (nfds < client) /* ajusteaza valoarea maximului */
                nfds = client;
	        FD_SET (client, &actfds);
            printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n",client, conv_addr (from));
            fflush (stdout);
	    }
        for (fd = 0; fd <= nfds; fd++)
	    {
            

	        if (fd != sd && FD_ISSET (fd, &readfds)) 
	        {      
                //primire mesaj de la clienti

                 char buffer [ 100 ] ;		
                 int bytes ;			
                 char msg [ 100 ] ;
                 char msgrasp [ 100 ] = " " ;
                 bytes = read ( fd , msg , sizeof ( buffer ) ) ; 
                 if ( bytes < 0 )
                {
                perror ( "Eroare la read() de la client.\n" ) ;
                return 0 ;
                 }
                 printf ( "[server]Mesajul a fost receptionat...%s\n" , msg ) ;      

            // exista sau nu playerul deja   
             
            bool first_login;
            first_login=0;
            exist=0; 
            for(i=1;i<=4;i++) if(ready[i]==fd) exist=1;

            if(exist==0 && ready[4]==-1)
             {for(i=1;i<=4;i++) if(ready[i]==-1) { ready[i]=fd;
                                                   nrjucatori++;
                                                   first_login=1;

                                                  strcpy(nume_client[i],msg);

                                                  bool exista_numele=0;
                                                  int nr_cur=0;
                                                  for(j=1;j<=nr_leaderboard;j++) 
                                                   if(strcmp(nume_leaderboard[j],msg)==0) {exista_numele=1; nr_cur=j;}

                                                 if(exista_numele==0) {nr_leaderboard++;nr_cur=nr_leaderboard;}
                                                  strcpy(nume_leaderboard[nr_cur],msg);

                                                   break;} }

            else if(exist==0) { write (fd, "Deja 4 jucatori", 16) ;}
       
                  
              if(first_login==0) {
            // orice player
               if ( strcmp ( msg , "Leaderboard" ) == 0 ) {
              
                char vectorul[2000],vec[5];
                bzero ( vectorul , 2000 ) ;
              for(i=1;i<=nr_leaderboard;i++) {
                            
                
                strcat ( vectorul ,"Playerul ");
                strcat(vectorul,nume_leaderboard[i]);
                strcat ( vectorul ," are scorul ");
                sprintf(vec, "%d", nr_castiguri[i]);
                strcat ( vectorul ,vec);
                strcat ( vectorul ,"\n");

              }
                int b = 0 ;
                b = strlen ( vectorul ) ;
                write ( fd , vectorul , b ) ;

/*
               for(i=1;i<=nr_leaderboard;i++) {printf("Playerul %s",nume_leaderboard[i]);
                                               printf(" are scorul %d\n",nr_castiguri[i]);}*/
               }

                 
               else if ( strcmp ( msg , "Reset" ) == 0 ) {stadiujoc=0;mutare=0; resetare_joc( fd,ready, curent, nrjucatori,tabla, baza, final,exista_castigator, nume_client);}  
               else if ( strcmp ( msg , "Comenzi" ) == 0 ) {afisare ( fd );}
               else if ( strcmp ( msg , "Reguli" ) == 0 ) {afisare_reguli ( fd );}
               else if( strcmp ( msg , "Exit" ) == 0 ) {nrjucatori--; 
                                                  for(i=1;i<=4;i++) {if(ready[i]==fd) ready[i]=-1;}
                                                  
                                                    printf ("[server] S-a deconectat clientul cu descriptorul %d.\n",fd);
                                                    fflush (stdout);
                                                    close (fd);		/* inchidem conexiunea cu clientul */
                                                    FD_CLR (fd, &actfds);/* scoatem si din multime */

                                                   write (fd, "Exit", 5) ;}
               else if ( strcmp ( msg , "Tabla" ) == 0 ) {
                                                     for(i=1;i<=40;i++) {
                                                        printf("%d ",tabla[i]);
                                                     if(i==10 || i==20 || i==30 || i==40) printf("\n");}

                                                      printf("\n");
                                                      for(i=1;i<=4;i++) {
                                                      printf("Spatiu final al jucatorului %d",i);
                                                      printf(": ");
                                                      for(j=1;j<=4;j++)  printf("%d ",final[i][j]);
                                                      printf("\n");
                                                      }

                                                      printf("\n");
                                                      for(i=1;i<=4;i++) {
                                                      printf("Numar de pioni in baza ai jucatorului %d",i);
                                                      printf(": ");
                                                      printf("%d ",baza[i]);
                                                      printf("\n");
                                                      }

                                                      write (fd, "Am afisat tabla in server\n", 27) ;
                                                    }
               

            // playerul actual
           else if(nrjucatori>1 && exista_castigator==0) {
                if(ready[curent]==fd){ 
                
                if(stadiujoc==0) {
                 if ( strcmp ( msg , "Roll" ) == 0 ) {stadiujoc=1;da_cu_zarul( fd,val_zar ); }

                 //comenzi nefolosibile inca
                 else if ( strcmp ( msg , "Move" ) == 0 ) write(fd, "Da cu zarul mai intai\n", 23) ;
                 else if ( strcmp ( msg , "End" ) == 0 )  write(fd, "Da cu zarul mai intai\n", 23) ;
                 
                 else write (fd, "Nu exista comanda sau nu este folosibila in acest moment\n", 58) ;
                 
                }

            
                else if(stadiujoc==1){


                   //comanda de mutare
                   if ( strcmp ( msg , "Move" ) == 0 && mutare==0) {                 
                   mutare=1;
                   char vector [ 100 ] ;
                   bzero ( vector , 100 ) ;
                   strcat ( vector , "Ce pion?\n" ) ;  
                   int b = 0 ;
                   b = strlen ( vector ) ;
                   write ( fd , vector , b ) ;

                   }

                   else if ( strcmp ( msg , "Skip" ) == 0  && mutare==1)  {stadiujoc=2;mutare=0;write(fd, "Skip turn\n", 11) ;}
                   
                   //selectare pion
                   else if ( (strcmp ( msg , "1" ) == 0 || strcmp ( msg , "2" ) == 0 || strcmp ( msg , "3" ) == 0 || strcmp ( msg , "4" ) == 0 ) && mutare==1) { 
                   int pion=atoi(msg);
                   bool existapion=0;
                   int pozpion=0,mutareinvalida=0; //mutareinvalida valori diferite pt output
                   pion=pion+10*curent; 

                   //printf ( "%d\n",pion) ;
                   // printf ( "%d\n",curent) ;

                   //implementare mutat pioni cu constrangerile de reguli
                   for(i=1;i<=40;i++) if(tabla[i]==pion) {existapion=1;pozpion=i;}
                
                   if(existapion==0){
                    bool existabaza=1;

                    //pionul exista la final
                   for(i=1;i<=4;i++) if(final[pion/10][i]==pion) 
                                     {  existabaza=0;
                                        if(val_zar+i<=4){
                                        if(final[pion/10][i+val_zar]==0) {final[pion/10][i+val_zar]==pion;final[pion/10][i]=0;}
                                        else mutareinvalida=1; // deja exista pion de al tau acolo
                                     }
                                       else mutareinvalida=1; // trece de limita final-ului                                      
                                       break;//i=5
                                     }
                   
                   //pionul exista la baza 
                   // trebuie dat 6 apoi:
                    //1  pion de al tau
                    //2 pion advers
                    //3 spatiu liber
                    if(existabaza==1 ){

                  if(val_zar==6) {
                  
                  //3
                  if(tabla[(curent-1)*10+1]==0) {baza[curent]--;
                                                 tabla[(curent-1)*10+1]=pion;}
                     
                  //1
                  else if(tabla[(curent-1)*10+1]/10==curent) mutareinvalida=1; // deja este un pion de al tau acolo

                  //2
                  else{
                    baza[curent]--;
                   baza[tabla[(curent-1)*10+1]/10]++;
                   tabla[(curent-1)*10+1]=pion;
                  }

                  }
                  
                  else mutareinvalida=1; // nu s-a dat 6 din baza                   

                    }
                    

                   }
                   
                   //pionul este pe tabla          
                    //1 pion de al tau
                    //2 pion advers
                    //3 spatiu liber
                    //4 trece in final
                   else{

                  int pozmax;
                  pozmax=(curent*10+30)%40;
                  if(curent==1) pozmax=40;
                  
                  //tabla[(pozpion+val_zar)%40]
                 
                  //4
                  if(pozpion<=pozmax && (pozpion+val_zar)>pozmax ){                                 
                      
                     // trece de limita final-ului  
                    if(pozpion+val_zar-pozmax>4)        
                         mutareinvalida=1; // trece de limita final-ului  

                     // exista pion de al tau
                     else if(final[pion/10][pozpion+val_zar-pozmax]!=0) 
                      mutareinvalida=1; // deja exista pion de al tau acolo
                                 
                     // nu exista pion de al tau
                     else{
                     tabla[pozpion]=0;
                     final[pion/10][pozpion+val_zar-pozmax]=pion;
                     }
                  
                  }

                  else{
                
               // if(pozpion>pozmax )  (pozpion+val_zar)%40 - updatare
               
                //3
                if(tabla[(pozpion+val_zar)%40 ]==0) { tabla[pozpion]=0;
                                                     tabla[(pozpion+val_zar)%40]=pion;}
                     
                //1
                else if(tabla[(pozpion+val_zar)%40 ]/10==curent) mutareinvalida=1; // deja este un pion de al tau acolo

                //2
                else{
                   tabla[pozpion]=0;
                   baza[tabla[(pozpion+val_zar)%40]/10]++;
                   tabla[(pozpion+val_zar)%40]=pion;
                  }

                  }


                   }

                  
                  char vector [ 100 ] ;
                   bzero ( vector , 100 ) ; 
                   int b = 0 ;
                   

                   //mutare efectuata cu succes
                   if(mutareinvalida==0){
                   mutare=0;
                   stadiujoc=2;
                   
                   strcat ( vector , "Ai mutat pionul\n" ) ;  
                   b = strlen ( vector ) ;
                   write ( fd , vector , b ) ;
                   }
                   
                   //mutare invalida
                   else if(mutareinvalida==1){
                   strcat ( vector , "Mutare invalida\n" ) ;  
                   b = strlen ( vector ) ;
                   write ( fd , vector , b ) ;
                   }


                   }



                 //comenzi nefolosibile inca
                 else if ( strcmp ( msg , "Roll" ) == 0 ) write(fd, "Muta pionul \n", 14) ;
                 else if ( strcmp ( msg , "End" ) == 0 )  write(fd, "Muta pionul mai intai \n", 24) ;
                 //else if ( strcmp ( msg , "Skip" ) == 0 )  {stadiujoc=2;mutare=0;write(fd, "Skip turn\n", 11) ;}

                 else write (fd, "Nu exista comanda sau nu este folosibila in acest moment\n", 58) ;
                }


                else{
                   

                   if(exista_castigator==0) {
                 
                 
                 // pentru final joc
                 /* 
                 final[curent][1]=1;
                 final[curent][2]=1;
                 final[curent][3]=1;
                 final[curent][4]=1;
                 */
                 
                 // caz castigat joc
                 if (final[curent][1] !=0 && final[curent][2] !=0 && final[curent][3] !=0 && final[curent][4] !=0)
                   { exista_castigator=1; curent_castigator=curent;
                    for(i=1;i<=nr_leaderboard;i++) {
                    if(strcmp(nume_client[curent_castigator],nume_leaderboard[i])==0) nr_castiguri[i]++;
                    }
                   
                   if(curent==nrjucatori) curent=1;
                    else curent++;
                      stadiujoc=0;
                   write (fd, "Ai castigat\n", 13) ;
                   }

                 else if ( strcmp ( msg , "End" ) == 0 ) { if(curent==nrjucatori) curent=1;
                                                      else curent++;
                                                      stadiujoc=0;
                                                      write (fd, "Your turn ended\n", 17) ;}
                

                 //comenzi nefolosibile inca
                 else {write (fd, "Foloseste comanda End sa termini tura\n", 39) ;} 
                   }
                }
                
               
                                        
            }
                
            //player in asteptare 

                else {
                if ( strcmp ( msg , "Comenzi" ) == 0 ) {afisare ( fd );}
                else write (fd, "Nu e randul tau\n", 17) ;} 

	            
            }
            else {
            if(exista_castigator==0) write(fd,"Nu te poti juca de unul singur\n",32);
            else {
                char vector [100];
                bzero ( vector , 100 ) ;
                strcat ( vector ,"A castigat playerul cu numele: ");
                strcat(vector, nume_client[curent_castigator]);
                int b = 0 ;
                b = strlen ( vector ) ;
                write ( fd , vector , b ) ;
           

            }

            }
          
              
                   }     
           else write(fd,"Am primit numele\n",18);        
                   }
	    }			
    }				
}			


// de pus toate comenzile in strcat
void afisare ( int fd ) 
{
  char vector [ 400] ;
  bzero ( vector , 400 ) ;
  strcat ( vector , "Comenzi - lista comenzi\nExit - iesi din joc\nLeaderboard - afiseaza lista cu punctaje a jucatorilor\n" );
  strcat ( vector , "Reset - reseteaza jocul\nRoll - da cu zarul\nReguli - regulile jocului\n" ) ;
  strcat ( vector , "Move - muta un pion, dupa care alege un numar de la 1 la 4 pentru a alege ce pion muti\n" ) ;
  strcat ( vector , "Skip - da skip la tura ta\nEnd - termina-ti tura\n" ) ;
  strcat (vector,"Tabla (comanda pentru server, folosita la verificarea tablei)\n");
  int b = 0 ;
  b = strlen ( vector ) ;
  //printf("%d",b);
  write ( fd , vector , b ) ;
}


void afisare_reguli ( int fd ) 
{
  char vector [ 499] ;
  bzero ( vector , 499 ) ;
  strcat ( vector , "Reguli joc:\n" );
  strcat ( vector , "Fiecare jucator detine 4 pioni si scopul este de a ajunge cu acestia din baza la pozitiile finale prin mutarea acestora cu valoarea zarului.\n" ) ;
  strcat ( vector , "Pentru a juca trebuie sa folosesti la tura ta comenzile acestea in ordine:\n" ) ;
  strcat ( vector , "1: Roll ce iti va afisa valoarea zarului pe care ai dat-o\n" ) ;
  strcat ( vector , "2: Move pentru a arata ca doresti sa muti un pion\n" ) ;
  strcat ( vector , "3: 1/2/3/4 in functie de ce pion doresti sau Skip pentru a da skip la tura\n" ) ;
  strcat ( vector , "4: End pentru a finaliza tura ta si a permite urmatorului jucator sa isi execute tura\n" ) ;
  int b = 0 ;
  b = strlen ( vector ) ;
  //printf("%d",b);
  write ( fd , vector , b ) ;
}

void da_cu_zarul( int fd ,int &val_zar) 
{ 
  int nr=(rand () % 6) + 1 ;
  nr=6; //doar momentan pt testari
  val_zar=nr;
  char vector [100];
  bzero ( vector , 100 ) ;
  sprintf(vector, "%d", nr);
  strcat ( vector ,"\n");
  int b = 0 ;
  b = strlen ( vector ) ;
  write ( fd , vector , b ) ;

}

// momentan doar reseteaza jocul cu playerii curenti
void resetare_joc(int fd,int ready[5],int &curent,int &nrjucatori,int tabla[41],int baza[5],int final[5][5],bool &exista_castigator,char nume_client[5][30]) {
int i,j,w=3;
curent=1;exista_castigator=0;

nrjucatori=0;
for(i=1;i<=4;i++) if(ready[i]!=-1) nrjucatori++;

//caz player deconectat si resetare joc, se vor reseta pozitiile/ready fd-ul
if(nrjucatori!=4) {
while(w)  {
for(i=1;i<4;i++) if(ready[i]==-1) {ready[i]=ready[i+1];ready[i+1]=-1;
                                  strcpy(nume_client[i],nume_client[i+1]);
                                  strcpy(nume_client[i+1],"");
                                    
                                    }
ready[4]=-1;
w--;
}       
}      

// inserat resetarea tablei de joc
for(i=1;i<=40;i++) tabla[i]=0;
for(i=1;i<=4;i++)  baza[i]=4;

for(i=1;i<=4;i++) 
for(j=1;j<=4;j++) final[i][j]=0;
write ( fd , "Resetare joc\n" , 14 ) ;
}

