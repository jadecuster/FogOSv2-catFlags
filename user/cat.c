#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char buf[512];
int flag_n = 0;
int flag_b = 0;
int flag_e = 0;
int flag_t = 0;
int flag_s = 0;
int flag_v = 0;

void 
printHelp (void){
	printf("Usage: cat [OPTION]... [FILE]...\n");
	printf("Concatenate FILE(s) to standard output.\n\n");
	printf("Options:\n");
	printf("  -n        number all lines\n");
	printf("  -b        number non-empty lines\n");
	printf("  -e        display non-printing characters (see the -v option), and display a dollar sign (‘$’) at the end of each line\n");
	printf("  -t        display non-printing characters (see the -v option), and display tab characters as ‘^I’.");
	printf("  -s        squeeze multiple blank lines\n");
	printf("  -v        show non-printing characters visibly\n");
	printf("  --help    display this help\n");
}


void
cat(int fd)
{
  int n;
  int line_num = 1; //tracks line #s
  int at_line_start = 1; //tracks whether next printed char is at the start of a newline
  int last_blank = 0; //boolean that checks for newline chars for -s
  

  while((n = read(fd, buf, sizeof(buf))) > 0) {
  	for (int i = 0; i < n; i++){
  		char c = buf[i];

  		//line numbering 
  		if (at_line_start){
  			if (flag_n || (flag_b && c != '\n')){
  				printf("%6d\t", line_num);
  				line_num ++;
  			}
  			//no longer at the start of the line
			at_line_start = 0;	
  		}

  		//squeezing blank lines
  		if(flag_s && c == '\t'){
  			if (last_blank){
  				continue;
  			}
  			last_blank = 1;
  		} else {
  			last_blank = 0;
  		}


  		//show tabs
  		if(flag_t && c == '\t'){
  			write(1,"^I",2);
  			continue;
  		}

  		//showing nonprint
  		if(flag_v){
  			if ((c < 32 && c != '\t' && c!= '\t') || (c == 127)){ //has to be a control char or del
  				char output[2];
				output[0] = '^';
				output[1] = (c == 127) ? '?' : c + 64;
				write(1,output,2);
				continue;
  			}
  		}

  		//show $
		if (flag_e && c == '\n'){
			write(1,"$",1);
		}

		//printing char
		write(1,&c,1);

		if(c == '\n'){
			at_line_start = 1;
		}	
  	}
  }
  
  if(n < 0){
    fprintf(2, "cat: read error\n");
    exit(1);
  }
}

int
main(int argc, char *argv[])
{
  int fd, i;

  if(argc <= 1){
    cat(0);
    exit(0);
  }

  for(i = 1; i < argc; i++){
  	//printing help usage
  	if (argv[i][0] == '-'){
  		if(strcmp(argv[i],"--help")){
  			printHelp();
  			exit(0);
  		}
  	}

  	char *option = argv[i] + 1;
	//adding option flags
  	while (*option){
  		switch (*option){
  			case 'n': 
  				flag_n = 1;
  				break;

  			case 'b':
  				flag_b = 1;
  				break;

  			case 'e':
  				flag_v = 1;
  				flag_e = 1;
  				break;

  			case 't':
  				flag_v = 1;
  				flag_e = 1;
  				break;

  			case 's':
  				flag_s = 1;
  				break;

  			case 'v':
  				flag_v = 1;
  				break;

  			default:
  				printf("unknown option -%c", *option);
  				exit(0);
  		}
		option++;
  	} 	
  }

  //run cat on files
  int files = 0;
  for(int i  = 0; i < argc; i++){
  	//skipping flags since they have already been dealt with 
  	if (argv[i][0] == '-'){
  		continue;
  	}

  	files++;
  	if((fd = open(argv[i], O_RDONLY)) < 0){
  		fprintf(2, "cat: cannot open %s\n", argv[i]);
  	    exit(1);
  	}
  	cat(fd);
  	close(fd);
  }
  
  exit(0);
}
