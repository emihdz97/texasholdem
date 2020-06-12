/*
    Ernesto Ramirez A01019589
    Naji M A Saadat A01025599
    Emilio Hernández A01336418
    
    Texas hold'em server program

    Simulates shuffling a deck of cards 
    using structures and typedef 
    by initializing, shuffling, and displaying the card deck
    https://www2.hawaii.edu/~walbritt/ics212/examples/cards.c

    Program for a simple bank server
    It uses sockets and threads
    The server will process simple transactions on a limited number of accounts
    Gilberto Echeverria
    gilecheverria@yahoo.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
#include <stdbool.h>
// Posix threads library
#include <pthread.h>
#include <sys/socket.h>

// Custom libraries
#include "sockets.h"
#include "analyzer.h"


#define BUFFER_SIZE 1024
#define MAX_PLAYERS 3

// Global variables for signal handlers
int n_of_players = 0;
int controller = 0;
float pool = 0;
int in_game = MAX_PLAYERS;
int deck_pos = 0;
int active_p = 0;

//structure definition
//structure of a client
typedef struct client_struct{
    int id;
    int connection_fd;
    float balance;
    float debt;
    float in_pool;
    int active;
    int card1_pos;
    int card2_pos;
    int score;
}client;

char deck_strings  [52][16] = {
        "AceSpades",
        "AceHearts",
        "AceDimonds",
        "AceClubs",

        "KingSpades",
        "KingHearts",
        "KingDiamonds",
        "KingClubs",

        "QueenSpades",
        "QueenHearts",
        "QueenDiamonds",
        "QueenClubs",

        "JackSpades",
        "JackHearts",
        "JackDiamonds",
        "JackClubs",

        "TenSpades",
        "TenHearts",
        "TenDiamonds",
        "TenClubs",

        "NineSpades",
        "NineHearts",
        "NineDiamonds",
        "NineClubs",

        "EightSpades",
        "EightHearts",
        "EightDiamonds",
        "EightClubs",

        "SevenSpades",
        "SevenHearts",
        "SevenDiamonds",
        "SevenClubs",

        "SixSpades",
        "SixHearts",
        "SixDiamonds",
        "SixClubs",

        "FiveSpades",
        "FiveHearts",
        "FiveDiamonds",
        "FiveClubs",

        "FourSpades",
        "FourHearts",
        "FourDiamonds",
        "FourClubs",

        "ThreeSpades",
        "ThreeHearts",
        "ThreeDiamonds",
        "ThreeClubs",

        "TwoSpades",
        "TwoHearts",
        "TwoDiamonds",
        "TwoClubs"

};

//structure of a card
struct card{ 
  char *rank;    
  char suit[9];  
};
typedef struct card Card;

//array of pointers to strings for ranks
char *ranks[13] = {"Ace", "Two", "Three", "Four", "Five", "Six", "Seven", 
			  "Eight", "Nine", "Ten", "Jack", "Queen", "King"};

//two-dimensional array of strings for suits
char suits[4][9] = {"Clubs", "Diamonds", "Hearts", "Spades"};

///// FUNCTION DECLARATIONS
void usage(char * program);
void waitForConnections(int server_fd, client * players_Data);
char* setup_deck();
void* attentionThread(void * arg);
void initialize(Card []);
void shuffle(Card []);
void quit(client *players_Data);
void check();
void pay(client * players_Data);
void push(client * players_Data, float push_amount);
void all_in(client * players_Data);
void gambling(client * players_Data);
void operation_case(client * players_Data, int operation, float push_amount);
void send_players_cards(const Card deck[], client * players_Data);
bool ready(client * players_Data);
void players_table(client * players_Data);
void three_cards_table(const Card deck[], client * players_Data);
void one_card(const Card deck[], client * players_Data);
void revel_cards(const Card deck[], client * players_Data);
void analizer_preparation(const Card deck[], client * players_Data, int playrerNum);
int winnig(const Card deck[], client * players_Data);
void repartition(int win, client * players_Data);
void free_memory(client * players_Data);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int server_fd;
    int win = 0;
    client * players_Data;

    printf("\n=== SIMPLE TEXAS SERVER ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

	// Show the IPs assigned to this computer
	printLocalIPs();

    // Start the server
    server_fd = initServer(argv[1], 3);
    
    // Listen for connections from the clients
    players_Data = malloc (3 * sizeof(client)); 
    waitForConnections(server_fd, players_Data);

    //Send the balances to all the players
    players_table(players_Data);

    //declare an array of 52 cards
    Card deck[52] = {"",""};
    initialize(deck);
    printf("\nshuffling deck ... \n");
    shuffle(deck);

    //Send the cards to the clients
    send_players_cards(deck, players_Data);

    //Time to put some money
    gambling(players_Data);

    //Three cards on table
    three_cards_table(deck, players_Data);

    //Second gambling round
    gambling(players_Data);

    //One more Card
    one_card(deck, players_Data);

    //Third gambling round
    gambling(players_Data);

    //The las card is opened
    one_card(deck, players_Data);

    //Last round for gamble
    gambling(players_Data);

    //Reveal teh other players hands
    revel_cards(deck, players_Data);   

    //Who win
    win = winnig(deck, players_Data);
    
    //Money distribution
    repartition(win, players_Data);
    
    // Close the socket
    close(server_fd);

    // Clean the memory used
    free_memory(players_Data);

    // Finish the main thread
    pthread_exit(NULL);

    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}

/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd, client * players_Data)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pthread_t new_tid;
    bool end = false;

    // Get the size of the structure to store client information
    client_address_size = sizeof client_address;
    while (end == false)
    {
        if (n_of_players < MAX_PLAYERS - 1)
        {
            client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
            if (client_fd == -1)
            {
                perror("ERROR: accept");
            }

            // Get the data from the client
            inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
            printf("Received incomming connection from %s on port %d\n", client_presentation, client_address.sin_port);

            //Prepare the structue of the new cliente
            players_Data[n_of_players].id = n_of_players+1;
            players_Data[n_of_players].connection_fd = client_fd;
            players_Data[n_of_players].balance = 0.0;
            players_Data[n_of_players].debt = 0.0;
            players_Data[n_of_players].in_pool= 0.0;
            players_Data[n_of_players].active = 0;
            players_Data[n_of_players].card1_pos = 0;
            players_Data[n_of_players].card2_pos = 0;
            players_Data[n_of_players].score = 0;
            //printf("Actual cliente fd: %d\n", client_fd);
            //printf("Actual client fd saved: %d\n", players_Data[n_of_players].connection_fd);
            //printf("Actual cliente id: %d\n", players_Data[n_of_players].id);

            //Create a thrad to recive the start amount of balance for each client
            pthread_create(&new_tid, NULL, attentionThread, (void *) players_Data);
        }
        end = ready(players_Data);
    }
    printf("Lets start the game, players are ready\n");
}

/*
    Hear the request from the client and send an answer
*/
void * attentionThread(void * arg)
{
    //Receive the balance of the current cliente
    char buffer[BUFFER_SIZE];
    client * temp_client = arg;
    int tester;
    int current_id = n_of_players;
    n_of_players += 1;

    //printf("This is the cliente fd: %d\n", temp_client->players_array[temp_client->id].connection_fd);
    tester = recv(temp_client[current_id].connection_fd, buffer, BUFFER_SIZE, 0);
    if (tester <= 0)
    {
        perror("Error in the recv\n");
    }
        
    sscanf(buffer, "%f", &temp_client[current_id].balance);
    //printf("The current data of the player number: %d\nConnection fd number: %d\nCurrent balance %f\n", temp_client[current_id].id, temp_client[current_id].connection_fd, temp_client[current_id].balance);
    pthread_exit(NULL);
}

