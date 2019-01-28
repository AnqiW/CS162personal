#include <stdio.h>
int main(int argc, char *argv[]){
int num_lines = 0;
int num_words = 0;
int num_chars = 0;
int newline = 1;
int lastisspace = 0;
int rflag = 0;
if (argc==1){
        char iochar;
 while ((iochar = getchar()) !=EOF)
    {if (newline && (iochar!=' ') &&( iochar != '\n')&&(iochar != '\r') ){
                        num_words+=1;
                        newline = 0;}
        if ((iochar == ' '|| iochar== '\t') && lastisspace == 0 && newline == 0){
                        num_words+=1;
			}
                if (iochar == '\n' && rflag ==0){
                        num_lines +=1;
			newline = 1;}
		if (iochar == '\r'){
			num_lines +=1;
			newline = 1;
			rflag = 1;	}
		if (iochar != '\r'){
			rflag = 0;}
		if (iochar == ' '|| iochar  == '\t'){
                lastisspace = 1;}
		else{
		lastisspace = 0;}
                num_chars += 1;

}
}else{
char *file = argv[1];
FILE *opened = fopen(file, "r");
char current = getc(opened);
char next_curr = getc(opened);
while (current != EOF){
		if (newline && current!=' ' && current != '\n'&&(current != '\r') ){
			num_words+=1;
			newline = 0;}
		if (current == ' ' && lastisspace == 0 && newline == 0){
                        num_words+=1;
                        }
                if (current == '\n' && rflag ==0){
                        num_lines +=1;
                        newline = 1;}
                if (current == '\r'){
                        num_lines +=1;
                        newline = 1;
                        rflag = 1;      }
                if (current != '\r'){
                        rflag = 0;}
                if (current == ' '){
                lastisspace = 1;}
                else{
                lastisspace = 0;}
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

