#include <stdio.h>
#include <stdint.h>
//#define MEMSIZE 4096
#define STACKSIZE 512

//global vars- whats the point of passing em around?
long int	stack[STACKSIZE];
long int*	sp;
//char	data[MEMSIZE];
//char*	dataptr;		//outdated
FILE* 	in;
FILE*	out;
uint16_t addr;	//addr: how many machine code bytes since beginnning of the program, like pc but compile time



void inc_data_ptr(){
/*
	if (counter<256 && counter>0){
		fprintf(out,"\tld a,$%02X\n", (char) counter);//CRAP,FIX
		fprintf(out,"\tadd a,l\n");
		fprintf(out,"\tld l,a\n");
		fprintf(out,"\tld a,0\n");
		fprintf(out,"\tadc a,h\n");
		fprintf(out,"\tld  h,a\n");
	}//probably over-complicating things I shouldnt
*/
	int counter=0;
	int temp=fgetc(in);
	while(temp=='>' || temp=='<'){
		counter+=(temp=='>' ? 1 : -1);
		temp=fgetc(in);
	}
	ungetc(temp,in);
	while(counter<0)	counter+=0x1000;		//(-5)%7 == -5
	counter%=0x1000;
	switch(counter){
		case 0x003:	fprintf(out,"\tinc hl\n");	addr++;
		case 0x002:	fprintf(out,"\tinc hl\n");	addr++;
		case 0x001:	fprintf(out,"\tinc hl\n");	addr++;
		case 0x000:	break;
		case 0xFFD:	fprintf(out,"\tdec hl\n");	addr++;
		case 0xFFE:	fprintf(out,"\tdec hl\n");	addr++;
		case 0xFFF:	fprintf(out,"\tdec hl\n");	addr++;
				break;
		default:fprintf(out,"\tld bc,$%02X\n",(uint16_t)counter);	addr+=3;
				fprintf(out,"\tadd hl,bc\n");	addr++;
				break;
	}
	fprintf(out,"\tres 4,h\n");		addr+=2;//to remain in the Cxxx range
										//doesnt cover all values of counter...

}
void dec_data_ptr(){
	fprintf(out,"\tdec hl\n");
	fprintf(out,"\tres 4,h\n");
	fprintf(out,"\tres 5,h\n");
	fprintf(out,"\tset 6,h\n");
	addr+=7;
}
void inc_data(){
	uint8_t counter=0;
	char temp=fgetc(in);
	while(temp=='+' || temp=='-'){
		counter+=(temp=='+' ? 1 : -1);
		temp=fgetc(in);
	}
	ungetc(temp,in);
//assumes 2's complement
	fprintf(out,".plus%02X_%04X\n",counter,addr);
	switch(counter){
		case 0x03:fprintf(out,"\tinc [hl]\n");	addr++;//3
		case 0x02:fprintf(out,"\tinc [hl]\n");	addr++;//3
		case 0x01:fprintf(out,"\tinc [hl]\n");	addr++;//3
		case 0x00:break;
		case 0xFD:fprintf(out,"\tdec [hl]\n");	addr++;//3
		case 0xFE:fprintf(out,"\tdec [hl]\n");	addr++;//3
		case 0xFF:fprintf(out,"\tdec [hl]\n");	addr++;//3
				break;
		default:fprintf(out,"\tld a,$%02X\n",	counter);	addr+=2; //2
				fprintf(out,"\tadd a,[hl]\n");			addr++;  //2
				fprintf(out,"\tld [hl],a\n");			addr++;  //2
				break;
}	}



void start_loop(){
	fprintf(out,".l_start%04X\n",addr);

	fprintf(out,"\txor A\n");	addr++;	//1
	fprintf(out,"\tcp [hl]\n");	addr++;	//2
	*(sp--)=ftell(out);	//push current file position
	*(sp--)=addr-2;		//jump to xor a, cant assume the value in a

	fprintf(out,"\tjp z,.l_end%04X\n",addr);	addr+=3;
									//placeholder adress, later replaced on ]
									//idea- put there an adress to redirect to
									//	an ERROR block in case ] wont replace it
}
void end_loop(){
	uint16_t temp_pos;
	fprintf(out,"\tjp .l_start%04X\n",(unsigned int) *(++sp));	addr+=3;
//for some reason, calling it ".loop_end_
	fprintf(out,".l_end%04X\n",addr);
	temp_pos=ftell(out);
	//fix corresponding [ jp instruction
	fseek(out,*(++sp),SEEK_SET);
	fprintf(out,"\tjp z,.l_end%04X\n",addr);	//the adress after ]
	fseek(out,temp_pos,SEEK_SET);
}

void print_data(){
	//fix the putc at some point- to use [hl] instead of d
	fprintf(out,".print_%04X\n",addr);

	fprintf(out,"\tpush hl\n");		addr++;
	fprintf(out,"\tld d,[hl]\n");	addr++;
	fprintf(out,"\tcall putChar\n");addr+=3;
	fprintf(out,"\tpop hl\n");		addr++;
}
void read_data(){
	/*
	INPUT?!?!
	*/
	/*idea- move all bg up by 3 tiles, put window on,  turn screen on,get input,
													turn screen off, return*/
	//*dataptr=getchar();
}

int main(int argc, char *argv[])
{
	{//argc, argv check
		if (argc<3){
			printf("please enter input and output file name\n");
			return 1;
		}if (argc>3){
			puts("too many args\n");
			return 1;
	}	}

	{//open in/out files
		in=fopen(argv[1],"r");
		if(in==NULL){
			printf("cant open %s!\n",argv[1]);
			return 1;
		}
		out=fopen(argv[2],"w");
		if(out==NULL){
			printf("cant open %s!\n",argv[2]);
			return 1;
	}	}
	{//init global things
		for (int i=0;i<STACKSIZE;i++) stack[i]=0;
		sp = &stack[STACKSIZE-1];
		{//add header file and init
			fprintf(out,"INCLUDE \"included/hardware.inc\"\n");
			fprintf(out,"INCLUDE \"included/ibmpc1.inc\"\n");
//			fprintf(out,"INCLUDE \"terminal/putc.z80\"\n\n");
//			fprintf(out,"INCLUDE \"terminal/initTerminal.z80\"\n");
			fprintf(out,"SECTION \"font data\",ROM0\n");
			fprintf(out,"font_start::\n");
			fprintf(out,"	chr_IBMPC1	1,8\n");
			fprintf(out,"font_end:\n\n");

			fprintf(out,"SECTION \"Header\", ROM0[$100]\n\n");
			fprintf(out,"EntryPoint:\n");
			fprintf(out,"\tDI\n");
			fprintf(out,"\tjp Start\n\n");
			fprintf(out,"REPT $150 - $104\n");
			fprintf(out,"\tDB $00\n");
			fprintf(out,"ENDR\n\n");


			fprintf(out,"SECTION \"Game code\", ROM0[$150]\n\n");
			addr=0x0150;
			fprintf(out,"Start:\n");
			fprintf(out,"\tld sp,$FFF0\n");
			addr+=3;
			fprintf(out,"\tcall initTerminal\n");
			addr+=3;
		}{	//clear screen
			fprintf(out,".clearScreen\n");
			fprintf(out,"\tld hl, _SCRN0\n");	addr+=3;/***/
			fprintf(out,"\tld de,32*32\n");		addr+=3;
			fprintf(out,"\tld b,\" \"\n");		addr+=2;
			fprintf(out,"\tcall memFill\n");	addr+=3;
		}{	//clear memory
			fprintf(out,".cleanWRAM\n");
			fprintf(out,"\tld hl,_RAM\n");		addr+=3;
			fprintf(out,"\tld de,$1000\n");		addr+=3;
			fprintf(out,"\tld b,0\n");			addr+=2;	//*optimization: maybe replace with xor a; ld b,a for a known value in a?
			fprintf(out,"\tcall memFill\n");	addr+=3;

		}/**/
		fprintf(out,"\tld hl,$C000\n");		addr+=3;//159
	}
	char ins;
	while (feof(in)==0){
		ins=fgetc(in);
		switch(ins){
			case '<'://:	dec_data_ptr();	break;
			case '>':{
						ungetc(ins,in);
						inc_data_ptr();	break;	}
			case '-':
			case '+':{	ungetc(ins,in);
						inc_data();	break;		}
			case '.':	print_data();	break;
			case ',':	read_data();	break;
			case '[':	start_loop();	break;
			case ']':	end_loop();	break;
	}	}
	putchar('\n');
	fprintf(out,"\tSTOP\n");
	fclose(in);
	fclose(out);
	return 0;
	/**/
}
