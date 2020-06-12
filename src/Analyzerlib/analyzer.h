
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void writeCommand(int i, int j, int k, int n, int m, int p, int q){

    char card1[5];
    char card2[5];
    char card3[5];
    char card4[5];
    char card5[5];
    char card6[5];
    char card7[5];

    sprintf(card1,"%i", i);
    sprintf(card2,"%i", j);
    sprintf(card3,"%i", k);
    sprintf(card4,"%i", m);
    sprintf(card5,"%i", n);
    sprintf(card6,"%i", p);
    sprintf(card7,"%i", q);

    char command[50];

    strcpy(command, "./analyzer.o ");
    strcat(command, card1);
    strcat(command, " ");
    strcat(command, card2);
    strcat(command, " ");
    strcat(command, card3);
    strcat(command, " ");
    strcat(command, card4);
    strcat(command, " ");
    strcat(command, card5);
    strcat(command, " ");
    strcat(command, card6);
    strcat(command, " ");
    strcat(command, card7);
    strcat(command, " >file_output.txt");

    system(command);
}


int readFromFile(){

    char string[10];
    int num;
    FILE * fp;
    fp = fopen ("file_output.txt", "r");

    fgets(string, 10, fp);
    //printf("from file: %s\n", string);
    
    num = atoi(string);
    //printf("from file int: %i\n", num);

    return num;

    

    fclose(fp);

}

int getScore(int i, int j, int k, int n, int m, int p, int q){
    writeCommand(i, j, k, m, n, p, q);

    int score = readFromFile();

    printf("score: %i\n",score);
    return score;
}


