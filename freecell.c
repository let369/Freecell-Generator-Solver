#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define N 5

#define H 0
#define D 1
#define S 2
#define C 3

#define breadth 1
#define depth	2
#define best	3
#define astar	4

#define freecell 3
#define foundation 2
#define new_stack 1
#define stack 0

clock_t t1;                 // Start time of the search algorithm
clock_t t2;                 // End time of the search algorithm
#define TIMEOUT     200     

struct card
{
	char suit;
	int value;
};
int dtop[8];
int ftop[4];
struct tree_node
{
    struct card card_board[3*N/2][8];
    int dtop[8];
    struct card freecells[4];
    struct card foundations[N][4];
    int ftop[4];
    struct tree_node *parent;
    struct tree_node *children[20];
    int h;
    int g;
    int f;
    int move;
};

struct frontier_node
{
    struct tree_node *n;
    struct frontier_node *next;
};

struct frontier_node *frontier_head=NULL;
struct frontier_node *frontier_tail=NULL;

int solution_length;
int *solution;

void syntax_message();
void display_puzzle(struct tree_node *node);
int is_solution(struct card foundations[N][4]);
int get_method(char* s);
int read_puzzle(char* filename,struct card deck[3*N/2][8]);
void initialize_search(struct card deck[3*N/2][8],int method);
struct tree_node *search(int method);
int add_frontier_front(struct tree_node *node);
int add_frontier_back(struct tree_node *node);
int add_frontier_in_order(struct tree_node *node);
void find_children(struct tree_node *current_node,int method);
int check_with_parents(struct tree_node *new_node);
int equal_puzzles(struct tree_node *new_node,struct tree_node *parent);
void extract_solution(struct tree_node *solution_node);
void write_solution_to_file(char* filename, int solution_length, int *solution);
int is_solution2(struct tree_node *node);

int main(int argc, char** argv)
{
    int i,j,err;
    struct card deck[3*N/2][8];
    struct tree_node *solution_node;
    int method;

    if(argc!=4){
        printf("Wrong number of arguments. Use correct syntax:\n");
        syntax_message();
        return -1;
    }
    method=get_method(argv[1]);
    if(method<0){
        printf("Wrong method. Use correct syntax:\n");
        syntax_message();
        return -1;
    }
    err=read_puzzle(argv[2],deck);

    printf("Solving %s using %s...\n",argv[2],argv[1]);

    t1=clock();

    initialize_search(deck,method);
    solution_node = search(method);

    t2=clock();

    if (solution_node!=NULL)
        extract_solution(solution_node);
    else
        printf("No solution found.\n");

    if (solution_length>0)
    {
        printf("Solution found! (%d steps)\n",solution_length);
        printf("Time spent: %f secs\n",((float) t2-t1)/CLOCKS_PER_SEC);
        write_solution_to_file(argv[3], solution_length, solution);
    }

    return 0;
}
int is_solution2(struct tree_node *node)
{
    int i;
    for(i=0;i<4;i++)
    {
        if(node->foundations[ftop[i]][i].value == N)
            continue;
        else return 0;
    }
    return 1;
}
void write_solution_to_file(char* filename, int solution_length, int *solution)
{
    int i;
    FILE *fout;
    fout=fopen(filename,"w");
    if (fout==NULL)
    {
        printf("Cannot open output file to write solution.\n");
        printf("Now exiting...");
        return;
    }
    fprintf(fout,"%d\n",solution_length);
    for (i=0;i<solution_length;i++)
        switch(solution[i])
        {
        case stack:
            fprintf(fout,"stack\n");
            break;
        case new_stack:
            fprintf(fout,"new stack\n");
            break;
        case foundation:
            fprintf(fout,"foundation\n");
            break;
        case freecell:
            fprintf(fout,"freecell\n");
            break;
    }
    fclose(fout);
}

void extract_solution(struct tree_node *solution_node)
{
    int i;
    struct tree_node *temp_node = solution_node;
    solution_length = solution_node->g;

    solution = (int*) malloc(solution_length*sizeof(int));
    temp_node = solution_node;
    i=solution_length;
    while(temp_node->parent!=NULL){
        i--;
        solution[i]=temp_node->move;
        temp_node=temp_node->parent;
    }
}

