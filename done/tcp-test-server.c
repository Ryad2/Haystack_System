#include "error.h"
#include "socket_layer.h"
#include "string.h"

const int MAX_ANSWER_LENGTH = 2048 + 5;
const char *const end = "<EOF>";
const char *const smallFileAck = "small";
const char *const bigFileAck = "big";
const char *const ack = "ACK";

int main(int argc, char* argv[]) {

    if (argc != 2) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        argc--; argv++; // skips ./

        // init
        int socket = (int) argv[0];
        tcp_server_init(socket);

        while(1) {

            // not sure how to initiate conversation
            tcp_accept(socket);


            // receive file length
            const char initMess[MAX_ANSWER_LENGTH + 1] = {0};
            tcp_read(socket, initMess, MAX_ANSWER_LENGTH);
            int length = atoi(initMess);

            // send ACK
            const char fileAckMess[MAX_ANSWER_LENGTH + 1] = {0};
            strcat(fileAckMess, (length >= 1024) ? bigFileAck : smallFileAck);
            strcat(fileAckMess, end);
            tcp_send(socket, fileAckMess, MAX_ANSWER_LENGTH);
            if (length >= 1024) continue;


            // get file
            const char fileMess[MAX_ANSWER_LENGTH + 1] = {0};
            tcp_read(socket, fileMess, MAX_ANSWER_LENGTH);
            printf("%.*s\n", length, fileMess);

            
            // end ACK
            const char lastMess[MAX_ANSWER_LENGTH + 1] = {0};
            strcat(lastMess, end);
            tcp_send(socket, fileMess, MAX_ANSWER_LENGTH);

        }
    }
}
