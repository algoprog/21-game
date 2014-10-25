#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define MAX_CARDS 5
#define MAX_PLAYERS 10
#define AI_NAME "Computer"

/*
  ______ ____
 |__    |_   |   .-----.---.-.--------.-----.
 |    __|_|  |_  |  _  |  _  |        |  -__|
 |______|______| |___  |___._|__|__|__|_____|
                 |_____|

 21 card game v. 1.0

 Developed by Chris Samarinas
 12/22/2013

*/

typedef struct{
    int card;
    int type;
} card;

//Player data
typedef struct{
    card *cards;
    char *name;
    char is_out;
    int money;
    int bet;
    int score;
    int hits;
    int aces;
} player;

int get_random(int from, int to);
int bet_ai(player *players, int pot, int difficulty, int ai_count[], int id);

char play_ai(player *players, int difficulty, int ai_count[], int id);
char name_taken(char *name, player *players, int added);

void generate_cards(card game_cards[]);
void give_card(player *players, card game_cards[], int cards_played, int ai_count[], int id);
void remove_card(int card_id, card game_cards[], int cards_played);
void show_hand(card *cards, int hits);
void end_turn(player *players, int id);
void pause(int i);
void top_ui();

int main()
{
    int players_count = 0, difficulty = 0, i, added = 0, dealer, sets = 0, temp_win, init_pot, stop, out_count = 0, ai_count[2], cards_played = 0, pot, start_money = 0;
    card game_cards[32];
    player *players;
    char name[31], check, round2_check = 0;

    top_ui(); //display top ui
    srand(time(NULL)); //randomize numbers

    ai_count[0] = 0; //low cards count
    ai_count[1] = 0; //high cards count

    printf("\n");

    //Get the number of players
    while(players_count>MAX_PLAYERS||players_count<1){
        printf("\n - Give number of human players <= %d: ", MAX_PLAYERS);
        fflush(stdin);
        scanf("%d", &players_count);
    }
    players_count += 1;
    players = (player *)calloc(players_count, sizeof(player));

    //Get start money
    while(start_money<=0){
        printf("\n - Give start money: ");
        fflush(stdin);
        scanf("%d",&start_money);
    }

    //Initialize money amount, hands and get players names
    for(i=0;i<players_count;i++){
        players[i].money = start_money;
        players[i].cards = (card *)calloc(MAX_CARDS, sizeof(card));
        if(i>0){ //if not the computer
            printf("\n - Player %d name: ", i);
            fflush(stdin);
            gets(name);
            while(name_taken(name, players, added)||strcmp(name, AI_NAME)==0){ //if name is already taken
                printf("\n - Enter a different name for Player %d: ", i);
                fflush(stdin);
                gets(name);
            }
            players[i].name = (char *)malloc((strlen(name)+1)*sizeof(char));
            strcpy(players[i].name,name);
            added++;
        }else players[i].name = AI_NAME;
    }

    //Get difficulty level
    printf("\n - Choose difficulty level (1:Easy, 2:Medium, 3:Hard): ");
    while(difficulty!=1&&difficulty!=2&&difficulty!=3){
        fflush(stdin);
        scanf("%d", &difficulty);
    }

    system("cls");
    top_ui();
    generate_cards(game_cards); //create all the available cards

    dealer = get_random(0, players_count-1); //choose a random dealer

    while(out_count<players_count-1){ //Start a set if there are at least 2 active players...

        system("cls");
        top_ui();

        //Find the next active player to become dealer
        if(dealer>players_count-1)
            dealer = 0;
        while(players[dealer].is_out==1){
            dealer++;
            if(dealer>players_count-1)
                dealer = 0;
        }
        if(round2_check==0){ //if we go for a second round
            sets++;
            pot = start_money / 5; //Add 20% of initial dealer's money to pot
            if(players[dealer].money<pot)
                pot = players[dealer].money;

            players[dealer].money = players[dealer].money - pot;
            init_pot = pot; //save the initial pot for later
            printf("\n\n - %s is the dealer for set %d!", players[dealer].name, sets);
        }
        else
            printf("\n\n - [%s]: Get prepared for set %d round 2!", players[dealer].name, sets);

        pause(1);

        for(i=0;i<players_count&&pot>0;i++){ //for each player, if the pot has money
            check = 'h';
            //Find the next active player if this is inactive
            if(players[i].is_out==1&&i==players_count-1)
                continue;
            while(players[i].is_out==1){
                i++;
                if(i>players_count-1)
                    i = 0;
            }
            if(i!=dealer){ //if not the dealer
                printf("\n\n * [%s]: %s, it's your turn!",players[dealer].name, players[i].name);
                pause(i);
                printf("\n");
                temp_win = -1;
                while(check == 'h'){ //if wants new hit
                    players[i].hits++; //increment player hits

                    if(cards_played==31){
                        generate_cards(game_cards);
                        cards_played = 0;
                        ai_count[0] = 0;
                        ai_count[1] = 0;
                    }

                    give_card(players, game_cards, cards_played, ai_count, i); //give card
                    cards_played++; //increment played cards number
                    if(i==0||(players[i].hits==1&&i>0))
                        printf("\n");
                    printf(" - [%s]: Your hand:", players[dealer].name);
                    show_hand(players[i].cards, players[i].hits);
                    pause(0);
                    if(players[i].hits==1){ //Request money for bet
                        printf("\n - [%s]: Your card value is %d.\n", players[dealer].name, players[i].score);
                        pause(0);
                        if(i==0){ //if player is ai
                            printf(" - [%s]: You have $%d. Please place your bet <= %d: ", players[dealer].name, players[i].money, pot);
                            players[i].bet = bet_ai(players, pot, difficulty, ai_count, i);
                            pause(0);
                            printf("%d", players[i].bet);
                        }else{
                            while(players[i].bet<=0||players[i].bet>pot||players[i].bet>players[i].money){
                                printf(" - [%s]: You have $%d. Please place your bet <= %d: ", players[dealer].name, players[i].money, pot);
                                fflush(stdin);
                                scanf("%d", &players[i].bet);
                            }
                        }
                    }
                    if(players[i].hits==MAX_CARDS && players[i].score<21){
                        check = 's';
                        printf("\n - [%s]: You cannot take more than 5 cards.", players[dealer].name);
                        pause(i);
                    }
                    else if(players[i].score==21){
                        check = 's';
                        printf("\n - [%s]: Your cards sum is 21. You win $%d.", players[dealer].name, players[i].bet);
                        temp_win = i;
                        pause(i);
                    }
                    else if(players[i].aces==2 && players[i].hits==2){
                        check = 's';
                        printf("\n - [%s]: You got two aces and win $%d!", players[dealer].name, players[i].bet);
                        temp_win = i;
                        pause(i);
                    }
                    else if(players[i].score>21){
                        check = 's';
                        printf("\n - [%s]: Your cards sum is %d > 21. You lose $%d.", players[dealer].name, players[i].score, players[i].bet);
                        temp_win = dealer;
                        pause(i);
                    }
                    else{
                        check = 0;
                        while(check!='h' && check!='s') //hit or stand?
                        {
                            if(i==0||(players[i].hits>1&&i>0))
                                printf("\n");
                            printf(" - [%s]: Your cards sum is %d %s, (h)it or (s)tand? ", players[dealer].name, players[i].score, players[i].name);
                            if(i==0){ //if player is ai
                                check = play_ai(players, difficulty, ai_count, i);
                                printf("%c", check);
                                pause(0);
                            }else{ //if player is human
                                fflush(stdin);
                                scanf("%c", &check);
                            }
                        }
                    }
                }
                if(temp_win==-1){
                    if(i==0)
                        printf("\n");
                    printf("\n * [%s]: It's my turn %s!\n", players[dealer].name, players[i].name);
                    pause(0);
                    if(players[i].hits==5)
                        stop = 21;
                    else
                        stop = 17;
                    while(players[dealer].score<stop){
                        players[dealer].hits++;

                        if(cards_played==31){
                            generate_cards(game_cards);
                            cards_played = 0;
                            ai_count[0] = 0;
                            ai_count[1] = 0;
                        }

                        give_card(players, game_cards, cards_played, ai_count, dealer);
                        cards_played++;
                        printf("\n - [%s]: My hand:", players[dealer].name);
                        show_hand(players[dealer].cards, players[dealer].hits);
                        pause(0);
                    }
                    if(players[dealer].score==21){
                        printf("\n - [%s]: My cards sum is 21. I win $%d.", players[dealer].name, players[i].bet);
                        temp_win = dealer;
                    }
                    else if(players[dealer].aces==2 && players[dealer].hits==2){
                        printf("\n - [%s]: I got two aces and win $%d!", players[dealer].name, players[i].bet);
                        temp_win = dealer;
                    }
                    else if(players[dealer].score>21){
                        printf("\n - [%s]: My cards sum is %d > 21. I lose $%d.", players[dealer].name, players[dealer].score, players[i].bet);
                        temp_win = i;
                    }else if(players[dealer].score>=players[i].score){
                        printf("\n - [%s]: My cards sum is %d. I win $%d!", players[dealer].name, players[dealer].score, players[i].bet);
                        temp_win = dealer;
                    }else{
                        printf("\n - [%s]: %s wins $%d!", players[dealer].name, players[i].name, players[i].bet);
                        temp_win = i;
                    }
                    pause(1);
                }
                else if(i==0)
                    pause(1);
                if(temp_win==dealer){
                    players[i].money -= players[i].bet;
                    pot += players[i].bet;
                }else{
                    players[i].money += players[i].bet;
                    pot -= players[i].bet;
                }
                if(players[i].money==0){
                    out_count++;
                    players[i].is_out = 1;
                }
                if(players[dealer].money==0){
                    out_count++;
                    players[dealer].is_out = 1;
                }
                end_turn(players,dealer);
                end_turn(players,i);
            }
        }

        if(pot>=3*init_pot && round2_check==0){
            round2_check = 1;
        }
        else{
            round2_check = 0;
            players[dealer].money += pot;
            dealer++;
        }
    }
    i = 0;
    while(players[i].is_out==1){
        i++;
    }
    printf("\n\n - %s won the game! Press enter to exit... ", players[i].name);
    getchar();
    return 0;
}

