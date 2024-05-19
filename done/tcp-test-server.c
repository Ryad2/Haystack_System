#include "error.h"
#include "socket_layer.h"
#include "string.h"
#include "util.h"
#include "sys/socket.h"

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
        int passive_socket = tcp_server_init(atouint16(argv[0]));
        printf("socket : %d\n", passive_socket);

        while(1) {
            listen(passive_socket, 1);
            int socket = tcp_accept(passive_socket);

            // receive file length
            const char initMess[MAX_ANSWER_LENGTH + 1] = {0};
            tcp_read(socket, initMess, MAX_ANSWER_LENGTH);
            printf("received %s\n", initMess);
            fflush(stdout);
            int length = atoi(initMess);

            // send ACK
            const char fileAckMess[MAX_ANSWER_LENGTH + 1] = {0};
            strcat(fileAckMess, (length >= 1024) ? bigFileAck : smallFileAck);
            strcat(fileAckMess, end);
            printf("sending %s\n", fileAckMess);
            fflush(stdout);
            tcp_send(socket, fileAckMess, MAX_ANSWER_LENGTH);
            if (length < 1024) {

                // get file
                const char fileMess[MAX_ANSWER_LENGTH + 1] = {0};
                tcp_read(socket, fileMess, MAX_ANSWER_LENGTH);
                printf("received \"%s\"\n", fileMess);
                fflush(stdout);
                
                // end ACK
                const char lastMess[MAX_ANSWER_LENGTH + 1] = {0};
                strcat(lastMess, ack);
                strcat(lastMess, end);
                printf("sending %s\n", lastMess);
                fflush(stdout);
                tcp_send(socket, fileMess, MAX_ANSWER_LENGTH);
            }

        }
    }
}