/*
  initialize the deck of cards to string values
  deck: an array of structure cards 
*/
void initialize(Card deck[]){
  int i = 0;
  for(i=0;i<52;i++){
    deck[i].rank = ranks[i%13];
    strncpy(deck[i].suit, suits[i/13], 9);
  }
}

/*
  use the pseudo-random number generator to shuffle the cards
  deck: an array of structure cards 
*/
void shuffle(Card deck[]){
  int swapper = 0; //index of card to be swapped
  Card temp = {"", ""}; //temp holding place for swap
  srand(time(NULL)); //seed the random numbers with current time
  for(int i=0;i<52;i++){
    //generate a pseudo-random number from 0 to 51
    swapper = rand() % 52; 
    //swap current card with da swapper
    temp = deck[i];
    deck[i] = deck[swapper];
    deck[swapper] = temp;
  }
}

//quit action
void quit(client *players_Data)
{
    //printf("Quit option\n");
    controller += 1;
    players_Data[active_p].active = 1;
    players_Data[active_p].debt = 0.0;
    players_Data[active_p].in_pool = 0.0;
    in_game -= 1;
}

void check()
{
    //printf("Check option\n");
    controller += 1;
}

//pay action
void pay(client * players_Data)
{
    //printf("Pay option\n");
    float amount = 0;
    amount += players_Data[active_p].debt;
    players_Data[active_p].debt = 0;
    players_Data[active_p].balance -= amount;
    players_Data[active_p].in_pool += amount;
    pool += amount;
    controller += 1;
}

