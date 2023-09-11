#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct generalList {
    int inf;
    int sup;
    char instr;
    char **old;
    struct generalList* next;
    struct generalList* prec;
    int diff; // dim old
} ;
typedef struct generalList *genList;

//mi dicono quanti undo e redo posso fare
int undo_number = 0;  //TODO potrebbe dare problemi
int undo_Index = 0;
int redo_number = 0;

genList lastValidCommand = NULL;        //Punta ultima posizione per eseguire undo e redo
genList currCommand = NULL;
int flagDelete = 0;
int flagUndoRedoPosition = 0;   //Flag che mi da la posizione corretta da dove eseguire undo e redo mentre sovrascrivo comandi
int flagRedoDeleteCommand = 0; //Flag che mi serve nel caso un cui eseguo tutte le redo possibili e poi successivamente un comando, e quindi non devo
                               // cancellare nessun comando in createNewCommand


typedef struct charList *list;

char** cFunction(genList *general, char **text, int *dimText, int *dimArray, FILE *fp) {

    genList gen = *general;
    gen->old = NULL;
    char input[1024];
    int i, j=0, dim , inf = gen->inf , sup = gen->sup ;

    if (inf <= (*dimText)){
        gen->old = (char**)malloc(sizeof(char *) * (sup-inf + 1));
        for ( int o = 0 ; o< sup-inf + 1; o++){
            gen->old[o] = NULL;
        }
    }

    for (i = inf; i <= sup; i++) {
        if (fgets(input, 1024, fp) == NULL && input[0] != '.') return 0;
        if (i <= (*dimText)) {
            gen->old[j] = text[i - 1];
            text[i - 1] = strdup(input);
            j++;
        } else {
            //write new lines non sto sovrascrivendo

            if (i > *dimArray) {

                if( gen->sup - gen->inf > 20) dim = gen->sup - gen->inf;
                else dim = 20;
                //dim=1000000;
                char **temp = NULL;
                temp = (char**)realloc(text, sizeof(char *) * ((*dimArray)+dim));
                /*for( int u = *dimArray; u < (*dimArray)+dim; u++){
                    text[u] = NULL;
                }*/
                /* Test bar for safety before blowing away foo */
                if (temp != NULL) {
                    text = temp;
                    temp = NULL;
                } else {
                    //printf("Memory error.\n");
                    free(temp);
                    exit(-1);
                }
                (*dimArray) = (*dimArray) + dim;
            }
            (*dimText)++;
            text[i - 1] = strdup(input);
        }

    }
    return text;
}

char** dFunction(genList *general, char **text, int *dimText, int *dimArray){


    genList gen = *general;
    gen->diff = 0;
    int j, x=0 , inf = gen->inf , sup = gen->sup;
    //printf("%d,%d%c\n",gen->inf,gen->sup,gen->instr);
    // if the lines aren't in the text don't do nothing

    if( sup == 0 && inf == 1){
        gen->inf = 0;
    }
    else{
        if(*dimText > 0 && inf <= *dimText ) {
            gen->old = (char**)malloc(sizeof(char *) * (sup-inf + 1));


            // check to delete only existing lines
            //gen->sup = (gen->sup < *dimText) ? gen->sup : *dimText;  //sbagliato

            int oldSizeText = *dimText, diff ;
            if(gen->sup <= *dimText) diff = gen->sup - gen->inf;
            else diff = *dimText - gen->inf;
            int newSizeText = oldSizeText - diff - 1;

            // eseguo backup stringhe da cancellare

            for (int i = inf; i <= sup && i<=*dimText; i++) {
                gen->diff ++;
                gen->old[x] = text[i - 1];
                x++;
            }

            memmove(&text[inf - 1],&text[sup],sizeof(char*)*(*dimArray - sup ));

            for (int i = newSizeText; i <= diff; i++) {
                //text[i-1] = text[i + diff];
                text[i] = NULL;

            }


            //traslo nelle zone cancellate gli elementi
            /*for (int i = inf; i <= newSizeText; i++) {
                text[i-1] = text[i + diff];
                text[i+diff] = NULL;

            }*/

            for (j = newSizeText; j <= oldSizeText-1

Giuseppe Paolini, [11/12/2020 19:08]
; ++j) {
                text[j] = NULL;
            }
            *dimText = newSizeText;
        }
    }
    return text;
}

