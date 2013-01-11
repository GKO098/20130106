//Name 
//			OKUYA Fuminori
//Student Number
//			03-123009
//Grade
//			3rd grade

//to run program ./a.out -f news.txt //(example)
//to run program ./a.out -f *****.txt

//Displaying "Enter query!"				So, you can enter query
//%john and said(example)
//Display "0:john and ..."

//If you want to search NOT mode,
//words which you want to search NOT mode, -"words"
//(example) -and
//(example) john -and

//If you want to search OR mode,
//words which you want to search OR mode, |"words"
//(example) |and
//(example) john |and

//If you want to search NOR mode,
//NOR

//Display number of matching lines and matching words

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 1000			//Size of Buffer
#define MAX_WORD_LEN 100	//Max of Word Length
#define HASHSIZE 255
#define AND_SEARCH 0
#define NOT_SEARCH 1
#define OR_SEARCH 2

typedef struct DOCUMENT *DocumentPtr;
typedef struct DOCUMENT {
	int id;
	char * body;
	struct DOCUMENT * next;
} DocumentNode;

typedef struct LINE *LinePtr;
typedef struct LINE {
	int line_number;			//enter exist line
	struct LINE * next;	//enter next line
} LineNode;

typedef struct WORD *WordPtr;
typedef struct WORD {
	char word[ MAX_WORD_LEN ];				//enter word
	LinePtr exist_line;			//enter exist line
	struct WORD * next;	//enter next word
} WordNode;

char * d_token, * w_token, * q_current;
DocumentNode d_Head={-1, NULL, NULL};				//dummy
DocumentPtr d_current;
WordPtr w_current;
int line_num = 0, word_num=0;
int * compare_exist_line;
LinePtr l_current;

unsigned hash( char * s ) {
	unsigned hashval;
	for ( hashval = 0 ; *s != '\0' ; s++ ) {
		hashval = *s + ( 31 * hashval );
	}
	return hashval % HASHSIZE;
}
void init_w_table( WordPtr w_table ) {
	int i;
	for( i = 0 ; i < HASHSIZE ; i++ ) {
		w_table[i].exist_line = ( LinePtr )malloc( sizeof( LineNode ) );
		w_table[i].next = NULL;
	}
}
void init_exist_line ( int * exist_line , int line_num, int value ) {
	int i;
	for( i=0 ; i < line_num ; i++ ) {
		exist_line[i] = value;
	}
}

load_text( FILE * fp, char * scanned_data, WordPtr w_table )
{
	int current_hash;
//ファイルを一行ずつ読み込み
	for( d_current = &( d_Head ); fgets( scanned_data, BUF_SIZE, fp ) != NULL; line_num++ ) {
		d_current = d_current->next = ( DocumentPtr )malloc( sizeof( DocumentNode ) );
		d_current->body = ( char * )malloc( sizeof( char ) * strlen( scanned_data ) );
		strcpy( d_current->body, scanned_data);
		d_current->id = line_num;
		d_current->next = NULL;
		for( w_token=strtok( scanned_data, " \n\t" ) ; w_token!=NULL ; w_token=strtok( NULL, " \n\t" ) ) {
			word_num++;
			current_hash = hash( w_token );
			for( w_current = &(w_table[ current_hash ]); w_current->next != NULL; w_current = w_current->next ) {		//Same Word or First Word
				if( strcmp( w_current->word, w_token ) == 0 ) {
					word_num--;
					l_current = w_current->exist_line;
					while( l_current->next != NULL ) {
						l_current = l_current->next;
					}
					l_current->line_number = line_num;
					l_current = l_current->next = ( LinePtr )malloc( sizeof( LineNode ) );
					l_current->next = NULL;
					break;
				}
			}
			if( w_current->next == NULL ) {								//New Word!
				strcpy( w_current->word, w_token );
				l_current = w_current->exist_line;
				l_current->line_number = line_num;
				l_current->next = ( LinePtr )malloc( sizeof( LineNode ) );
				l_current->next->next = NULL;
				w_current = w_current->next = ( WordPtr )malloc( sizeof( WordNode ) );
				w_current->exist_line = ( LinePtr )malloc( sizeof( LineNode ) );
				w_current->next = NULL;
			}
		}
	}
}

