#include <stdio.h>
#include <string.h>


//Count how many lines a .txt file has
int count_lines(char* fname){

  FILE *file;
  int count = 0; 
  char c; 


  file = fopen(fname, "r");

  if (file == NULL) {
    return -1;
  }

  for (c = getc(file); c != EOF; c = getc(file))
    if (c == '\n')
      count = count + 1;

  fclose(file);

  return count;
}

// Returning via char* buffer the nth line from a .txt file
void get_n_line(char* file_name, int line_number, char* buffer, int buffer_size){
  
  FILE *file = fopen(file_name, "r");

  if (file == NULL) {
    buffer = "Error";
    return;
  }

    int copy_characters = 0;

    int count = 1;
    
    while(fgets(buffer, buffer_size, file) != NULL){

      if (count == line_number){
        fclose(file);
        return;
      }

      count++;
    }

    fclose(file);
    strcpy(buffer, "Error");

    return;
  
}