void pFunction (int inf, int sup, char **text, int numTextLines) {

    if ( sup == 0 ) {
        fputs(".\n", stdout);
        fflush(stdout);
        return;
    }
    if ( inf == 0 ) {
        inf = 1;
        fputs(".\n", stdout);
        fflush(stdout);
    }

    for(int i = inf; i <= sup; i++){
        fputs(i <= numTextLines ? text[i - 1] : ".\n", stdout);
        fflush(stdout);
    }
}

//TODO DOPO UNDO E REDO CURRENT COMMAND == LASTVALIDCOMMAND

void uFunction(genList *general, int numUndo, char **text, int *dimText) {

    genList gen = *general;
    char *temp = NULL;
    int i = 0, j = 0, k = 0 , x = 0, undo = undo_number , inf , sup;
    //printf( "Numero undo %d\n", numUndo);
    if(undo_number > 0) {
        if ( flagUndoRedoPosition == 1 ) gen = currCommand;
        else if( lastValidCommand != NULL) gen = lastValidCommand;
        inf = gen->inf;
        sup = gen->sup;
        if ( numUndo < undo ) undo_Index = undo_Index + numUndo;
        else undo_Index = undo_Index + undo;
        while( j < numUndo && j < undo ) {
            if( gen->inf == 0) {
                if(gen->prec != NULL){
                    gen = gen->prec;
                    inf = gen->inf;
                    sup = gen->sup;
                    x=0;
                } else break;
                j++;
                continue;  //caso in cui ho delete 0,0d, non faccio nulla e vado avanti
            }
            //printf( "Istruzione %d,%d%c\n", gen->inf,gen->sup,gen->instr);
            if (gen->instr == 'c') {

                for (i = gen->inf; i <= gen->sup; i++) {
                    if (gen->old != NULL && gen->old[x] != NULL ) { // Se il blocco old era già stato inizializzato in precedenza //TODO gen->old[x] se da errori
                        // Switch of lines between old ones and the text
                        //if( oldLine != gen->old) oldLine = oldLine->next;
                        temp = text[i - 1];
                        text[i - 1] = gen->old[x];
                        gen->old[x] = temp;
                    } else {
                        //Se il blocco old di gen non è stato mai inizializzato
                        if ( gen->old == NULL ) {
                            gen->old = (char**)malloc(sizeof(char *) * (sup-inf + 1));
                            for ( int o = 0 ; o< sup-inf + 1; o++){
                                gen->old[o] = NULL;
                            }

                        }
                        gen->old[x] = text[i - 1];
                        text[i -1] = NULL;
                        (*dimText)--;
                    }
                    x++;
                }
            } else {  //CASO DELETE

                int diff = 0, oldSizeText = *dimText;
                int newSizeText = oldSizeText + gen->diff ;

                if (gen->old != NULL) {      // True only when something was deleted

                    *dimText = newSizeText;

                    // Shifting right the elements starting from the last one e ripristino elementi
                    if( inf <= oldSizeText) { //<=

                        //parto dal fondo dell'array a traslare
                        for(int u = sup - inf + 1 + oldSizeText; u > sup ; u --){
                            text[u - 1] = text[u - gen->diff - 1];
                        }
                        for (int l = inf; l <= sup && l <= *dimText; l++) {
                            text[l - 1] = gen->old[x];
                            x++;
                        }
                    } else{
                        for (int l = inf; l <= sup && l <= *dimText; l++) {
                            text[l - 1] = gen->old[x];
                            x++;
                        }
                    }
                }
            }
            j++;
            if(gen->prec != NULL){
                gen = gen->prec;
                inf = gen->inf;
                sup = gen->sup;
                x=0;

Giuseppe Paolini, [11/12/2020 19:08]
}
        }
        flagUndoRedoPosition = 0 ;
        flagRedoDeleteCommand = 0 ;
    }
    if( numUndo > undo_number) undo_number = 0;
    else undo_number = undo_number - numUndo;
    redo_number = undo_Index;
    currCommand = gen;
    lastValidCommand = gen;

}
//todo sistema la dimText che non si alza nella redo e controlla se ordine di esecuzione di print è giusto ma credo di si
void rFunction(genList *general, int numRedo, char **text, int *dimText , int *dimArray) {


    genList gen = *general;
    int flag = 0, flag0 , flag1, x = 0, inf = gen->inf , sup = gen->sup ; //flag0 è per temp2line, flag1 se devo eliminare la old di una gen
    list oldLine = NULL, temp2Line = NULL;  //temp2Line mi serve nel caso in cui devo eliminare una next di una old fatta in più dalla malloc
    char *temp = NULL;
    int i, j = 0, k = 1 , dimensionText = *dimText, oldDimension = dimensionText ;

    if(redo_number > 0) {

        if ( flagUndoRedoPosition == 1 ) {
            if( undo_number == 0 ) gen = currCommand;
            else gen = currCommand->next;
        }
        else if( undo_number == 0 ) gen = lastValidCommand;
        else if( lastValidCommand != NULL ) gen = lastValidCommand->next;

        inf = gen->inf;
        sup = gen->sup;

        temp2Line = oldLine;
        //( "Istruzione %d,%d%c\n", gen->inf,gen->sup,gen->instr);

        while( j < numRedo && j < redo_number ) {
            if ( inf == 0 ) {
                if(gen->next != NULL){
                    gen = gen->next;
                    //oldLine = gen->old;
                    oldDimension = *dimText;
                    inf = gen->inf;
                    sup = gen->sup;
                    x=0;
                } else {
                    flag = 1;
                    break;
                }
                j++;
                continue;  //caso 0,0d
            }
            flag = 0;
            flag1 = 0;
            if (gen->instr == 'c') {                    //potrei eliminare il pezzo di old qua
                if (*dimText < sup ) {
                    *dimText = sup;
                    dimensionText = *dimText;           //aumento dimensione testo, sto ripristinando stringhe
                }
                for (i = inf; i <= sup; i++) {

                    // mi dice che devo ripristinare qualche stringa
                    if (gen->old != NULL ) {

                        // Switch of lines between old ones and the text
                        if( oldDimension != 0 ){

                            //Questo controllo lo faccio in caso sono in una situazione in cui è stata eseguita una delete e quindi non ho un testo da salvarmi come backup
                            if(text[i - 1] != NULL){
                                temp = text[i - 1];
                                text[i - 1] = gen->old[x];
                                gen->old[x] = temp;
                            }
                            else{

                                //flag1 = 1 se sono nel caso in cui non mi serve più la old creata precedentemente da una undo
                                if( i == inf ) flag1 = 1;

                                //flag0 = 1;
                                text[i - 1] = gen->old[x];
                                gen->old[x] = NULL;
                            }
                        }
                        else{
                            //se dimensione zero dopo undo allora non devo ripristinare nulla in oldLine.row
                            if( gen->old[x] != NULL) {
                                text[i - 1] = gen->old[x];
                                gen->old[x] = NULL;

                            }
                            else{
                                text[i - 1] = NULL;
                            }
                            flag1 = 1;
                        }

                    } else { //forse inutile

                        // If the text was written ex novo it just copy the text in the old lines and delete it
                        if ( text[i-1] != NULL){

Giuseppe Paolini, [11/12/2020 19:08]
gen->old[x] =text[i - 1];
                            (*dimText)--;
                        }
                    }
                    x++;
                }
            } else { //CASO DELETE


                dimensionText = *dimText;
                text = dFunction(&gen,text,&dimensionText , dimArray);
                *dimText = dimensionText;
            }
            j++;
            if( flag1 == 1){
                free(gen->old);
                gen->old = NULL;
            }
            if(gen->next != NULL){
                gen = gen->next;
                //oldLine = gen->old;
                oldDimension = *dimText;
                inf = gen->inf;
                sup = gen->sup;
                x=0;
            } else flag = 1;
        }
        if (redo_number - j != 0 ) flagUndoRedoPosition = 0 ;   //TODO forse non va bene questo controllo
        else flagUndoRedoPosition = 1;
    }


    if(redo_number < numRedo ) undo_number = undo_number + redo_number;
    else undo_number = undo_number + numRedo;
    if( numRedo > redo_number ) redo_number = 0;
    else redo_number = redo_number - numRedo;
    if( numRedo > undo_Index ) undo_Index = 0;
    else undo_Index = undo_Index - numRedo;
    if( flag == 0 ){
        lastValidCommand = gen->prec;
        currCommand = gen->prec;
        flagRedoDeleteCommand = 0;
    }
    else{
        lastValidCommand = gen;
        currCommand = gen;
        flagRedoDeleteCommand = 1;
    }
}