int search_words( WordPtr w_table, int match_words )
{
	for( w_current = &( w_table[ hash( q_current ) ] ); w_current->next != NULL; w_current = w_current->next ) {
		if( strcmp( w_current->word, q_current ) == 0 ) {
			for( l_current = w_current->exist_line ; l_current->next != NULL ; l_current = l_current->next ) {
				compare_exist_line[ l_current->line_number ] = 1;
				match_words++;
			}
		}
	}
	return match_words;
}

int main( int argc, char *argv[] ) {
	int c, i;
	int * exist_line;
	int match_lines;
	int match_words;
	int search_mode;
	WordPtr w_table = ( WordPtr )malloc( HASHSIZE * sizeof( WordNode ) );
	char scanned_data[ BUF_SIZE ], query[ BUF_SIZE ];
	FILE * fp;
	
	while( ( c = getopt(argc, argv, "f:") ) != -1 ) {				//Get FileName
		if( c== 'f' ) {
			w_token = optarg;
		}
	}
	if( ( fp = fopen( w_token, "r" ) ) == NULL) {				//Read File Mode
		printf("File Open Error!\n");
		return -1;
	}
	init_w_table( w_table );												//Init w_table (HASH_SIZE)
	load_text( fp,  scanned_data, w_table );
	printf("Loading the documents to memory from %s\n", optarg );
	exist_line = (int *)malloc( ( line_num + 1 ) * sizeof( int ) );
	compare_exist_line = (int *)malloc( ( line_num + 1 ) * sizeof( int ) );
	printf("Total number of documents read = %d\n", line_num);
	printf("Total number of words in the index = %d\n", word_num);
	init_exist_line( exist_line, line_num, 1 );
	while( 1 ) {
		match_lines = 0;
		printf("Enter query!\n");
		scanf(" %[^\n]", query );
		for( q_current = strtok( query, " " ) ; q_current != NULL ; q_current = strtok( NULL, " " ) ) {
			init_exist_line( compare_exist_line, line_num, 0 );
			match_words = 0;
			search_mode = AND_SEARCH;
			if( ( char )( *q_current ) == '-' ) {
				search_mode = NOT_SEARCH;
				q_current++;
			}
			if( ( char )( *q_current ) == '|' ) {
				search_mode = OR_SEARCH;
				q_current++;
			}
			match_words = search_words( w_table, match_words );
			if( search_mode == NOT_SEARCH ) {
				for( i = 0 ;i < line_num ;i++ ) {
					compare_exist_line[ i ] = abs( 1 - compare_exist_line[ i ] );//inverse
				}
			}
			if( search_mode == OR_SEARCH ){
				for( i = 0 ;i < line_num ;i++ ) {
					exist_line[ i ] |= compare_exist_line[ i ];
				}
			}else{
				for( i = 0 ;i < line_num ;i++ ) {
					exist_line[ i ] &= compare_exist_line[ i ];
				}
			}
			printf("match_word( %s ) = %d words\n", q_current, match_words);
		}
		d_current = ( d_Head.next );
		for( i = 0 ; i < line_num ; i++ ) {
			if( exist_line[ i ] == 1 ) {
				printf("%d:%s\n", i, d_current->body );
				match_lines++;
			}
			d_current = d_current->next;
		}
		printf("match_line = %d lines\n", match_lines);
		printf("More ?Yes[Y],No[other]\n");
		scanf(" %c", query );
		if(query == "Y") {
			init_exist_line( exist_line, line_num, 1 );
		}
	}
	fclose( fp );
	free( compare_exist_line );
	free( exist_line );
	free( w_table );
}