void give_card(player *players, card game_cards[], int cards_played, int ai_count[], int id)
{
    card play_card;
    int card_id;

    card_id = get_random(0,31-cards_played);

    play_card.card = game_cards[card_id].card;
    play_card.type = game_cards[card_id].type;

    remove_card(card_id, game_cards, cards_played);

    players[id].score += play_card.card;

    if(play_card.card==11)
        players[id].aces++;

    players[id].cards[players[id].hits-1].card = play_card.card;
    players[id].cards[players[id].hits-1].type = play_card.type;

    if(play_card.card<=7)
        ai_count[0] += 1;
    else
        ai_count[1] += 1;
}

void remove_card(int card_id, card game_cards[], int cards_played)
{
    int i;
    for(i=card_id;i<=30-cards_played;i++){
        game_cards[i].card = game_cards[i+1].card;
        game_cards[i].type = game_cards[i+1].type;
    }
    game_cards[i].card = 0;
    game_cards[i].type = 0;
}

int bet_ai(player *players, int pot, int difficulty, int ai_count[], int id)
{
    int bet;
    if(difficulty==3){
        if(players[id].score>=9)
            bet = 0.5*pot*(1+(ai_count[0]/(2*ai_count[1]+0.1)));
        else
            bet = pot*0.2;
    }else if(difficulty==2){
        if(get_random(0,1))
            if(players[id].score>=9)
                bet = 0.5*pot*(1+(ai_count[0]/(2*ai_count[1]+0.1)));
            else
                bet = pot*0.2;
        else
            bet = pot*0.01*get_random(10,70);
    }else
        bet = pot*0.01*get_random(10,70);
    if(bet==0)
        bet = pot*0.2;
    if(bet>pot)
        bet = pot;
    if(bet>players[id].money)
        bet = players[id].money;
    return bet;
}