int comma( char* string){
    int i=0;
    while(1){
        if(string[i]==',') return i;
        i++;
    }
}

void undo_redo_sum ( genList *general, int numUndo, char **text, int *dimText, int *dimArray , FILE *fp , char* string ) {
    genList gen = *general;

    //backupUndo e backupRedo mi servono per vedere se sono state fatte "precedentemente" delle undo o redo e quindi se devo impostare i flag per
    //eliminare comandi vecchi
    int sumUndo = numUndo, sumRedo = 0, number = 0, print = 0 , backupUndo = undo_number, backupRedo = redo_number;
    char input[20] = "\0", *res = NULL, instr ='\0' ;
    if ( sumUndo > undo_number ) sumUndo = undo_number;
    while(1) {
        res = fgets(input, 20, fp);
        //printf("%s", input);
        if( input[0] != 'q' ) instr = input[strlen(input)-2];
        else instr = 'q';
        if(res==NULL) {
            printf("Errore di lettura\n");
            break;
        }
        if( instr == 'c' || instr == 'd' || instr == 'q' ){
            if( sumUndo > 0 && sumRedo == 0){
                //printf("numeroUndo %d", sumUndo);
                fflush(stdout);
                uFunction( general, sumUndo, text, dimText );
                sumUndo = 0;
                redo_number = 0;
                flagDelete = 1;
                flagUndoRedoPosition = 1;
                undo_Index = 0;
            }
            else if ( sumRedo > 0 && sumUndo == 0 ) {
                //printf("numeroRedo %d", sumRedo);
                rFunction( general, sumRedo, text, dimText, dimArray );
                sumRedo = 0;
                redo_number = 0;
                flagDelete = 1;
                flagUndoRedoPosition = 1;
                undo_Index = 0;
            }
            else if ( undo_number != backupUndo || redo_number != backupRedo ){ //se in passato ho fatto undo e redo imposto flag
                redo_number = 0;
                flagDelete = 1;
                flagUndoRedoPosition = 1;
                undo_Index = 0;
            }



            //lastValidCommand = gen;
            if( instr == 'q' ) string[0] = 'q';
            else strcpy(string,input);
            return;
        }
        if( instr == 'u' ){
            number = atoi(input);
            if ( number < sumRedo){
                sumRedo = sumRedo - number;
                number = 0;
            }
            else {  //caso in cui number > sumRedo
                number = number - sumRedo;
                sumRedo = 0;
            }
            sumUndo = sumUndo + number;
            if ( sumUndo > undo_number ) sumUndo = undo_number;
            continue;

Giuseppe Paolini, [11/12/2020 19:08]
}
        if( instr == 'r' ){
            number = atoi(input); //number mi da il numero di redo del comando corrente
            if ( number < sumUndo ) sumUndo = sumUndo - number; //caso senza nessuna print
            else {
                if( print == 1){ //caso con una print già effettuata
                    sumRedo = sumRedo + (-1)*(sumUndo - number);
                    if( sumRedo > redo_number ) sumRedo = redo_number;  //caso in cui eseguo il massimo numero di redo disponibile
                }
                sumUndo = 0;
            }
        }
        if( instr == 'p' ){ //eseguo print
            //flagDelete = 0;
            print = 1;
            if( sumUndo > 0 && sumRedo == 0){
                uFunction( general, sumUndo, text, dimText );
                sumUndo = 0;
            }
            if ( sumRedo > 0 && sumUndo == 0 ) {
                rFunction( general, sumRedo, text, dimText , dimArray);
                sumRedo = 0;
            }
            pFunction(atoi(input), atoi(input + comma(input) + 1), text , *dimText );
        }
    }
}