//push action
void push(client * players_Data, float push_amount)
{
    //printf("Push option \n");
    float amount = 0;
    amount += players_Data[active_p].debt;
    players_Data[active_p].debt = 0;
    amount += push_amount;
    players_Data[active_p].in_pool += amount;
    players_Data[active_p].balance -= amount;
    pool += amount;
    controller = 1;
}

//all in action
void all_in(client * players_Data)
{
    //printf("All in option\n");
    players_Data[active_p].in_pool += players_Data[active_p].balance;
    pool += players_Data[active_p].balance;
    players_Data[active_p].balance = 0;
    players_Data[active_p].debt = 0;
    players_Data[active_p].active = 3;

}

//Handle the gamble round
void gambling(client * players_Data)
{
    char buffer[BUFFER_SIZE];
    int operation = 0;
    float push_amount = 0;

    while (controller <= MAX_PLAYERS - 1 && in_game != 1)
    {
        if(0 < players_Data[active_p].active)
        {
            controller += 1;
        }
        else
        {
            sprintf(buffer,"%d %d", 0, active_p);
            send(players_Data[active_p].connection_fd, buffer, BUFFER_SIZE, 0);
            recv(players_Data[active_p].connection_fd, buffer, BUFFER_SIZE, 0);
            
            float debt_acum = 0;
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (debt_acum < players_Data[i].in_pool)
                {
                    debt_acum = players_Data[i].in_pool;
                }
            }
            players_Data[active_p].debt = debt_acum - players_Data[active_p].in_pool;
            
            sprintf(buffer, "%f %f", players_Data[active_p].balance, players_Data[active_p].debt);
            send(players_Data[active_p].connection_fd, buffer, BUFFER_SIZE, 0);
            recv(players_Data[active_p].connection_fd, buffer, BUFFER_SIZE, 0);
            sscanf(buffer, "%d %f", &operation, &push_amount);
            operation_case(players_Data, operation, push_amount);
        }
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if(active_p != i)
            {
                sprintf(buffer,"%d %d", 1, active_p);
                send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
                recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
                sprintf(buffer, "%d %f", operation, push_amount);
                send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
                recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
            }
        }
        active_p += 1;
        //printf("Active_p number: %d\n", active_p);
        if (active_p > 2)
        {
            
            active_p = 0;
        }
        
    }
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        sprintf(buffer, "%d", 3);
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
    }
    
    controller = 0;
    if (active_p > 2)
    {        
        active_p = 0;
    }
}

//Calls the corresponding function acording to the selected action
void operation_case(client * players_Data, int operation, float push_amount)
{
    switch (operation)
    {
    case 0:
        quit(players_Data);
        break;
    
    case 1:
        check();
        break;
    
    case 2:
        pay(players_Data);
        break;
    
    case 3:
        push(players_Data, push_amount);
        break;

    case 4:
        all_in(players_Data);
        break;

    }
}

