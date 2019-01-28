#include <stdio.h>
int main(int argc, char *argv[]){
int num_lines = 0;
int num_words = 0;
int num_chars = 0;
int newline = 1;
if (argc==1){
        char iochar;
 while ((iochar = getchar()) !=EOF)
    {if (newline && (iochar!=' ') &&( iochar != '\n') ){
                        num_words+=1;
                        newline = 0;}
        if (iochar == ' '){
                        num_words+=1;}
                if (iochar == '\n'){
                        num_lines +=1;
			newline = 1;}
                num_chars += 1;

}
}else{
char *file = argv[1];
FILE *opened = fopen(file, "r");
char current = getc(opened);
char next_curr = getc(opened);
while (current != EOF){
		if (newline && current!=' ' && current != '\n' ){
			num_words+=1;
			newline = 0;}
                if (current == ' '){
                        num_words+=1;}
                if (current == '\n'){
			newline = 1;
                        num_lines +=1;}

                num_chars += 1;
                current = next_curr;
                next_curr = getc(opened);
                }
}


printf("%d ", num_lines);
printf("%d ", num_words);
printf("%d ", num_chars);
return 0;
}