//TODO DOPO CREATECOMMAND COMMAND != LASTVALIDCOMMAND

genList createCommand ( genList *general, int inf, int sup, char instr ){
    genList newCommand, gen = *general, curr = lastValidCommand;

    // devo sovrascrivere i comandi già creati ricordati di modificare undo e redo per restituire puntatori alla gen corrente
    // ricorda che l'ho scritto in modo che il general che passo deve rimanere come comando, mentre gen->next deve essere
    // sovrascritto


    if( flagDelete == 1 && flagRedoDeleteCommand == 0) {
        if( undo_number != 0){
            gen = lastValidCommand->next;
            //if (lastValidCommand->next != NULL ) lastValidCommand->next = NULL;  //aggiunta
        }
        else {
            gen = lastValidCommand;
            //lastValidCommand = NULL;
            //lastValidCommand = lastValidCommand->prec;
            //lastValidCommand->next = NULL;//aggiunta
        }
        curr = gen->prec;
        //curr->next = NULL ;
                                      //ricorda sto assumendo che sia l'ultimo comando valido quindi vado con il next
                                    //forse gen->next = NULL
        /*while ( gen != NULL ){
            newCommand = gen;
            //int i = 0;
            if( gen->old != NULL){
                /*while(i < gen->diff){
                    if(gen->old[i] != NULL) {
                        free(gen->old[i]);
                        gen->old[i] = NULL;
                    }
                    i++;
                }*/
                /*free(gen->old);
                gen->old = NULL;    //da non mettere
            }
            gen->old = NULL;
            gen->prec = NULL;
            gen = gen->next;
            free(newCommand);
        }/*/
        flagDelete = 0;
        gen = curr;
        if (gen != NULL ) gen->next = NULL;
    }
    //else flagRedoDeleteCommand = 0 ;  //Caso in cui tutte le redo sono stato effettuate e non devo modificare i comandi ma solo aggiungerlo uno nuovo



      //fai le solite cose
    newCommand = malloc(sizeof(struct generalList));
    newCommand->old = NULL;
    newCommand->prec = NULL;
    newCommand->next = NULL;
    newCommand->inf = 0;
    newCommand->sup = 0;
    newCommand->instr = '\0';
    if(gen != NULL) {

        gen->next = newCommand;
        newCommand->prec = gen;
    }

    gen = newCommand;
    gen->inf = inf;
    gen->sup = sup;
    gen->instr = instr;
    newCommand = NULL;

    currCommand = gen;
    if ( undo_number == 0 ) lastValidCommand = gen;
    undo_number++;
    return  gen;
}

