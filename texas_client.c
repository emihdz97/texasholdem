/*
    Client program to access the accounts in the bank
    This program connects to the server using sockets
    Gilberto Echeverria
    gilecheverria@yahoo.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
// Custom libraries
#include "sockets.h"

#define BUFFER_SIZE 1024

///// FUNCTION DECLARATIONS
void usage(char * program);
void balance_scan(int connection_fd);
void handler(int connection_fd);
void options(float balance, float debt, int connection_fd);
float validation(float balance, float debt, float push_amount);
void operation(char action, float push_amount, int connection_fd);
void players_info(int connection_fd);
void my_hand(int connection_fd);
void three_cards(int connection_fd);
void one_card(int connection_fd);
void hand_reveals(int connection_fd);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int connection_fd;

    printf("\n=== TEXAS CLIENT PROGRAM ===\n");

    // Check the correct arguments
    if (argc != 3)
    {
        usage(argv[0]);
    }

    // Start the server
    connection_fd = connectSocket(argv[1], argv[2]);

    //Send the current balance that the cliente will put on the table
    balance_scan(connection_fd);

    //Check all the table situation
    players_info(connection_fd);
    
    //Recive and print your cards
    my_hand(connection_fd);

    //Gamble turn or listen
    handler(connection_fd);

    //Three cards at the center
    three_cards(connection_fd);

    //Second Gamble round
    handler(connection_fd);

    //Recibe one more card
    one_card(connection_fd);

    //Third Gamble round
    handler(connection_fd);

    //Recibe one more card
    one_card(connection_fd);

    //Last Gamble round
    handler(connection_fd);

    //Reveal players hands
    hand_reveals(connection_fd);


    // Close the socket
    close(connection_fd);

    return 0;
}

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {server_address} {port_number}\n", program);
    exit(EXIT_FAILURE);
}

void handler(int connection_fd)
{
    char buffer[BUFFER_SIZE];

    int action_in = 0;
    int player_in_turn;
    
    //struct pollfd test_fds[1];
    //test_fds[0].fd = connection_fd;
    //test_fds[0].events = POLLIN;
    
    while (action_in != 3)
    {
        recv(connection_fd, buffer, BUFFER_SIZE, 0);
        sscanf(buffer, "%d %d", &action_in, &player_in_turn);
        if (action_in == 0)
        {
            printf("\nYour turn.\n");
            sprintf(buffer, "OK");
            send(connection_fd, buffer, BUFFER_SIZE, 0);

            float balance, debt;

            recv(connection_fd, buffer, BUFFER_SIZE, 0);
            sscanf(buffer, "%f %f", &balance, &debt);
            printf("Your current balance is: %f\nAnd at least you need to pay: %f\n", balance, debt);
            options(balance, debt, connection_fd);
            printf("End of your turn\n");
        }
        else if (action_in == 1)
        {
            float push_amount = 0;
            int oper = 0;
            printf("\nTurn of player: %d\n", player_in_turn + 1);
            sprintf(buffer, "OK");
            send(connection_fd, buffer, BUFFER_SIZE, 0);
            recv(connection_fd, buffer, BUFFER_SIZE, 0);
            sscanf(buffer, "%d %f", &oper, &push_amount);
            if (oper == 0)
            {
                printf("Player: %d\nLeave the round.\n", player_in_turn + 1);
            }
            else if (oper == 1)
            {
                printf("Player: %d\nCheck.\n", player_in_turn + 1);
            }
            else if (oper == 2)
            {
                printf("Player: %d\nPay the push.\n", player_in_turn + 1);
            }
            else if (oper == 3)
            {
                printf("Player: %d\nPush: %f\n", player_in_turn, push_amount + 1);
            }
            else if(oper == 4)
            {
                printf("Player: %d\nAll in.\n", player_in_turn + 1);
            }
            sprintf(buffer, "OK");
            send(connection_fd, buffer, BUFFER_SIZE, 0);
        }
        
    }
    printf("\nEnd of the gambling round\n");
    sprintf(buffer, "OK");
    send(connection_fd, buffer, BUFFER_SIZE, 0);
}

//The user send the amount of money that he/her will have at the start of the game
void balance_scan(int connection_fd)
{
    char buffer[BUFFER_SIZE];
    float balance;
    
    printf("Please insert the amount of chips you will put on the table\n");
    scanf("%f", &balance);
    sprintf(buffer, "%f", balance);
    send(connection_fd, buffer, BUFFER_SIZE, 0);
    printf("Waiting for the other players\n");
}

void options(float balance, float debt, int connection_fd)
{
    char action;
    float push_amount;

    if (debt == 0)
    {
        while (1)
        {
            printf("Select one of the following options:\n");
            printf("b. Check\n");
            printf("d. Push\n");
            printf("e. All-in\n");    
            scanf("%c", &action);
            if (action == 'b' || action == 'e')
            {
                operation(action, push_amount, connection_fd);
                break;
            }
            else if (action == 'd')
            {
                printf("Please put the amount you want to push.\n");
                scanf("%f", &push_amount);
                push_amount = validation(balance, debt, push_amount);
                operation(action, push_amount, connection_fd);
                break;
            }
            else
            {
                printf("That is not an option, try again. First condition\n");
            }
        }
    }
    else if (0 < debt && debt < balance)
    {
        while (1)
        {
            printf("Select one of the following options:\n");
            printf("a. Leave\n");
            printf("c. Pay\n");
            printf("d. Push\n");
            printf("e. All-in\n");    
            
            scanf("%c", &action);
            if (action == 'a' || action == 'c' || action == 'e')
            {
                operation(action, push_amount, connection_fd);
                break;
            }
            else if (action == 'd')
            {
                printf("Please put the amount you want to push.\n");
                scanf("%f", &push_amount);
                push_amount = validation(balance, debt, push_amount);
                operation(action, push_amount, connection_fd);
                break;
            }
            else
            {
                printf("That is not an option, try again. Second condition\n");
            }
        }
    }
    else if (0 < debt && debt >= balance)
    {
        while (1)
        {
            printf("Select one of the following options:\n");
            printf("a. Leave\n");
            printf("e. All-in\n");    
            
            scanf("%c", &action);
            if (action == 'a' || action == 'e')
            {
                operation(action, push_amount, connection_fd);
                break;
            }
            else
            {
                printf("That is not an option, try again.\n");
            }       
        }
    }
}

float validation(float balance, float debt, float push_amount)
{
    float amount_push = push_amount;
    float total = balance + debt;

    while (1)
    {
        if (amount_push < total)
        {
            printf("Valid push\n");
            return amount_push;
        }
        else
        {
            printf("Insuficient fonds, you actually have: %f\nAnd for complete the push you need to pay: %f\n", balance, total);
            printf("Please put the amount you want to push.\n");
            amount_push = 0;
            scanf("%f", &amount_push);
        }
    }
}

void operation(char action, float push_amount, int connection_fd)
{
    char buffer[BUFFER_SIZE];

    switch (action)
    {
    case 'a':
        sprintf(buffer, "%d %f", 0, 0.0);
        send(connection_fd, buffer, BUFFER_SIZE, 0);
        break;
    case 'b':
        sprintf(buffer, "%d %f", 1, 0.0);
        send(connection_fd, buffer, BUFFER_SIZE, 0);
        break;
    case 'c':
        sprintf(buffer, "%d %f", 2, 0.0);
        send(connection_fd, buffer, BUFFER_SIZE, 0);   
        break;
    case 'd':
        sprintf(buffer, "%d %f", 3, push_amount);
        send(connection_fd, buffer, BUFFER_SIZE, 0);
        break;
    case 'e':
        sprintf(buffer, "%d %f", 4, 0.0);
        send(connection_fd, buffer, BUFFER_SIZE, 0);
        break;
    }
}

void players_info(int connection_fd)
{
    float p1B, p2B, p3B;
    char buffer[BUFFER_SIZE];

    recv(connection_fd, buffer, BUFFER_SIZE, 0);
    sscanf(buffer, "%f %f %f", &p1B, &p2B, &p3B);
    printf("\nThe table is ready.\nThis are the players:\nPlayer 1\nBalance: %f\nPlayer 2\nBalance: %f\nPlayer 3\nBalance: %f\n", p1B, p2B, p3B);
    sprintf(buffer, "OK");
    send(connection_fd, buffer, BUFFER_SIZE, 0);
}

void my_hand(int connection_fd)
{
    char card1[20];
    char card2[20];
    char buffer[BUFFER_SIZE];

    printf("Your cards: \n");
    for (int i = 0; i < 2; i++)
    {
        recv(connection_fd, buffer, BUFFER_SIZE, 0);
        sscanf(buffer, "%s %s", card1, card2);
        printf("%s of %s\n", card1, card2);
        sprintf(buffer, "OK");
        send(connection_fd, buffer, BUFFER_SIZE, 0);
    }   
}

void three_cards(int connection_fd)
{
    char card1[20];
    char card2[20];
    char buffer[BUFFER_SIZE];

    printf("\nThe next three cards are going to get open for everyone.\n");
    for (int i = 0; i < 3; i++)
    {
        recv(connection_fd, buffer, BUFFER_SIZE, 0);
        sscanf(buffer, "%s %s", card1, card2);
        printf("%s of %s\n", card1, card2);
        sprintf(buffer, "OK");
        send(connection_fd, buffer, BUFFER_SIZE, 0);
        
    }
    
}

void one_card(int connection_fd)
{
    char card1[20];
    char card2[20];
    char buffer[BUFFER_SIZE];

    printf("\nOne more card is going to get open for everyone.\n");
    recv(connection_fd, buffer, BUFFER_SIZE, 0);
    sscanf(buffer, "%s %s", card1, card2);
    printf("%s of %s\n", card1, card2);
    sprintf(buffer, "OK");
    send(connection_fd, buffer, BUFFER_SIZE, 0);
}

void hand_reveals(int connection_fd)
{
    char card1[20];
    char card2[20];
    int id;
    char buffer[BUFFER_SIZE];

    for (int i = 0; i < 3; i++)
    {
        recv(connection_fd, buffer, BUFFER_SIZE, 0);
        sscanf(buffer, "%d %s %s", &id, card1, card2);
        printf("Player %d\n%s of %s\n", id, card1, card2);
        sprintf(buffer, "OK");
        send(connection_fd, buffer, BUFFER_SIZE, 0);
        
        recv(connection_fd, buffer, BUFFER_SIZE, 0);
        sscanf(buffer, "%s %s", card1, card2);
        printf("%s of %s\n", card1, card2);
        sprintf(buffer, "OK");
        send(connection_fd, buffer, BUFFER_SIZE, 0);
    }
}