#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXNAME 80  /* maximum permitted name size, not including \0 */
#define NPITS 6  /* number of pits on a side, not including the end pit */
#define NPEBBLES 4 /* initial number of pebbles per pit */
#define MAXMESSAGE (MAXNAME + 50) /* initial number of pebbles per pit */

int port = 3000;
int listenfd;

struct player {
    int fd;
    char name[MAXNAME+1]; 
    int pits[NPITS+1];  // pits[0..NPITS-1] are the regular pits 
                        // pits[NPITS] is the end pit
    int turn;
    struct player *next;
};
struct player *playerlist = NULL;


extern int accept_connection(int fd, struct player *playerlist);
extern void parseargs(int argc, char **argv);
extern void makelistener();
extern int compute_average_pebbles();
extern int game_is_over();  /* boolean */
extern void broadcast(char *s, int length);
extern int same_name(struct player* new_player);
extern void display_score(int broad, int fd);


int main(int argc, char **argv) {
    char msg[MAXMESSAGE];

    parseargs(argc, argv);
    makelistener();

    int max_fd = listenfd;
    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(listenfd, &all_fds);

    int num_players = 0;
    while (!game_is_over()) {
        fd_set listen_fds = all_fds;
        int ready = select(max_fd+1, &listen_fds, NULL, NULL, NULL);
        if (ready == -1) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(listenfd, &listen_fds)) {
            int client_fd = accept_connection(listenfd, playerlist);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            FD_SET(client_fd, &all_fds);

            printf("Accepted connection\n");

            write(client_fd, "Welcome to Mancala. What is your name?\r\n", sizeof(char)*40);

            if (!playerlist) { // when the new player is the first player to join
                playerlist = malloc(sizeof(struct player));
                playerlist->fd = client_fd;

                read(client_fd, playerlist->name, sizeof(char) * (MAXNAME+1));
                while (playerlist->name[0] == '\n') {
                    write(client_fd, "Invalid name. Please enter a different name.\r\n", sizeof(char) * 46);
                    read(client_fd, playerlist->name, sizeof(char) * (MAXNAME+1));
                }
                for (int i = 0; i < MAXNAME+1; i++) {
                    if (playerlist->name[i] == '\n') {
                        playerlist->name[i] = '\0';
                    }
                }

                for (int i = 0; i < NPITS; i++) {
                    playerlist->pits[i] = 4;
                }

                playerlist->next = NULL;
                num_players++;
            } else { // when there are already other players in the game
                struct player* new_player = malloc(sizeof(struct player));
                new_player->fd = client_fd;

                read(client_fd, new_player->name, sizeof(char) * (MAXNAME+1));
                while (new_player->name[0] == '\n' || same_name(new_player)) {
                    write(client_fd, "Invalid name. Please enter a different name.\r\n", sizeof(char) * 46);
                    read(client_fd, new_player->name, sizeof(char) * (MAXNAME+1));
                }
                for (int i = 0; i < MAXNAME+1; i++) {
                    if (new_player->name[i] == '\n') {
                        for (int j = i; j < MAXNAME+1; j++) {
                            new_player->name[j] = '\0';
                        }
                    }
                }

                for (int i = 0; i < NPITS; i++) {
                    new_player->pits[i] = compute_average_pebbles();
                }

                new_player->next = playerlist;
                playerlist = new_player;
                num_players++;
            }

            broadcast(playerlist->name, MAXNAME+1);
            broadcast(" has been added to the game!\r\n", 30); // notifies all players that a new player has joined

            display_score(0, playerlist->fd); // displays the scores to the new player
        }

        int first_turn = 1;
        for (struct player *p = playerlist; p; p = p->next) { // determine next player's turn
            if (p->turn) {
                first_turn = 0;
                p->turn = 0;
                if (p->next) {
                    p->next->turn = 1;
                    break;
                } else {
                    playerlist->turn = 1;
                }
            }
        }
        if (first_turn) {
            playerlist->turn = 1;
        }

        for (struct player* p = playerlist; p; p = p->next) {
            if (p->turn && num_players > 1) {
                broadcast("It is ", 6);
                broadcast(p->name, MAXNAME+1);
                broadcast("'s move.\r\n", 10);
                break;
            }
        }
        
        char select_pit[1]; // pit selected by the player whose turn it is
        int pit_num;
        for (struct player *p = playerlist; p; p = p->next) {
            if (num_players > 1) {                
                if (p->turn) {
                    write(p->fd, "Your move?\r\n", sizeof(char) * 12);
                    read(p->fd, select_pit, sizeof(char) * 1);
                    while (select_pit[0] == '\n') {
                        read(p->fd, select_pit, sizeof(char) * 1);
                    }
                    pit_num = strtol(select_pit, NULL, 10);
                    
                    while (pit_num < 0 || pit_num > NPITS) { // invalid pit number
                        write(p->fd, "Invalid number. Please try again.\r\n", sizeof(char) * 35);
                        read(p->fd, select_pit, sizeof(char) * 1);
                        pit_num = strtol(select_pit, NULL, 10);
                    }
                    broadcast(p->name, MAXNAME+1);
                    broadcast(" entered ", 9);
                    broadcast(select_pit, 1);
                    broadcast("\r\n", 2);

                    // take pebbles in pit_num and distribute them to the right
                    for (int i = 1; i < p->pits[pit_num]+1; i++) {
                        if (pit_num+i <= NPITS) {
                            p->pits[pit_num+i] += 1;
                        } else {
                            if (p->next) {
                                p->next->pits[pit_num+i-NPITS-1] += 1;
                            } else {
                                playerlist->pits[pit_num+i-NPITS-1] += 1;
                            }
                        }
                    }
                    p->pits[pit_num] = 0;

                    display_score(1, 0); // displays the score after someone's turn
                }
            }
        }
    }

    broadcast("Game over!\r\n", 12);
    printf("Game over!\n");
    for (struct player *p = playerlist; p; p = p->next) {
        int points = 0;
        for (int i = 0; i <= NPITS; i++) {
            points += p->pits[i];
        }
        printf("%s has %d points\r\n", p->name, points);
        snprintf(msg, MAXMESSAGE, "%s has %d points\r\n", p->name, points);
        broadcast(msg, MAXMESSAGE);
    }

    return 0;
}

int accept_connection(int fd, struct player *playerlist) { // wrapper function for accept
    int client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        close(fd);
        exit(1);
    }

    return client_fd;
}

int same_name(struct player* new_player) { // checks if the new player's name is the same as an already existing player's name
    int same; 
    for (struct player* p = playerlist; p; p = p->next) {
        same = 1;
        for (int i = 0; i < MAXNAME; i++) {
            if (p->name[i] != new_player->name[i] && new_player->name[i] != '\n') {
                same = 0;
            }
        }
        if (same) {
            return 1;
        }
    }
    return 0;
}

void display_score(int broad, int fd) { // displays the current number of pebbles in each pit
    char pit[1], pebbles[1];
    if (broad) {
        for (struct player* p = playerlist; p; p = p->next) {
            broadcast(p->name, MAXNAME+1);
            broadcast(":  ", 3);

            for (int i = 0; i < NPITS; i++) {
                broadcast("[", 1);
                sprintf(pit, "%d", i);
                broadcast(pit, 1);
                broadcast("]", 1);

                sprintf(pebbles, "%d", p->pits[i]);
                broadcast(pebbles, 1);
                broadcast(" ", 1);
            }

            broadcast("[end pit]", 9);
            sprintf(pebbles, "%d", p->pits[NPITS]);
            broadcast(pebbles, 1);
            broadcast("\r\n", 2);
        }
    } else {
        for (struct player* p = playerlist; p; p= p->next) {
            write(fd, p->name, sizeof(char) * (MAXNAME+1));
            write(fd, ": ", sizeof(char) * 3);

            for (int i = 0; i < NPITS; i++) {
                write(fd, "[", sizeof(char));
                sprintf(pit, "%d", i);
                write(fd, pit, sizeof(char));
                write(fd, "]", sizeof(char));

                sprintf(pebbles, "%d", p->pits[i]);
                write(fd, pebbles, sizeof(char));
                write(fd, " ", sizeof(char));
            }

            write(fd, "[end pit]", sizeof(char)*9);
            sprintf(pebbles, "%d", p->pits[NPITS]);
            write(fd, pebbles, sizeof(char));
            write(fd, "\r\n", sizeof(char)*2);
        } 
    }
}


void parseargs(int argc, char **argv) {
    int c, status = 0;
    while ((c = getopt(argc, argv, "p:")) != EOF) {
        switch (c) {
        case 'p':
            port = strtol(optarg, NULL, 0);  
            break;
        default:
            status++;
        }
    }
    if (status || optind != argc) {
        fprintf(stderr, "usage: %s [-p port]\n", argv[0]);
        exit(1);
    }
}


void makelistener() {
    struct sockaddr_in r;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
               (const char *) &on, sizeof(on)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    memset(&r, '\0', sizeof(r));
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(port);
    if (bind(listenfd, (struct sockaddr *)&r, sizeof(r))) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, 5)) {
        perror("listen");
        exit(1);
    }
}



/* call this BEFORE linking the new player in to the list */
int compute_average_pebbles() { 
    struct player *p;
    int i;

    if (playerlist == NULL) {
        return NPEBBLES;
    }

    int nplayers = 0, npebbles = 0;
    for (p = playerlist; p; p = p->next) {
        nplayers++;
        for (i = 0; i < NPITS; i++) {
            npebbles += p->pits[i];
        }
    }
    return ((npebbles - 1) / nplayers / NPITS + 1);  /* round up */
}


int game_is_over() { /* boolean */
    int i;

    if (!playerlist) {
       return 0;  /* we haven't even started yet! */
    }

    for (struct player *p = playerlist; p; p = p->next) {
        int is_all_empty = 1;
        for (i = 0; i < NPITS; i++) {
            if (p->pits[i]) {
                is_all_empty = 0;
            }
        }
        if (is_all_empty) {
            printf("got here\n");
            return 1;
        }
    }
    return 0;
}


void broadcast(char* s, int length) {
    struct player* players = playerlist;
    while (players) {
        write(players->fd, s, sizeof(char)*length);
        players = players->next;
    }
}