genList createCommandNull ( int inf, int sup, char instr){

    genList newCommand;

    undo_number++;
    newCommand = malloc(sizeof(struct generalList));
    newCommand->old = NULL;
    newCommand->prec = NULL;
    newCommand->next = NULL;
    newCommand->inf = inf;
    newCommand->sup = sup;
    newCommand->instr = instr;

    return newCommand;
}

///////////////////////////////////////// MAIN //////////////////////////////////////////////////////

Giuseppe Paolini, [11/12/2020 19:08]
int main() {


    int inf ,sup,inputLenght ;
    char input[1024] = "\0";
    char instr,  string[20] = "\0";
    char *res = NULL;
    int dimText = 0, dimArray = 1;
    char **text = NULL;
    genList currGeneral = NULL ;
    text = (char**)malloc(sizeof(char *)* 1);
    FILE *fp = NULL;
    fp = fopen("test.txt", "r");
    if(fp == NULL){
        printf("Errore nell'apertura del file");
        return 0;
    }
    string[0] = 'n';
    input[0] = '\0';
    //for (int i=0; i<10; i++){
    //  text[i] = malloc(sizeof(char) * 1024);
    //}

    while(1) {

        instr ;
        if(string[0] == 'n' ) res = fgets(input, 1024, fp);
        else {
            strcpy(input,string);
            string[0] = 'n';
        }

        instr = input[strlen(input)-2];
        if(input[0] == '.') continue;
        if(res==NULL) {
            printf("Errore di lettura\n");
            break;
        }
        //printf("%s", input);
        //fflush(stdout);
        if( instr == 'c' || instr == 'd' || instr == 'p' ){
            inf = atoi(input);
            sup = atoi(input + comma(input) + 1);
            if(sup == 0) {
                if(instr == 'p') {
                    pFunction( inf, sup , text , dimText);
                    continue;
                }
            }
            if (instr == 'p') {
                //flagDelete = 0;
                pFunction( inf, sup , text , dimText);
                continue;
            }
            if( inf == 0) inf = 1;
            if( currGeneral != NULL) {
                currGeneral = createCommand  (&currGeneral, inf , sup, instr);
                //newURPointedGeneral = currGeneral;
            }
            else currGeneral =  createCommandNull(inf, sup, instr);

        }
        if(instr == 'c'){
            text = cFunction(&currGeneral, text, &dimText, &dimArray, fp);
            continue;
        }
        if(instr == 'd'){
            text = dFunction(&currGeneral, text, &dimText, &dimArray);
            continue;
        }
        /*if(instr == 'p'){
            pFunction(inf, sup, text, dimText);
            continue;
        }*/


        if(instr == 'u'){
            inputLenght = atoi(input);
            if(inputLenght == 0) continue;
            undo_redo_sum(&currGeneral, inputLenght, text, &dimText,&dimArray , fp , string);
            if (string[0] == 'q') {
                /*for (int m = 0; m < dimArray; m++ ) {
                    if (text[m] != NULL) free(text[m]);
                    text[m] = NULL;
                }
                free(text);
                currGeneral = headGeneral;
                while (currGeneral != NULL){
                    do{
                        if(currGeneral->old != NULL){
                            if( currGeneral->old[x] != NULL ) free(currGeneral->old[x]);

                        }
                        x++;
                    } while (x < currGeneral->sup );
                    free(currGeneral->old);
                    headGeneral = currGeneral;
                    currGeneral = currGeneral->prec;
                    free(headGeneral);

                }
                */return 0;
            }
            continue;
        }

        if(input[0] == 'q'){
            /*for (int m = 0; m < dimArray; m++ ) {
                if (text[m] != NULL) free(text[m]);
                text[m] = NULL;
            }
            free(text);
            currGeneral = headGeneral;
            while (currGeneral != NULL){
                do{
                    if(currGeneral->old != NULL){
                        if( currGeneral->old[x] != NULL ) free(currGeneral->old[x]);
                        x++;
                    }
                } while (x < currGeneral->sup );
                free(currGeneral->old);
                headGeneral = currGeneral;
                currGeneral = currGeneral->prec;
                free(headGeneral);

            }
            //free(headGeneral);*/
            return 0;
        }
    }
    return 0;
}