void find_children(struct tree_node *current_node,int method)
{
    int i,k,j,jj,cc,s,s2;

    for(i=0;i<20;i++){
        current_node->children[i]=NULL;
    }
    cc=0;
    for(i=0;i<8;i++){
            
            if(current_node->card_board[current_node->dtop[i]][i].suit=='H')
                s=0;
            if(current_node->card_board[current_node->dtop[i]][i].suit=='D')
                s=1;
            if(current_node->card_board[current_node->dtop[i]][i].suit=='S')
                s=2;
            if(current_node->card_board[current_node->dtop[i]][i].suit=='C')
                s=3;
            if(current_node->ftop[s]==0){
            if((current_node->card_board[current_node->dtop[i]][i].value-1) == current_node->foundations[current_node->ftop[s]][s].value){

                current_node->children[cc]= (struct tree_node*) malloc(sizeof(struct tree_node));
                for(j=0;j<20;j++){
                    current_node->children[cc]->children[j]=NULL;
                }
                for(j=0;j<3*N/2;j++){
                    for(jj=0;jj<8;jj++){
                        current_node->children[cc]->card_board[j][jj].suit=current_node->card_board[j][jj].suit;
                        current_node->children[cc]->card_board[j][jj].value=current_node->card_board[j][jj].value;
                        current_node->children[cc]->dtop[jj]=current_node->dtop[jj];
                    }
                }
                for(j=0;j<4;j++){
                    current_node->children[cc]->freecells[j].suit=current_node->freecells[j].suit;
                    current_node->children[cc]->freecells[j].value=current_node->freecells[j].value;
                }
                for(j=0;j<N;j++){
                    for(jj=0;jj<4;jj++){
                        current_node->children[cc]->foundations[j][jj].suit=current_node->foundations[j][jj].suit;
                        current_node->children[cc]->foundations[j][jj].value=current_node->foundations[j][jj].value;
                        current_node->children[cc]->ftop[jj]=current_node->ftop[jj];
                    }
                }
                for(j=0;j<8;j++){
                    current_node->children[cc]->dtop[j]=current_node->dtop[j];
                }

                current_node->children[cc]->parent=current_node;
                current_node->children[cc]->move=foundation;
                current_node->children[cc]->g=current_node->g+1;

                current_node->children[cc]->foundations[current_node->children[cc]->ftop[s]][s].suit=current_node->card_board[current_node->children[cc]->dtop[i]][i].suit;
                current_node->children[cc]->foundations[current_node->children[cc]->ftop[s]][s].value=current_node->card_board[current_node->children[cc]->dtop[i]][i].value;
                current_node->children[cc]->card_board[current_node->children[cc]->dtop[i]][i].suit=' ';
                current_node->children[cc]->card_board[current_node->children[cc]->dtop[i]][i].value=0;
                if(current_node->children[cc]->dtop[i]!=0){
                    current_node->children[cc]->dtop[i]=current_node->children[cc]->dtop[i]-1;
                }
                current_node->children[cc]->ftop[s]=current_node->children[cc]->ftop[s]+1;


                if(check_with_parents(current_node->children[cc])==0){
                    free(current_node->children[cc]);
                    current_node->children[cc]=NULL;
                    cc--;
                }
                else{
                    current_node->children[cc]->h=0; //prepei na sumplirwsw tin euristiki
                    if(method==best){
                        current_node->children[cc]->f=current_node->children[cc]->h;
                    }
                    else if(method==astar){
                        current_node->children[cc]->f=current_node->children[cc]->h + current_node->children[cc]->g;
                    }
                    else{
                        current_node->children[cc]->h=0;
                    }
                }
                cc++;
            }}
            else{
                if((current_node->card_board[current_node->dtop[i]][i].value-1) == current_node->foundations[current_node->ftop[s]-1][s].value){

                current_node->children[cc]= (struct tree_node*) malloc(sizeof(struct tree_node));
                for(j=0;j<20;j++){
                    current_node->children[cc]->children[j]=NULL;
                }
                for(j=0;j<3*N/2;j++){
                    for(jj=0;jj<8;jj++){
                        current_node->children[cc]->card_board[j][jj].suit=current_node->card_board[j][jj].suit;
                        current_node->children[cc]->card_board[j][jj].value=current_node->card_board[j][jj].value;
                        current_node->children[cc]->dtop[jj]=current_node->dtop[jj];
                    }
                }
                for(j=0;j<4;j++){
                    current_node->children[cc]->freecells[j].suit=current_node->freecells[j].suit;
                    current_node->children[cc]->freecells[j].value=current_node->freecells[j].value;
                }
                for(j=0;j<N;j++){
                    for(jj=0;jj<4;jj++){
                        current_node->children[cc]->foundations[j][jj].suit=current_node->foundations[j][jj].suit;
                        current_node->children[cc]->foundations[j][jj].value=current_node->foundations[j][jj].value;
                        current_node->children[cc]->ftop[jj]=current_node->ftop[jj];
                    }
                }
                for(j=0;j<8;j++){
                    current_node->children[cc]->dtop[j]=current_node->dtop[j];
                }

                current_node->children[cc]->parent=current_node;
                current_node->children[cc]->move=foundation;
                current_node->children[cc]->g=current_node->g+1;

                current_node->children[cc]->foundations[current_node->children[cc]->ftop[s]][s].suit=current_node->card_board[current_node->children[cc]->dtop[i]][i].suit;
                current_node->children[cc]->foundations[current_node->children[cc]->ftop[s]][s].value=current_node->card_board[current_node->children[cc]->dtop[i]][i].value;
                current_node->children[cc]->card_board[current_node->children[cc]->dtop[i]][i].suit=' ';
                current_node->children[cc]->card_board[current_node->children[cc]->dtop[i]][i].value=0;
                if(current_node->children[cc]->dtop[i]!=0){
                    current_node->children[cc]->dtop[i]=current_node->children[cc]->dtop[i]-1;
                }
                current_node->children[cc]->ftop[s]=current_node->children[cc]->ftop[s]+1;


                if(check_with_parents(current_node->children[cc])==0){
                    free(current_node->children[cc]);
                    current_node->children[cc]=NULL;
                    cc--;
                }
                else{
                    current_node->children[cc]->h=0; //prepei na sumplirwsw tin euristiki
                    if(method==best){
                        current_node->children[cc]->f=current_node->children[cc]->h;
                    }
                    else if(method==astar){
                        current_node->children[cc]->f=current_node->children[cc]->h + current_node->children[cc]->g;
                    }
                    else{
                        current_node->children[cc]->h=0;
                    }
                }
                cc++;
            }
            }
    }
    for(i=0;i<8;i++){
        for(k=0;k<8;k++){
            if(current_node->card_board[current_node->dtop[i]][i].suit=='H')
                s=0;
            if(current_node->card_board[current_node->dtop[i]][i].suit=='D')
                s=1;
            if(current_node->card_board[current_node->dtop[i]][i].suit=='S')
                s=2;
            if(current_node->card_board[current_node->dtop[i]][i].suit=='C')
                s=3;
            if(current_node->card_board[current_node->dtop[k]][k].suit=='H')
                s2=0;
            if(current_node->card_board[current_node->dtop[k]][k].suit=='D')
                s2=1;
            if(current_node->card_board[current_node->dtop[k]][k].suit=='S')
                s2=2;
            if(current_node->card_board[current_node->dtop[k]][k].suit=='C')
                s2=3;
            
            if(s==0 || s==1){
                if(current_node->card_board[current_node->dtop[i]][i].value == current_node->card_board[current_node->dtop[k]][k].value-1 || current_node->card_board[current_node->dtop[k]][k].value==0 && (s2==2 || s2==3)){
                    
                    current_node->children[cc] = (struct tree_node*) malloc(sizeof(struct tree_node));
                    for(j=0;j<20;j++){
                        current_node->children[cc]->children[j]=NULL;
                    }
                    for(j=0;j<3*N/2;j++){
                        for(jj=0;jj<8;jj++){
                            current_node->children[cc]->card_board[j][jj].suit=current_node->card_board[j][jj].suit;
                            current_node->children[cc]->card_board[j][jj].value=current_node->card_board[j][jj].value;
                            current_node->children[cc]->dtop[jj]=current_node->dtop[jj];
                        }
                    }
                    for(j=0;j<4;j++){
                        current_node->children[cc]->freecells[j].suit=current_node->freecells[j].suit;
                        current_node->children[cc]->freecells[j].value=current_node->freecells[j].value;
                    }
                    for(j=0;j<N;j++){
                        for(jj=0;jj<4;jj++){
                            current_node->children[cc]->foundations[j][jj].suit=current_node->foundations[j][jj].suit;
                            current_node->children[cc]->foundations[j][jj].value=current_node->foundations[j][jj].value;
                            current_node->children[cc]->ftop[jj]=current_node->ftop[jj];
                        }
                    }
                    
                    current_node->children[cc]->parent=current_node;
                    current_node->children[cc]->move=stack;
                    current_node->children[cc]->g=current_node->g+1;

                    current_node->children[cc]->card_board[current_node->children[cc]->dtop[k]+1][k].suit=current_node->card_board[current_node->children[cc]->dtop[i]][i].suit;
                    current_node->children[cc]->card_board[current_node->children[cc]->dtop[k]+1][k].value=current_node->card_board[current_node->children[cc]->dtop[i]][i].value;
                    current_node->children[cc]->card_board[current_node->children[cc]->dtop[i]][i].suit=' ';
                    current_node->children[cc]->card_board[current_node->children[cc]->dtop[i]][i].value=0;
                    if(current_node->children[cc]->dtop[i]!=0){
                        current_node->children[cc]->dtop[i]=current_node->children[cc]->dtop[i]-1;
                    }
                    current_node->children[cc]->dtop[k]=current_node->children[cc]->dtop[k]+1;

                    if(check_with_parents(current_node->children[cc])==0){
                    free(current_node->children[cc]);
                    current_node->children[cc]=NULL;
                    cc--;
                    }
                    else{
                        current_node->children[cc]->h=0; //prepei na sumplirwsw tin euristiki
                        if(method==best){
                            current_node->children[cc]->f=current_node->children[cc]->h;
                        }
                        else if(method==astar){
                            current_node->children[cc]->f=current_node->children[cc]->h + current_node->children[cc]->g;
                        }
                        else{
                            current_node->children[cc]->h=0;
                        }
                    }
                    cc++;
                }
            }
            else{
                if(current_node->card_board[current_node->dtop[i]][i].value == current_node->card_board[current_node->dtop[k]][k].value-1 || current_node->card_board[current_node->dtop[k]][k].value==0 && (s2==0 || s2==1)){
                    
                    current_node->children[cc] = (struct tree_node*) malloc(sizeof(struct tree_node));
                    for(j=0;j<20;j++){
                        current_node->children[cc]->children[j]=NULL;
                    }
                    for(j=0;j<3*N/2;j++){
                        for(jj=0;jj<8;jj++){
                            current_node->children[cc]->card_board[j][jj].suit=current_node->card_board[j][jj].suit;
                            current_node->children[cc]->card_board[j][jj].value=current_node->card_board[j][jj].value;
                            current_node->children[cc]->dtop[jj]=current_node->dtop[jj];
                        }
                    }
                    for(j=0;j<4;j++){
                        current_node->children[cc]->freecells[j].suit=current_node->freecells[j].suit;
                        current_node->children[cc]->freecells[j].value=current_node->freecells[j].value;
                    }
                    for(j=0;j<N;j++){
                        for(jj=0;jj<4;jj++){
                            current_node->children[cc]->foundations[j][jj].suit=current_node->foundations[j][jj].suit;
                            current_node->children[cc]->foundations[j][jj].value=current_node->foundations[j][jj].value;
                            current_node->children[cc]->ftop[jj]=current_node->ftop[jj];
                        }
                    }

                    current_node->children[cc]->parent=current_node;
                    current_node->children[cc]->move=stack;
                    current_node->children[cc]->g=current_node->g+1;

                    current_node->children[cc]->card_board[current_node->children[cc]->dtop[k]+1][k].suit=current_node->card_board[current_node->children[cc]->dtop[i]][i].suit;
                    current_node->children[cc]->card_board[current_node->children[cc]->dtop[k]+1][k].value=current_node->card_board[current_node->children[cc]->dtop[i]][i].value;
                    current_node->children[cc]->card_board[current_node->children[cc]->dtop[i]][i].suit=' ';
                    current_node->children[cc]->card_board[current_node->children[cc]->dtop[i]][i].value=0;
                    if(current_node->children[cc]->dtop[i]!=0){
                        current_node->children[cc]->dtop[i]=current_node->children[cc]->dtop[i]-1;
                    }
                    current_node->children[cc]->dtop[k]=current_node->children[cc]->dtop[k]+1;

                    if(check_with_parents(current_node->children[cc])==0){
                    free(current_node->children[cc]);
                    current_node->children[cc]=NULL;
                    cc--;
                    }
                    else{
                        current_node->children[cc]->h=0; //prepei na sumplirwsw tin euristiki
                        if(method==best){
                            current_node->children[cc]->f=current_node->children[cc]->h;
                        }
                        else if(method==astar){
                            current_node->children[cc]->f=current_node->children[cc]->h + current_node->children[cc]->g;
                        }
                        else{
                            current_node->children[cc]->h=0;
                        }
                    }
                    cc++;
                }
            }
        }
    }
    for(i=0;i<8;i++){
        for(k=0;k<4;k++){
            if(current_node->freecells[k].value==0){
                current_node->children[cc] = (struct tree_node*) malloc(sizeof(struct tree_node));
                    for(j=0;j<20;j++){
                        current_node->children[cc]->children[j]=NULL;
                    }
                    for(j=0;j<3*N/2;j++){
                        for(jj=0;jj<8;jj++){
                            current_node->children[cc]->card_board[j][jj].suit=current_node->card_board[j][jj].suit;
                            current_node->children[cc]->card_board[j][jj].value=current_node->card_board[j][jj].value;
                            current_node->children[cc]->dtop[jj]=current_node->dtop[jj];
                        }
                    }
                    for(j=0;j<4;j++){
                        current_node->children[cc]->freecells[j].suit=current_node->freecells[j].suit;
                        current_node->children[cc]->freecells[j].value=current_node->freecells[j].value;
                    }
                    for(j=0;j<N;j++){
                        for(jj=0;jj<4;jj++){
                            current_node->children[cc]->foundations[j][jj].suit=current_node->foundations[j][jj].suit;
                            current_node->children[cc]->foundations[j][jj].value=current_node->foundations[j][jj].value;
                            current_node->children[cc]->ftop[jj]=current_node->ftop[jj];
                        }
                    }

                current_node->children[cc]->parent=current_node;
                current_node->children[cc]->move=stack;
                current_node->children[cc]->g=current_node->g+1;

                current_node->children[cc]->freecells[k].suit=current_node->card_board[current_node->children[cc]->dtop[i]][i].suit;
                current_node->children[cc]->freecells[k].value=current_node->card_board[current_node->children[cc]->dtop[i]][i].value;
                current_node->children[cc]->card_board[current_node->children[cc]->dtop[i]][i].suit=' ';
                current_node->children[cc]->card_board[current_node->children[cc]->dtop[i]][i].value=0;
                if(current_node->children[cc]->dtop[i]!=0){
                    current_node->children[cc]->dtop[i]=current_node->children[cc]->dtop[i]-1;
                }

                if(check_with_parents(current_node->children[cc])==0){
                    free(current_node->children[cc]);
                    current_node->children[cc]=NULL;
                    cc--;
                }
                else{
                    current_node->children[cc]->h=0; //prepei na sumplirwsw tin euristiki
                    if(method==best){
                        current_node->children[cc]->f=current_node->children[cc]->h;
                    }
                    else if(method==astar){
                        current_node->children[cc]->f=current_node->children[cc]->h + current_node->children[cc]->g;
                    }
                    else{
                        current_node->children[cc]->h=0;
                    }
                }
                cc++;
            }
        }
    }
    /*for(i=0;i<8;i++){
        for(k=0;k<4;k++){
            if(current_node->freecells[k].value!=0){
                if(current_node->freecells[k].suit=='H')
                    s=0;
                if(current_node->freecells[k].suit=='D')
                    s=1;
                if(current_node->freecells[k].suit=='S')
                    s=2;
                if(current_node->freecells[k].suit=='C')
                    s=3;
                if(current_node->card_board[current_node->dtop[i]][i].suit=='H')
                    s2=0;
                if(current_node->card_board[current_node->dtop[i]][i].suit=='D')
                    s2=1;
                if(current_node->card_board[current_node->dtop[i]][i].suit=='S')
                    s2=2;
                if(current_node->card_board[current_node->dtop[i]][i].suit=='C')
                    s2=3;
                if((s==0 || s==1 && s2==2 ||s2==3) || (s==2 || s==3 && s2==0 ||s2==1)){
                    if(current_node->freecells[k].value == current_node->foundations[ftop[s]][s].value || current_node->freecells[k].value == current_node->card_board[current_node->dtop[i]][i].value-1 || current_node->card_board[current_node->dtop[i]][i].value==0){
                        current_node->children[cc] = (struct tree_node*) malloc(sizeof(struct tree_node));
                        for(j=0;j<20;j++){
                            current_node->children[cc]->children[j]=NULL;
                        }
                        for(j=0;j<3*N/2;j++){
                            for(jj=0;jj<8;jj++){
                                current_node->children[cc]->card_board[j][jj].suit=current_node->card_board[j][jj].suit;
                                current_node->children[cc]->card_board[j][jj].value=current_node->card_board[j][jj].value;
                                current_node->children[cc]->dtop[jj]=current_node->dtop[jj];
                            }
                        }
                        for(j=0;j<4;j++){
                            current_node->children[cc]->freecells[j].suit=current_node->freecells[j].suit;
                            current_node->children[cc]->freecells[j].value=current_node->freecells[j].value;
                        }
                        for(j=0;j<N;j++){
                            for(jj=0;jj<4;jj++){
                                current_node->children[cc]->foundations[j][jj].suit=current_node->foundations[j][jj].suit;
                                current_node->children[cc]->foundations[j][jj].value=current_node->foundations[j][jj].value;
                                current_node->children[cc]->ftop[jj]=current_node->ftop[jj];
                            }
                        }

                        current_node->children[cc]->parent=current_node;
                        current_node->children[cc]->move=stack;
                        current_node->children[cc]->g=current_node->g+1;

                        if(current_node->freecells[k].value-1 == current_node->foundations[ftop[s]-1][s].value){
                            current_node->children[cc]->foundations[ftop[s]][s].suit=current_node->freecells[k].suit;
                            current_node->children[cc]->foundations[ftop[s]][s].value=current_node->freecells[k].value;
                            current_node->freecells[k].suit=' ';
                            current_node->freecells[k].value=0;
                            current_node->children[cc]->ftop[s]=current_node->children[cc]->ftop[s]+1;
                        }
                        if(current_node->freecells[k].value == current_node->card_board[current_node->dtop[i]][i].value-1){
                            current_node->children[cc]->card_board[dtop[i]][i].suit=current_node->freecells[k].suit;
                            current_node->children[cc]->card_board[dtop[i]][i].value=current_node->freecells[k].value;
                            current_node->freecells[k].suit=' ';
                            current_node->freecells[k].value=0;
                            current_node->children[cc]->dtop[i]=current_node->children[cc]->dtop[i]+1;
                        }

                        if(check_with_parents(current_node->children[cc])==0){
                            free(current_node->children[cc]);
                            current_node->children[cc]=NULL;
                            cc--;
                        }
                        else{
                            current_node->children[cc]->h=0; //prepei na sumplirwsw tin euristiki
                            if(method==best){
                                current_node->children[cc]->f=current_node->children[cc]->h;
                            }
                            else if(method==astar){
                                current_node->children[cc]->f=current_node->children[cc]->h + current_node->children[cc]->g;
                            }
                            else{
                                current_node->children[cc]->h=0;
                            }
                        }
                        cc++;
                    }
                }
            }
        }
    }*/
    return;
}

