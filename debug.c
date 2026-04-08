#include <stdio.h>

int main() {
    // The most important line for CGI
    printf("Content-type: text/html\n\n");
    
    printf("<html><body>");
    printf("<h1>System Check: Success!</h1>");
    printf("<p>Apache on D: drive is running C code correctly.</p>");
    printf("</body></html>");
    
    return 0;
}