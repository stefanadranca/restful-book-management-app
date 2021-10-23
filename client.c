#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "parson.h"
#include "helpers.h"
#include "requests.h"

#define MAX_LENGTH 100

// primeste raspunsul de la server si extrage eroarea
char *get_error(char *response) {

    char *pos = strstr(response, "error");
    if (pos == NULL) return NULL;
    char *beg = pos + 8;
    char *end = strstr(response + (beg - response), "\"");

    int size = end - beg;
    char *error = (char *) calloc(size + 1, sizeof(char));
    strncpy(error, response + (beg - response), size);

    return error;
}


// printeaza succes daca cererea a putut fi efectuata sau eroarea
// primita in cazul in care cererea nu s-a putut efectua
void print_out(char *response) {
    char *error = get_error(response);
    if (error != NULL) {
        printf("ERROR: %s\n", error);
    } else {
        puts("SUCCESS!\n");
    }
}

// identifica si returneaza sub forma de string elementele 
// de tip json din raspunsul de la server
char *get_json(char *response) {
    char *pos = strstr(response, "[{");
    if (pos == NULL) return NULL;
    char *beg = pos + 1;
    char *end = strstr(response + (beg - response), "]");

    int size = end - beg;
    char *json = (char *) calloc(size + 1, sizeof(char));
    strncpy(json, response + (beg - response), size);

    return json;
}


// printeaza o lista de obiecte json daca cererea a putut fi efectuata 
// sau eroarea primita in cazul in care cererea nu s-a putut efectua
void print_out_json(char *response) {
    char *error = get_error(response);
    if (error != NULL) {
        printf("ERROR: %s\n", error);
    } else {
        char *json = get_json(response);
        if(json != NULL) {
            printf("%s\n", json);
        } else {
            printf("Nothing to show\n");
        }
        
    }
}

// extrage cookie din raspunsul primit
char *get_cookie(char *response) {

    char *pos = strstr(response, "Set-Cookie");
    if (pos == NULL) return NULL;
    char *beg = pos + 12;
    char *end = strstr(response + (beg - response), ";");

    int size = end - beg;
    char *cookie = (char *) calloc(size + 1, sizeof(char));
    strncpy(cookie, response + (beg - response), size);

    return cookie;
}

// extrage token din raspunsul primit
char *get_token(char *response) {

    char *pos = strstr(response, "token");
    if (pos == NULL) return NULL;
    char *beg = pos + 8;
    char *end = strstr(response + (beg - response), "\"");

    int size = end - beg;
    char *token = (char *) calloc(size + 1, sizeof(char));
    strncpy(token, response + (beg - response), size);

    return token;
}

// returneaza un string ce respecta formatul json cu username si parola
char *auth_json() {

    char username[MAX_LENGTH];
    char password[MAX_LENGTH];
    printf("username=");
    fgets(username, MAX_LENGTH, stdin);
    username[strlen(username) - 1] = '\0';
    printf("password=");
    fgets(password, MAX_LENGTH, stdin);
    password[strlen(password) - 1] = '\0';

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    serialized_string = json_serialize_to_string_pretty(root_value);

    return serialized_string;
}

// returneaza un string ce respecta formatul json cu detaliile despre o carte
char *book_details_json() {

    char title[MAX_LENGTH];
    char author[MAX_LENGTH];
    char genre[MAX_LENGTH];
    int page_count;
    char publisher[MAX_LENGTH];

    printf("title=");
    fgets(title, MAX_LENGTH, stdin);
    title[strlen(title) - 1] = '\0';
    printf("author=");
    fgets(author, MAX_LENGTH, stdin);
    author[strlen(author) - 1] = '\0';
    printf("genre=");
    fgets(genre, MAX_LENGTH, stdin);
    genre[strlen(genre) - 1] = '\0';
    printf("publisher=");
    fgets(publisher, MAX_LENGTH, stdin);
    publisher[strlen(publisher) - 1] = '\0';
    printf("page_count=");
    scanf("%d", &page_count);

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_number(root_object, "page_count", page_count);
    json_object_set_string(root_object, "publisher", publisher);
    serialized_string = json_serialize_to_string_pretty(root_value);

    return serialized_string;
}

int main(int argc, char *argv[])
{
    char *message = NULL;
    char *response = NULL;
    char *error = NULL;
    char *cookie = NULL;
    char *token = NULL;
    int sockfd;
    char command[20];

    sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);

    while (1) {
        
        memset(command, 0, 20);
        fgets(command, 20, stdin);

        if (strncmp(command, "exit", 4) == 0) {
            break;
        }

        if (strncmp(command, "register", 8) == 0) {
            sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_post_request("34.118.48.238", "/api/v1/tema/auth/register", "application/json",
                auth_json(), NULL, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            print_out(response);
            continue;
        }

        if (strncmp(command, "login", 5) == 0) {
            sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_post_request("34.118.48.238", "/api/v1/tema/auth/login", "application/json",
                auth_json(), NULL, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            cookie = get_cookie(response);
            print_out(response);
            continue;
        }

        if (strncmp(command, "enter_library", 13) == 0) {
            sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.118.48.238", "/api/v1/tema/library/access", NULL, cookie, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            token = get_token(response);
            print_out(response);
            continue;
        }

        if (strncmp(command, "get_books", 9) == 0) {
            sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.118.48.238", "/api/v1/tema/library/books", NULL, cookie, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            print_out_json(response);
            continue;
        } else if (strncmp(command, "get_book", 8) == 0) {
            sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);

            // pregatirea rutei de acces
            char path[30];
            int id;
            printf("id=");
            scanf("%d", &id);
            sprintf(path, "/api/v1/tema/library/books/%d", id);

            message = compute_get_request("34.118.48.238", path, NULL, cookie, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            print_out_json(response);
            continue;
        }

        if (strncmp(command, "add_book", 8) == 0) {
            sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_post_request("34.118.48.238", "/api/v1/tema/library/books", "application/json",
                book_details_json(), cookie, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            print_out(response);
            continue;
        }
        if (strncmp(command, "delete_book", 11) == 0) {
            sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);

            // pregatirea rutei de acces
            char path[30];
            int id;
            printf("id=");
            scanf("%d", &id);
            sprintf(path, "/api/v1/tema/library/books/%d", id);

            message = compute_delete_request("34.118.48.238", path, NULL, cookie, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            print_out(response);
            continue;
        }
        if (strncmp(command, "logout", 6) == 0) {
            sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.118.48.238", "/api/v1/tema/auth/logout", NULL,
                cookie, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            print_out(response);
            error = get_error(response);

            // daca nu am primit nicio eroare stergem tokenul si cookie-urile existente
            if (error == NULL) {
                if (token != NULL) {
                    free(token);
                    token = NULL;
                }
                if (cookie != NULL) {
                    free(cookie);
                    cookie = NULL;
                }   
            }
            continue;
        }
    }
    
    close_connection(sockfd);

    return 0;
}