struct tree_node *search(int method)
{
    clock_t t;
    int i,err;
    struct frontier_node *temp_frontier_node;
    struct tree_node *current_node;

    while(frontier_head!=NULL){
        t=clock();
        if(t-t1>CLOCKS_PER_SEC*TIMEOUT){
            printf("Timeout\n");
            return NULL;
        }
    

        current_node = frontier_head->n;
        printf("Extracted from frontier...\n");
        display_puzzle(current_node);

        if(is_solution2(current_node)){
            return current_node;
        }
        
        temp_frontier_node=frontier_head;
        frontier_head=frontier_head->next;
        free(temp_frontier_node);
        if (frontier_head==NULL){
            frontier_tail=NULL;
        }

        find_children(current_node, method);

        for(i=0;i<20;i++)
            if (current_node->children[i]!=NULL)
            {
                if (method==depth)
                    err=add_frontier_front(current_node->children[i]);
                else if (method==breadth)
                    err=add_frontier_back(current_node->children[i]);
                else if (method==best)
                    err=add_frontier_in_order(current_node->children[i]);
                else if (method==astar)
                    err=add_frontier_in_order(current_node->children[i]);
                if (err<0)
                {
                    printf("Memory exhausted while creating new frontier node. Search is terminated...\n");
                    return NULL;
                }
            }
    }

    return NULL;
}

int add_frontier_front(struct tree_node *node)
{
    struct frontier_node *new_frontier_node = (struct frontier_node*) malloc(sizeof(struct frontier_node));
    if(new_frontier_node==NULL)
        return-1;

    new_frontier_node->n=node;
    new_frontier_node->next=frontier_head;

    if(frontier_head==NULL){
        frontier_head=new_frontier_node;
        frontier_tail=new_frontier_node;
    }
    else{
        frontier_head=new_frontier_node;
    }

    printf("Added to the front...\n");
    display_puzzle(node);

    return 0;
}

int add_frontier_back(struct tree_node *node)
{
    struct frontier_node *new_frontier_node = (struct frontier_node*) malloc(sizeof(struct frontier_node));
    if(new_frontier_node==NULL)
        return-1;

    new_frontier_node->n=node;
    new_frontier_node->next==NULL;

    if(frontier_tail==NULL){
        frontier_head=new_frontier_node;
        frontier_tail=new_frontier_node;
    }
    else{
        frontier_tail->next=new_frontier_node;
        frontier_tail=new_frontier_node;
    }

    printf("Added to the back...\n");
    display_puzzle(node);

    return 0;
}

int add_frontier_in_order(struct tree_node *node)
{
    struct frontier_node *new_frontier_node = (struct frontier_node*) malloc(sizeof(struct frontier_node));
    if(new_frontier_node==NULL)
        return-1;

    new_frontier_node->n=node;
    new_frontier_node->next==NULL;

    if(frontier_head==NULL){
        frontier_head=new_frontier_node;
        frontier_tail=new_frontier_node;
    }
    else{
        struct frontier_node *pt,*pt_previous;
        pt = frontier_head;
        pt_previous=NULL;

        while(pt!=NULL && (pt->n->f < node->f || (pt->n->f == node->f && pt->n->h < node->h))){
            pt_previous=pt;
            pt=pt->next;
        }
        if(pt!=NULL){
            if(pt_previous==NULL){
                new_frontier_node->next=pt;
                frontier_head=new_frontier_node;
            }
            else{
                new_frontier_node->next=pt;
                pt_previous->next=new_frontier_node;
            }
        }
        else{
            frontier_tail->next=new_frontier_node;
            frontier_tail=new_frontier_node;
        }
    }

    printf("Added in order (f=%d)...\n",node->f);
    display_puzzle(node);

    return 0;

}

void initialize_search(struct card deck[3*N/2][8],int method)
{
    struct tree_node *root=NULL;
    int i,j,q;

    root = (struct tree_node*) malloc(sizeof(struct tree_node));
    root->parent=NULL;
    root->move=-1;

    for(q=0;q<20;q++){
        root->children[q]=NULL;
    }
    for(i=0;i<3*N/2;i++){
        for(j=0;j<8;j++){
            root->card_board[i][j]=deck[i][j];
        }
    }
    for(i=0;i<4;i++){
        root->freecells[i].suit=' ';
        root->freecells[i].value=0;
    }
    for(i=0;i<N;i++){
        for(j=0;j<4;j++){
            root->foundations[i][j].suit=' ';
            root->foundations[i][j].value=0;
        }
    }
    for(i=0;i<8;i++){
        for(j=0;j<3*N/2;j++){
            if(deck[j][i].value == 0){
                root->dtop[i]=j-1;
                 break;
            }
        }
    }

    for(i=0;i<4;i++){
            root->ftop[i]=0;
    }

    root->g=0;
    root->h=0; //Exw na sumplirwsw tin euretiki sunartisi
    if(method==best){
        root->f=root->g;
    }
    else if(method==astar){
        root->f=root->g+root->h;
    }
    else{
        root->f=0;
    }
    
    add_frontier_front(root);
}

int get_method(char* s)
{
	if (strcmp(s,"breadth")==0)
		return  breadth;
	else if (strcmp(s,"depth")==0)
		return depth;
	else if (strcmp(s,"best")==0)
		return best;
	else if (strcmp(s,"astar")==0)
		return astar;
	else
		return -1;
}

int read_puzzle(char* filename,struct card deck[3*N/2][8])
{
	FILE *fin;
	int i,x,y,err,v;
	char c,space;
	fin=fopen(filename, "r");
	if (fin==NULL)
	{
		#ifdef SHOW_COMMENTS
			printf("Cannot open file %s. Program terminates.\n",filename);
		#endif
		return -1;
	}
    for(i=0;i<3*N/2;i++){
        for(x=0;x<8;x++){
            deck[i][x].suit=' ';
            deck[i][x].value=0;
        }
    }
    x=y=0;
	for(i=1;i<=4*N;i++){
        err=fscanf(fin,"%c%d%c",&c,&v,&space);
        if(err != EOF){
            deck[x][y].suit=c;
            deck[x][y].value=v;
        }
        y++;
        if(i%8==0){
            x++;
            y=0;
        }
    }

	fclose(fin);
}

void display_puzzle(struct tree_node *node)
{
    int i,j;
    printf("Card Board------------\n");
    for(i=0;i<3*N/2;i++){
        for(j=0;j<8;j++){
            printf("%c%d ",node->card_board[i][j].suit,node->card_board[i][j].value);
        }printf("\n");
    }
    printf("Freecells-------------\n");
    for(i=0;i<4;i++){
        printf("%c%d ",node->freecells[i].suit,node->freecells[i].value);
    }printf("\n");
    printf("Foundations-----------\n");
    for(i=0;i<N;i++){
        for(j=0;j<4;j++){
            printf("%c%d ",node->foundations[i][j].suit,node->foundations[i][j].value);
        }printf("\n");
    }
}

int is_solution(struct card foundations[N][4])
{
    int i,j,s;

    for(i=0;i<N;i++){
        for(j=0;j<4;j++){
            if(foundations[i][j].suit=='H'){
                s=0;
            }
            if(foundations[i][j].suit=='D'){
                s=1;
            }
            if(foundations[i][j].suit=='S'){
                s=2;
            }
            if(foundations[i][j].suit=='C'){
                s=3;
            }
            if(i != (foundations[i][j].value-1) || j!=s)
                return 0;
        }
    }
    return 1;
}

int check_with_parents(struct tree_node *new_node)
{
    struct tree_node *parent = new_node->parent;
    
    while(parent!=NULL){
        if(equal_puzzles(new_node,parent)){
            return 0;
        }
        parent=parent->parent;
    }
    return 1;
}

int equal_puzzles(struct tree_node *new_node,struct tree_node *parent)
{
    int i,j;
    for(i=0;i<3*N/2;i++){
        for(j=0;j<8;j++){
            if (new_node->card_board[i][j].suit == parent->card_board[i][j].suit && new_node->card_board[i][j].value == parent->card_board[i][j].value)
                return 0;
        }
    }
    for(i=0;i<N;i++){
        for(j=0;j<4;j++){
            if(new_node->foundations[i][j].suit == parent->foundations[i][j].suit && new_node->foundations[i][j].value == parent->foundations[i][j].value)
                return 0;
        }
    }
    for(i=0;i<4;i++){
        if(new_node->freecells[i].suit == parent->freecells[i].suit && new_node->freecells[i].value == parent->freecells[i].value)
            return 0;
    }

    return 1;
}

void syntax_message()
{
    printf("puzzle <method> <input-file> <output-file>\n\n");
    printf("where: ");
    printf("<method> = breadth|depth|best|astar\n");
    printf("<input-file> is a file containing a %dx%d puzzle description.\n",N,N);
    printf("<output-file> is the file where the solution will be written.\n");
}