//Send to the players their first 2 cards
void send_players_cards(const Card deck[], client * players_Data)
{
    char buffer[BUFFER_SIZE];
    
    players_Data[0].card1_pos = 0;
    //printf("Player %d Card1_pos: %d\n", players_Data[0].id, players_Data[0].card1_pos);
    //printf("The card is: %s of %s\n", deck[players_Data[0].card1_pos].rank, deck[players_Data[0].card1_pos].suit);
    players_Data[0].card2_pos = 1;
    //printf("Player %d Card1_pos: %d\n", players_Data[0].id, players_Data[0].card2_pos);
    //printf("The card is: %s of %s\n", deck[players_Data[0].card2_pos].rank, deck[players_Data[0].card2_pos].suit);
    players_Data[1].card1_pos = 2;
    //printf("Player %d Card1_pos: %d\n", players_Data[1].id, players_Data[1].card1_pos);
    //printf("The card is: %s of %s\n", deck[players_Data[1].card1_pos].rank, deck[players_Data[1].card1_pos].suit);
    players_Data[1].card2_pos = 3;
    //printf("Player %d Card1_pos: %d\n", players_Data[1].id, players_Data[1].card2_pos);
    //printf("The card is: %s of %s\n", deck[players_Data[1].card2_pos].rank, deck[players_Data[1].card2_pos].suit);
    players_Data[2].card1_pos = 4;
    //printf("Player %d Card1_pos: %d\n", players_Data[2].id, players_Data[2].card1_pos);
    //printf("The card is: %s of %s\n", deck[players_Data[2].card1_pos].rank, deck[players_Data[2].card1_pos].suit);
    players_Data[2].card2_pos = 5;
    //printf("Player %d Card1_pos: %d\n", players_Data[2].id, players_Data[2].card2_pos);
    //printf("The card is: %s of %s\n", deck[players_Data[2].card2_pos].rank, deck[players_Data[2].card2_pos].suit);
    
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        sprintf(buffer, "%s %s", deck[players_Data[i].card1_pos].rank, deck[players_Data[i].card1_pos].suit);
        
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
    }
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        sprintf(buffer, "%s %s", deck[players_Data[i].card2_pos].rank, deck[players_Data[i].card2_pos].suit);
        
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
    }

    printf("Everybody have they cards\n");
}

//Returns true when all the players have a valid balance for starting the game
bool ready(client * player_Data)
{
    bool cond = true;
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (player_Data[i].balance == 0)
        {
            cond = false;
        }
    }
    return cond;
}

//Once the 3 players are connected send to all the players the balance of each player
void players_table(client * players_Data)
{
    char buffer[BUFFER_SIZE];

    //for (int i = 0; i < MAX_PLAYERS; i++)
    //{
        //printf("\nPlayer id: %d\n", players_Data[i].id);
        //printf("Player connection_id: %d\n", players_Data[i].connection_fd);
        //printf("Player balance : %f\n", players_Data[i].balance);
        //printf("Player debt: %f\n", players_Data[i].debt);
        //printf("Player balance : %f\n", players_Data[i].in_pool);
        //printf("Player balance : %d\n", players_Data[i].active);
        //printf("Player card1: %d\n", players_Data[i].card1_pos);
        //printf("Player card2: %d\n", players_Data[i].card2_pos);
        //printf("Player score: %d\n", players_Data[i].score);
    //}
    
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        sprintf(buffer, "%f %f %f", players_Data[0].balance, players_Data[1].balance, players_Data[2].balance);
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
    }
    
}

//Send to all the players the first 3 open cards
void three_cards_table(const Card deck[], client * players_Data)
{
    char buffer[BUFFER_SIZE];

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        for (int j = 6; j < 9; j++)
        {
            sprintf(buffer, "%s %s", deck[j].rank, deck[j].suit);
            send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
            recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0); 
        }
    }
}

//Send one card to all the palyers
void one_card(const Card deck[], client * players_Data)
{
    char buffer[BUFFER_SIZE];

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        sprintf(buffer, "%s %s", deck[9].rank, deck[9].suit);
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
    }
    
}

//Send to all players all the players cards
void revel_cards(const Card deck[], client * players_Data)
{
    char buffer[BUFFER_SIZE];

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        sprintf(buffer, "%d %s %s", players_Data[0].id, deck[players_Data[0].card1_pos].rank, deck[players_Data[0].card1_pos].suit);
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        sprintf(buffer, "%s %s", deck[players_Data[0].card2_pos].rank, deck[players_Data[0].card2_pos].suit);
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);

        sprintf(buffer, "%d %s %s", players_Data[1].id, deck[players_Data[1].card1_pos].rank, deck[players_Data[1].card1_pos].suit);
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        sprintf(buffer, "%s %s", deck[players_Data[1].card2_pos].rank, deck[players_Data[1].card2_pos].suit);
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);

        sprintf(buffer, "%d %s %s", players_Data[2].id, deck[players_Data[2].card1_pos].rank, deck[players_Data[2].card1_pos].suit);
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        sprintf(buffer, "%s %s", deck[players_Data[2].card2_pos].rank, deck[players_Data[2].card2_pos].suit);
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
    }
}   

//Call a c++ poker hand analizer for texas
void analizer_preparation(const Card deck[], client * players_Data, int playrerNum)
{ 


    char cards_arr[7][16];

    for(int i=0;i<5;i++){
        strcpy(cards_arr[i], deck[6+i].rank);
        strcat(cards_arr[i], deck[6+i].suit);

    }
    
    
    strcpy(cards_arr[5], deck[players_Data[playrerNum].card1_pos].rank);
    strcat(cards_arr[5], deck[players_Data[playrerNum].card1_pos].suit);

    strcpy(cards_arr[6], deck[players_Data[playrerNum].card2_pos].rank);
    strcat(cards_arr[6], deck[players_Data[playrerNum].card2_pos].suit);

    //for(int i=0;i<7;i++){
    //    printf("card: %s\n",cards_arr[i] );
//
    //} 

    //array of the players hand in numbers
    int cards_num_arr [7];


    //assign number from string
    for (int i = 0; i < 7; ++i){
        for(int j=0 ;j <52;j++){

            if(!strcmp(cards_arr[i], deck_strings[j])){
                cards_num_arr[i]=j;
                //printf("%i\n",cards_num_arr[i] );
                break;
            }

        }
    
    } 

    //for(int i=0;i<7;i++){
    //    printf("card number: %d\n",cards_num_arr[i] );
    //}   
        //Uso
    int score = getScore(cards_num_arr[0],cards_num_arr[1],cards_num_arr[2],cards_num_arr[3],
        cards_num_arr[4],cards_num_arr[5],cards_num_arr[6]);

    //printf(score);
    //printf("%i\n", score );

    players_Data[playrerNum].score = score;

    //enum cards current_card=AceSpades;

    //printf("Value of TenSpades= %d\n",current_card );
}

//Find the winner player 
int winnig(const Card deck[], client * players_Data)
{
    if (players_Data[0].active != 1)
    {
        analizer_preparation(deck, players_Data, 0);
    }
    
    //printf("Player 1\nScore: %d\n", players_Data[0].score);

    if (players_Data[1].active != 1)
    {
        analizer_preparation(deck, players_Data, 1);
    }
    
    //printf("Player 2\nScore: %d\n", players_Data[1].score);

    if (players_Data[2].active != 1)
    {
        analizer_preparation(deck, players_Data, 2);
    }
    
    //printf("Player 3\nScore: %d\n", players_Data[2].score);

    int scores [3]={
            players_Data[0].score,
            players_Data[1].score,
            players_Data[2].score
    };


    
    if(scores[0]>scores[1] && scores[0]>scores[2]){
        printf("Wins player 1\n");
        return 0;
    }else 

    if(scores[1]>scores[0] && scores[1]>scores[2]){
        printf("Wins player 2\n");
        return 1;
    }else 

    if(scores[2]>scores[0] && scores[2]>scores[1]){
        printf("Wins player 3\n");
        return 2;
    }else{
        return -1;
    } 
}

//Update the players balance acording to the winner and send it to the client
void repartition(int win, client * players_Data)
{
    char buffer[BUFFER_SIZE];
    players_Data[win].balance += players_Data[win].in_pool;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (i != win)
        {
            if (players_Data[i].in_pool <= players_Data[win].in_pool)
            {
                players_Data[win].balance += players_Data[i].in_pool;
                players_Data[i].in_pool = 0;
            }
            else
            {
                players_Data[win].balance += players_Data[win].in_pool;
                players_Data[i].in_pool -= players_Data[win].in_pool;
            }
            if (players_Data[i].in_pool != 0)
            {
                players_Data[i].balance += players_Data[i].in_pool;
                players_Data[i].in_pool = 0;
            }            
        }
    }
    
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        //printf("Player: %d\nBalance: %f\n", i, players_Data[i].balance);
        sprintf(buffer, "%d %f", win, players_Data[i].balance);
        send(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
        recv(players_Data[i].connection_fd, buffer, BUFFER_SIZE, 0);
    }
    
}

//Free the memony that had been locked
void free_memory(client * players_Data)
{
    free(players_Data);
}