char play_ai(player *players, int difficulty, int ai_count[], int id)
{
    if(difficulty==3)
        if(players[id].score>=18)
            return 's';
        else if((players[id].score>=16 && ai_count[1]>=2*ai_count[0])||players[id].score<=15)
            return 'h';
        else
            return 's';
    else if(difficulty==2)
        if(get_random(0,1))
            if(players[id].score>=18)
                return 's';
            else if((players[id].score>=16 && ai_count[1]>=2*ai_count[0])||players[id].score<=15)
                return 'h';
            else
                return 's';
        else
            if(players[id].score<17)
                return 'h';
            else
                return 's';
    else
        if(players[id].score<17)
            return 'h';
        else
            return 's';
}

void end_turn(player *players, int id)
{
    players[id].aces = 0;
    players[id].bet = 0;
    players[id].hits = 0;
    players[id].score = 0;
    free(players[id].cards);
    players[id].cards = (card *)calloc(MAX_CARDS, sizeof(card));
}

void show_hand(card *cards, int hits)
{
    int i;
    for(i=0;i<hits;i++){
        switch(cards[i].card){
        case 2:
            printf(" J");
            break;
        case 3:
            printf(" Q");
            break;
        case 4:
            printf(" K");
            break;
        case 11:
            printf(" A");
            break;
        default:
            printf(" %d",cards[i].card);
            break;
        }
        switch(cards[i].type){
        case 1:
            printf("\x03");
            break;
        case 2:
            printf("\x04");
            break;
        case 3:
            printf("\x05");
            break;
        default:
            printf("\x06");
            break;
        }
    }
}

void generate_cards(card game_cards[])
{
    int i, j;
    for(j=0;j<=3;j++){ //For each kind of card
        for(i=0;i<=2;i++){ //J,Q,K
            game_cards[8*j+i].card = i+2;
            game_cards[8*j+i].type = j+1;
        }
        for(i=3;i<=6;i++){ //7-10
            game_cards[8*j+i].card = i+4;
            game_cards[8*j+i].type = j+1;
        }
        game_cards[8*j+7].card = 11; //aces
        game_cards[8*j+7].type = j+1;
    }
}

int get_random(int from, int to)
{
    return (rand()%(to-from))+from;
}

char name_taken(char *name, player *players, int added)
{
    int i = 0;
    char found = 0;
    while(found==0&&i<added){
        if(strcmp(name, players[i].name)==0){
            found = 1;
        }else{
            i++;
        }
    }
    return found;
}

void pause(int i)
{
    if(i!=0){
        printf(" Enter to continue... ");
        fflush(stdin);
        getchar();
    }else{
        Sleep(1000);
    }
}

void top_ui()
{
    printf("\n +----------------------------------------------+\n");
    printf(" |  ______ ____                                 |\n");
    printf(" | |__    |_   |   .-----.---.-.--------.-----. |\n");
    printf(" | |    __|_|  |_  |  _  |  _  |        |  -__| |\n");
    printf(" | |______|______| |___  |___._|__|__|__|_____| |\n");
    printf(" |                 |_____|                      |\n");
    printf(" |                                              |\n");
    printf(" +----------------------------------------------+");
    printf("\n\n   Developed by Chris Samarinas\n");
}
