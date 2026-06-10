.equ SYS_WRITE, 64
.equ SYS_READ, 63
.equ SYS_CLOSE, 57
.equ SYS_EXIT, 93
.equ SYS_SOCKET, 198
.equ SYS_BIND, 200
.equ SYS_LISTEN, 201
.equ SYS_ACCEPT, 202

.equ AF_INET, 2
.equ SOCK_STREAM, 1
.equ PORT, 8080
.equ BACKLOG, 5
.equ BUFFER_SIZE, 4096

.global start_api, get_route, send_json, send_404

.section .text

start_api:
    mov x0, #AF_INET
    mov x1, #SOCK_STREAM
    mov x2, #0
    mov x8, #SYS_SOCKET
    svc #0
    cmp x0, #0
    b.lt socket_failed
    mov x19, x0
    
    sub sp, sp, #32
    mov x0, #AF_INET
    strh w0, [sp, #0]
    mov x0, #0x901F
    strh w0, [sp, #2]
    mov x0, #0
    str x0, [sp, #4]
    
    mov x0, x19
    mov x1, sp
    mov x2, #16
    mov x8, #SYS_BIND
    svc #0
    cmp x0, #0
    b.lt bind_failed
    
    mov x0, x19
    mov x1, #BACKLOG
    mov x8, #SYS_LISTEN
    svc #0
    cmp x0, #0
    b.lt listen_failed
    
    mov x0, #1
    adr x1, msg_start
    mov x2, msg_start_end - msg_start
    mov x8, #SYS_WRITE
    svc #0
    ret

socket_failed:
    mov x0, #1
    adr x1, err_socket
    mov x2, err_socket_end - err_socket
    mov x8, #SYS_WRITE
    svc #0
    b exit_fail

bind_failed:
    mov x0, #1
    adr x1, err_bind
    mov x2, err_bind_end - err_bind
    mov x8, #SYS_WRITE
    svc #0
    b exit_fail

listen_failed:
    mov x0, #1
    adr x1, err_listen
    mov x2, err_listen_end - err_listen
    mov x8, #SYS_WRITE
    svc #0

exit_fail:
    mov x0, #1
    mov x8, #SYS_EXIT
    svc #0

get_route:
    mov x0, x19
    mov x1, #0
    mov x2, #0
    mov x8, #SYS_ACCEPT
    svc #0
    cmp x0, #0
    b.lt accept_failed
    mov x20, x0
    
    sub sp, sp, #BUFFER_SIZE
    mov x0, x20
    mov x1, sp
    mov x2, #BUFFER_SIZE - 1
    mov x8, #SYS_READ
    svc #0
    
    mov x0, sp
    ret

accept_failed:
    mov x0, #0
    ret

send_json:
    mov x21, x0
    mov x22, x1

    mov x0, x20
    adr x1, json_hdr
    mov x2, json_hdr_end - json_hdr
    mov x8, #SYS_WRITE
    svc #0

    mov x0, x20
    mov x1, x21
    mov x2, x22
    mov x8, #SYS_WRITE
    svc #0

    mov x0, x20
    mov x8, #SYS_CLOSE
    svc #0

    add sp, sp, #BUFFER_SIZE
    ret

send_404:
    mov x0, x20
    adr x1, json_404
    mov x2, json_404_end - json_404
    mov x8, #SYS_WRITE
    svc #0
    
    mov x0, x20
    mov x8, #SYS_CLOSE
    svc #0
    add sp, sp, #BUFFER_SIZE
    ret

.section .data
msg_start:
    .ascii "\n[+] ARM64 API Server Started\n"
    .ascii "[+] Listening on port 8080\n"
    .ascii "[+] Ready for requests\n\n"
msg_start_end:

json_hdr:
    .ascii "HTTP/1.1 200 OK\r\n"
    .ascii "Content-Type: application/json\r\n"
    .ascii "Connection: close\r\n"
    .ascii "\r\n"
json_hdr_end:

json_404:
    .ascii "HTTP/1.1 404 Not Found\r\n"
    .ascii "Content-Type: application/json\r\n"
    .ascii "Connection: close\r\n"
    .ascii "\r\n"
    .ascii "{\"error\":\"Endpoint not found\"}\r\n"
json_404_end:

err_socket:
    .ascii "ERROR: socket() failed\n"
err_socket_end:
err_bind:
    .ascii "ERROR: bind() failed\n"
err_bind_end:
err_listen:
    .ascii "ERROR: listen() failed\n"
err_listen_end: