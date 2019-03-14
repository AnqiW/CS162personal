#include <stdio.h>
int main(int argc, char *argv[]){
int num_lines = 0;
int num_words = 0;
int num_chars = 0;
if (argc==1){
	char iochar;
 while ((iochar = getchar()) !=EOF)
    {
	if (iochar == ' '){
                        num_words+=1;}
                if (iochar == '\n'){
                        num_lines +=1;}
		num_chars += 1;

}
}else{
char *file = argv[1];
FILE *opened = fopen(file, "r");
char current = getc(opened);
char next_curr = getc(opened);
while (current != EOF){
		if (current == ' '){
			num_words+=1;}
		if (current == '\n'){
			num_lines +=1;}
		
		num_chars += 1;
		current = next_curr;
		next_curr = getc(opened);
		}
}
if( num_chars!=0){
num_words += 1;}

printf("%d ", num_lines);
printf("%d ", num_words);
printf("%d ", num_chars);
}
