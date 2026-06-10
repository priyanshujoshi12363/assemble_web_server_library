.include "include/web.inc"

.global _start

.section .text

_start:
    bl start_api

main_loop:
    bl get_route
    mov x21, x0
    
    sub sp, sp, #256
    mov x22, sp
    mov x0, x21
    mov x1, x22
    bl extract_path
    
    mov x0, x22
    adr x1, path_users
    bl compare_path
    cbz x0, check_health
    
    adr x0, users_json
    mov x1, users_json_end - users_json
    bl send_json
    add sp, sp, #256
    b main_loop

check_health:
    mov x0, x22
    adr x1, path_health
    bl compare_path
    cbz x0, not_found
    
    adr x0, health_json
    mov x1, health_json_end - health_json
    bl send_json
    add sp, sp, #256
    b main_loop

not_found:
    bl send_404
    add sp, sp, #256
    b main_loop

extract_path:
    add x0, x0, #4
copy_loop:
    ldrb w2, [x0], #1
    cmp w2, #' '
    beq copy_done
    strb w2, [x1], #1
    b copy_loop
copy_done:
    strb wzr, [x1]
    ret

compare_path:
    mov x2, #0
comp_loop:
    ldrb w3, [x0, x2]
    ldrb w4, [x1, x2]
    cmp w3, w4
    bne no_match
    cmp w3, #0
    beq is_match
    add x2, x2, #1
    b comp_loop
no_match:
    mov x0, #0
    ret
is_match:
    mov x0, #1
    ret

.section .data
path_users:
    .asciz "/users"
path_health:
    .asciz "/health"

users_json:
    .ascii "{\"users\":[{\"id\":1,\"name\":\"Alice\"},{\"id\":2,\"name\":\"Bob\"}]}\n"
users_json_end:

health_json:
    .ascii "{\"status\":\"ok\",\"message\":\"Server is healthy\"}\n"
health_json_end: