// -------------------------------------------------------------
//
// This program solves the Free Solitaire game using four algorithms:
// - Depth first search
// - Breadth first search
// - Best first search
// - A*
// N is defined as a constant.
// Cardboards are read from an input file, while solution is written
// to an output file.
//
// Author: Karamoulas Eleftherios, May 2015
//
// --------------------------------------------------------------



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define N 5

#define HEARTS 0
#define DIAMONDS 1
#define SPADES 2
#define CLUBS 3

#define breadth 1		// Constants denoting the four algorithms
#define depth	2
#define best	3
#define astar	4

#define freecell 0
#define stack 1
#define new_stack 2
#define foundation 3

clock_t t1;					// Start time of the search algorithm
clock_t t2;					// End time of the search algorithm
#define TIMEOUT		120		// Program terminates after TIMOUT secs

//int global_counter=0;

struct card{
    
    int suit;
    int value;
    
};
struct tree_node
{
    struct card card_board[16][(3*N)/2];
    int top[16];
    int h;				// the value of the heuristic function for this node
    int g;				// the depth of this node wrt the root of the search tree
    int f;				// f=0 or f=h or f=h+g, depending on the search algorithm used.
    struct tree_node *parent;	// pointer to the parrent node (NULL for the root).
    int move;				//The kind of move made in order to reach this state
    //Defined by global variables at the top of the file(freecell etc.)
    struct card mcard0,mcard1; //Mcard0 is the card moved at the stack over the mcard1
    struct tree_node *children[20];// pointer to a table with 4 pointers to the childerns (NULL if no child).
    // The four pointers in this table correspond to the four directions in which
    // the blank can be moved, accordning to the constants left, right, up and down (see above).
};

//A struct in order to use it for the global dynamic table with the moves of the solution
struct solution_move{
    int move;
    struct card card0,card1;
};

// A node of the frontier. Frontier is kept as a double-linked list,
// for efficiency reasons for the breadth-first search algorithm.
struct frontier_node{
    struct tree_node *n;				// pointer to a search-tree node
    struct frontier_node *previous;		// pointer to the previous frontier node
    struct frontier_node *next;			// pointer to the next frontier node
};

struct frontier_node *frontier_head=NULL;	// The one end of the frontier
struct frontier_node *frontier_tail=NULL;	// The other end of the frontier



int solution_length;	// The lenght of the solution table.
struct solution_move *solution;			// Pointer to a dynamic table with the moves of the solution.


//Definition of all the functions of the file for a better look of the arguments
void display_cardboard(struct tree_node *node);
int read_freecell(char* filename, struct card card_board[16][(3*N)/2],int top[16]);
int get_method(char* s);
int equal_cardboard(struct tree_node *node0, struct tree_node *node1);
int freecell_count(struct tree_node *node);
void evaluate_child(struct tree_node *child_node, int method);
int is_solution(struct tree_node *node);
int check_with_parents(struct tree_node *new_node);
void find_children(struct tree_node *current_node, int method);
void create_child(struct tree_node *current_node, int move, int child, int mcard0_pos, int mcard1_pos, int method);
int heuristic(struct tree_node *node);
int add_frontier_front(struct tree_node *node);
int add_frontier_back(struct tree_node *node);
int add_frontier_in_order(struct tree_node *node);
void initialize_search(struct card cb[16][(3*N)/2], int top[16], int method);
void extract_solution(struct tree_node *solution_node);
void write_solution_to_file(char* filename, int solution_length, struct solution_move *solution);
struct tree_node *search(int method);
void syntax_message();
int freestacks_count(struct tree_node *node);
int num_cards_at_foundations(struct tree_node *node);

//Function in order to check the cardboard of a given node with terminal
void display_cardboard(struct tree_node *node)
{
    int i, j;
    for (i = 0; i < 16; i++)
    {
        for (j = 0; j < N * 4; j++)
        {
            if (node->card_board[i][j].value != -1)
            {
                if (node->card_board[i][j].suit == 0)
                    printf("H");
                else if (node->card_board[i][j].suit == 1)
                    printf("D");
                else if (node->card_board[i][j].suit == 2)
                    printf("S");
                else if (node->card_board[i][j].suit == 3)
                    printf("C");
                
                printf("%d ", node->card_board[i][j].value);
            }
        }
        printf("top: %d\n", node->top[i]);
    }
}

// This function reads a file containing a cardboard and stores the values and suits
// in the global variable struct card card_board[16][(3*N)/2].
// Inputs:
//		char* filename	: The name of the file containing a N constant cardboard.
// Output:
//		0 --> Successful read.
//		1 --> Unsuccessful read

int read_freecell(char* filename, struct card card_board[16][(3*N)/2],int top[16])
{
    FILE *fin;
    int i,j,err;
    char temp;
    
    fin=fopen(filename,"r");
    if(fin==NULL)
    {
        printf("Cannot open file %s. Program terminates.\n",filename);
        return -1;
    }
    //Initialization of the card board
    for (i=0;i<16;i++)
    {
        for (j=0;j<(3*N)/2;j++)
        {
            card_board[i][j].suit=-1;
            card_board[i][j].value=-1;
        }
        top[i]=-1;
    }
    //Filling the first 8 stacks of the cardboard from the given file and converting chars to ints
    //Computations in order to make it possible to run for diferrent values of the constant N
    for (i=0;i<8;i++)
    {
        if((N%2)==1 & i<4)
        {
            for (j=0;j<(N/2)+1;j++)
            {
                err=fscanf(fin, "%c", &temp);
                if (err<1)
                {
#ifdef SHOW_COMMENTS
                    printf("Cannot read item [%d][%d] of the cardboard. Program terminates.\n",i,j);
#endif
                    fclose(fin);
                    return -1;
                }
                switch (temp)
                {
                    case 'H': card_board[i][j].suit=0;
                        break;
                    case 'D': card_board[i][j].suit=1;
                        break;
                    case 'S': card_board[i][j].suit=2;
                        break;
                    case 'C': card_board[i][j].suit=3;
                        break;
                }
                err=fscanf(fin,"%d ",&card_board[i][j].value);
                if (err<1)
                {
#ifdef SHOW_COMMENTS
                    printf("Cannot read item [%d][%d] of the cardboard. Program terminates.\n",i,j);
#endif
                    fclose(fin);
                    return -1;
                }
                top[i]++;
            }
        }
        else
        {
            for (j=0;j<N/2;j++)
            {
                err=fscanf(fin, "%c", &temp);
                if (err<1)
                {
#ifdef SHOW_COMMENTS
                    printf("Cannot read item [%d][%d] of the cardboard. Program terminates.\n",i,j);
#endif
                    fclose(fin);
                    return -1;
                }
                switch (temp)
                {
                    case 'H': card_board[i][j].suit=0;
                        break;
                    case 'D': card_board[i][j].suit=1;
                        break;
                    case 'S': card_board[i][j].suit=2;
                        break;
                    case 'C': card_board[i][j].suit=3;
                        break;
                }
                err=fscanf(fin,"%d ",&card_board[i][j].value);
                if (err<1)
                {
#ifdef SHOW_COMMENTS
                    printf("Cannot read item [%d][%d] of the cardboard. Program terminates.\n",i,j);
#endif
                    fclose(fin);
                    return -1;
                }
                top[i]++;
            }
        }
    }
    fclose(fin);
    return 0;
}

// Reading run-time parameters.
int get_method(char* s)
{
    if(strcmp(s,"breadth")==0)
        return breadth;
    else if (strcmp(s,"depth")==0)
        return depth;
    else if (strcmp(s,"best")==0)
        return best;
    else if (strcmp(s,"astar")==0)
        return astar;
    else
        return -1;
}

// This function checks whether two nodes have the same cardboards.
//The check containing a awkward mechanism in order to provide
//that the position that a card will be in a freecell does not matter
//and the nodes are equal even if the one has one card on the freecell
//number 0 and the other has the same card at the freecell number 3
//with the same stack states
// Inputs:
//		memory value of the node0
//		memory value of the node1
// Output:
//		1 --> the cardboards of the nodes are equal
//		0 --> the cardboards of the nodes are not equal

int equal_cardboard(struct tree_node *node0, struct tree_node *node1)
{
    for(int i=0;i<16;i++)
        for(int j=0;j<(3*N)/2;j++)
        {
            {
                if(node0->card_board[i][j].value!=node1->card_board[i][j].value || node0->card_board[i][j].suit!=node1->card_board[i][j].suit)
                    return 0;
            }
            
        }
    
    return 1;
    
}
/*
 int equal_cardboard(struct tree_node *node0, struct tree_node *node1)
 {
	struct card freecell1[3],freecell2[3];
	int i,j,controller,resulter;
	for(i=0;i<16;i++)
 for(j=0;j<(3*N)/2;j++)
 {
 if(i>7 && i<12)
 {
 freecell1[i-8]=node0->card_board[i][0];
 freecell2[i-8]=node1->card_board[i][0];
 continue;
 }
 else if (node0->card_board[i][j].value!=node1->card_board[i][j].value || node0->card_board[i][j].suit!=node1->card_board[i][j].suit)
 return 0;
 }
	controller=0;
	resulter=0;
	for(i=0;i<4;i++)
	{
 for(j=0;j<4;j++)
 {
 if(freecell1[i].value==freecell2[j].value && freecell1[i].suit==freecell2[j].suit)
 {
 controller++;
 resulter++;
 if(controller>1)
 {
 resulter--;
 continue;
 }
 }
 }
 controller=0;
	}
	if(resulter==4)
	{
 return 1;
	}
	else return 0;
 }
 */
/*
 int equal_cardboard(struct tree_node *node0, struct tree_node *node1)
 {
	int counter,inside_counter, equ_card_counter;
	counter=0;
	for(int i=12;i<16;i++)
	{
 inside_counter=0;
 for(int y=12;y<16;y++)
 {
 equ_card_counter=0;
 for(int j=0;j<(3*N)/2;j++)
 {
 if(node0->card_board[y][j].value!=node1->card_board[i][j].value || node0->card_board[y][j].suit!=node1->card_board[i][j].suit)
 j=(3*N)/2;
 else
 equ_card_counter++;
 }
 if (equ_card_counter==(3*N)/2)
 inside_counter++;
 }
 if (inside_counter==1)
 counter++;
	}
	if (counter!=4)
 return 0;
 
	counter=0;
	for(int i=8;i<12;i++)
	{
 inside_counter=0;
 for(int y=8;y<12;y++)
 {
 equ_card_counter=0;
 for(int j=0;j<(3*N)/2;j++)
 {
 if(node0->card_board[y][j].value!=node1->card_board[i][j].value || node0->card_board[y][j].suit!=node1->card_board[i][j].suit)
 break;
 else
 equ_card_counter++;
 }
 if (equ_card_counter==(3*N)/2)
 inside_counter++;
 }
 if (inside_counter==1)
 counter++;
	}
	if (counter!=4)
 return 0;
 
	counter=0;
	for(int i=0;i<8;i++)
	{
 inside_counter=0;
 for(int y=0;y<8;y++)
 {
 equ_card_counter=0;
 for(int j=0;j<(3*N)/2;j++)
 {
 if(node0->card_board[y][j].value!=node1->card_board[i][j].value || node0->card_board[y][j].suit!=node1->card_board[i][j].suit)
 break;
 else
 equ_card_counter++;
 }
 if (equ_card_counter==(3*N)/2)
 inside_counter++;
 }
 if (inside_counter==1)
 counter++;
	}
	if (counter!=8)
 return 0;
 
 return 1;
 
 }
 */

// Evaluates the child node generated by
// computing the evaluation function value based on the search method used.
void evaluate_child(struct tree_node *child_node, int method)
{
    if (method==best)
        child_node->f=heuristic(child_node);
    else if (method==astar)
        child_node->f=child_node->g + heuristic(child_node);
    else
        child_node->f=0;
}



// This function checks whether a cardboard of a node
// is a solution cardboard.
// Inputs:
//		one memory value of a node
// Outputs:
//		1 --> The cardboard of the node is a solution cardboard
//		0 --> The cardboard of the node is NOT a solution cardboard
int is_solution(struct tree_node *node)
{
    int i;
    for(i=12;i<16;i++)
    {
        if(node->top[i]==N-1)
            continue;
        else return 0;
    }
    return 1;
}


// This function checks whether a node in the search tree
// holds exactly the same cardboard with at least one of its
// predecessors. This function is used when creating the childs
// of an existing search tree node, in order to check for each one of the childs
// whether this appears in the path from the root to its parent.
// This is a moderate way to detect loops in the search.
// Inputs:
//		struct tree_node *new_node	: A search tree node (usually a new one)
// Output:
//		1 --> No coincidence with any predecessor
//		0 --> Loop detection
int check_with_parents(struct tree_node *new_node)
{
    struct tree_node *parent=new_node->parent;
    while (parent!=NULL)
    {
        if (equal_cardboard(new_node,parent))
            return 0;
        parent=parent->parent;
    }
    return 1;
}

// This function expands a leaf-node of the search tree.
// A leaf-node may have up to 20 childs. A table with 20 pointers
// to these childs is created, with NULLs for those childrens that do not exist.
// In case no child exists (due to loop-detections), the table is not created
// and a 'higher-level' NULL indicates this situation.
// Inputs:
//		struct tree_node *current_node	: A leaf-node of the search tree.
// Output:
//		The same leaf-node expanded with pointers to its children (if any).
void find_children(struct tree_node *current_node, int method)
{
    int i, j, child;
    for (i = 0; i < 20; i++)
    {
        current_node->children[i] = NULL;
    }
    
    child=0;
    for(i=0;i<12;i++)
    {
        if(current_node->top[i]!=-1)
        {
            //first I check for foundations(1st operator)
            for(j=12;j<16;j++)
            {
                if(current_node->card_board[i][current_node->top[i]].value==current_node->card_board[j][current_node->top[j]].value+1)
                    if(current_node->card_board[i][current_node->top[i]].suit==current_node->card_board[j][current_node->top[j]].suit || current_node->card_board[i][current_node->top[i]].value==0 )
                    {
                        create_child(current_node,foundation,child,i,j,method);
                        child++;
                        break;
                    }
            }
        }
    }
    
    for(i=0;i<12;i++)
    {
        if(current_node->top[i]!=-1)
        {
            //then I check for stacks and mainly for simple stack moves and then for free stacks(2st operator)
            for(j=0;j<8;j++)
            {
                if(i==j)
                    continue;
                if(current_node->card_board[i][current_node->top[i]].value==current_node->card_board[j][current_node->top[j]].value-1 && current_node->top[j]!=-1)
                {
                    if((current_node->card_board[i][current_node->top[i]].suit==HEARTS || current_node->card_board[i][current_node->top[i]].suit==DIAMONDS) && (current_node->card_board[j][current_node->top[j]].suit==SPADES || current_node->card_board[j][current_node->top[j]].suit==CLUBS))
                    {
                        create_child(current_node, stack, child, i, j, method);
                        child++;
                        continue;
                    }
                    if((current_node->card_board[j][current_node->top[j]].suit==HEARTS || current_node->card_board[j][current_node->top[j]].suit==DIAMONDS) && (current_node->card_board[j][current_node->top[j]].suit==SPADES || current_node->card_board[j][current_node->top[j]].suit==CLUBS))
                    {
                        create_child(current_node, stack, child, i, j, method);
                        child++;
                        continue;
                    }
                }
                if(current_node->top[j]==-1)
                {
                    create_child(current_node, new_stack, child, i, j, method);
                    child++;
                }
            }
        }
    }
    for(i=0;i<12;i++)
    {
        if(current_node->top[i]!=-1)
        {
            //At last I check for freecells(3rd operator)
            if(i<8)
            {
                for(j=8;j<12;j++)
                {
                    if (current_node->top[j]==-1)
                    {
                        create_child(current_node,freecell,child,i,j,method);
                        child++;
                        break;
                    }
                }
            }
        }
    }
}

// Create Child Node
void create_child(struct tree_node *current_node, int move, int child, int mcard0_pos, int mcard1_pos, int method)
{
    //global_counter++;
    //printf("%d \n",global_counter);
    int i,j,jj;
    // Initializing the new child
    struct tree_node *child_node;
    // Allocating Space for the new child node.
    child_node = (struct tree_node*)malloc(sizeof(struct tree_node));
    current_node->children[child]=child_node;
    
    for(jj=0;jj<20;jj++)
        child_node->children[jj]=NULL;
    
    child_node->parent=current_node;
    child_node->move=move;
    child_node->g=current_node->g+1;		// The depth of the new child
    child_node->mcard0 = current_node->card_board[mcard0_pos][current_node->top[mcard0_pos]];
    child_node->mcard1 = current_node->card_board[mcard1_pos][current_node->top[mcard1_pos]];
    
    // Copy all positions
    for(i=0;i<16;i++)
    {
        for(j=0;j<(3*N)/2;j++)
        {
            child_node->card_board[i][j]=current_node->card_board[i][j];
            child_node->top[i]=current_node->top[i];
        }
    }
   	//Make the transactions of the cards at the top of the stacks
    //Computing the cardboard of the new child
   	child_node->top[mcard1_pos]++;
   	child_node->card_board[mcard1_pos][child_node->top[mcard1_pos]]=current_node->card_board[mcard0_pos][current_node->top[mcard0_pos]];
    child_node->card_board[mcard0_pos][child_node->top[mcard0_pos]].suit=-1;
    child_node->card_board[mcard0_pos][child_node->top[mcard0_pos]].value=-1;
   	child_node->top[mcard0_pos]--;
    
    //Check for loops
    if (!check_with_parents(child_node))
    {
        // In case of loop detection, the child is deleted
        free(child_node);
        current_node->children[child]=NULL;
        return;
    }
    // Computing the heuristic value
    evaluate_child(child_node,method);
}

//A function with argument one node that returns the number of freecells are in use
int freecell_count(struct tree_node *node)
{
    int i,value;
    for (i=8;i<12;i++)
    {
        if(node->top[i]==0)
        {
            value++;
        }
    }
    return value;
}

//A function that computes and returns the count of the cards at the foundations
int foundation_count(struct tree_node *node)
{
    int value,i;
    value=1;
    for(i=12;i<16;i++)
        if (node->top[i]!=-1)
        {
            value+=node->top[i];
            value++;
        }
    
    return value;
}

//A function that computes and returns how many free stacks exists in the input node
int freestacks_count(struct tree_node *node)
{
    int i, value = 0;
    
    for (i=0;i<8;i++)
    {
        if (node->top[i]==-1)
            value++;
    }
    
    return value;
}
//A function tha use the tree functions above in order to evaluate a node
int heuristic(struct tree_node *node)
{
    int hvalue,freecells,freestacks,foundation_cards;
    
    hvalue=0;
    freestacks=freestacks_count(node)*5;
    freecells=freecell_count(node);
    foundation_cards=foundation_count(node)*20;
    hvalue=foundation_cards-freestacks-freecells;
    
    return hvalue;
}

// This function adds a pointer to a new leaf search-tree node at the front of the frontier.
// This function is called by the depth-first search algorithm.
// Inputs:
//		struct tree_node *node	: A (leaf) search-tree node.
// Output:
//		0 --> The new frontier node has been added successfully.
//		-1 --> Memory problem when inserting the new frontier node .
int add_frontier_front(struct tree_node *node)
{
    // Creating the new frontier node
    struct frontier_node *new_frontier_node=(struct frontier_node*) malloc(sizeof(struct frontier_node));
    if (new_frontier_node==NULL)
        return -1;
    
    new_frontier_node->n=node;
    new_frontier_node->previous=NULL;
    new_frontier_node->next=frontier_head;
    
    if (frontier_head==NULL)
    {
        frontier_head=new_frontier_node;
        frontier_tail=new_frontier_node;
    }
    else
    {
        frontier_head->previous=new_frontier_node;
        frontier_head=new_frontier_node;
    }
    
    return 0;
}

// This function adds a pointer to a new leaf search-tree node at the back of the frontier.
// This function is called by the breadth-first search algorithm.
// Inputs:
//		struct tree_node *node	: A (leaf) search-tree node.
// Output:
//		0 --> The new frontier node has been added successfully.
//		-1 --> Memory problem when inserting the new frontier node .
int add_frontier_back(struct tree_node *node)
{
    // Creating the new frontier node
    struct frontier_node *new_frontier_node=(struct frontier_node*) malloc(sizeof(struct frontier_node));
    if (new_frontier_node==NULL)
        return -1;
    
    new_frontier_node->n=node;
    new_frontier_node->next=NULL;
    new_frontier_node->previous=frontier_tail;
    
    if (frontier_tail==NULL)
    {
        frontier_head=new_frontier_node;
        frontier_tail=new_frontier_node;
    }
    else
    {
        frontier_tail->next=new_frontier_node;
        frontier_tail=new_frontier_node;
    }
    
    return 0;
}

// This function adds a pointer to a new leaf search-tree node within the frontier.
// The frontier is always kept in increasing order wrt the f values of the corresponding
// search-tree nodes. The new frontier node is inserted in order.
// This function is called by the heuristic search algorithm.
// Inputs:
//		struct tree_node *node	: A (leaf) search-tree node.
// Output:
//		0 --> The new frontier node has been added successfully.
//		-1 --> Memory problem when inserting the new frontier node .
int add_frontier_in_order(struct tree_node *node)
{
    // Creating the new frontier node
    struct frontier_node *new_frontier_node=(struct frontier_node*) malloc(sizeof(struct frontier_node));
    if (new_frontier_node==NULL)
        return -1;
    
    new_frontier_node->n=node;
    new_frontier_node->previous=NULL;
    new_frontier_node->next=NULL;
    
    if (frontier_head==NULL)
    {
        frontier_head=new_frontier_node;
        frontier_tail=new_frontier_node;
    }
    else
    {
        struct frontier_node *pt;
        pt=frontier_head;
        // Search in the frontier for the first node that corresponds to either a larger f value
        // or to an equal f value but larger h value
        // Note that for the best first search algorithm, f and h values coincide.
        while (pt!=NULL && (pt->n->f>node->f || (pt->n->f==node->f && pt->n->h>node->h)))
            pt=pt->next;
        
        if (pt!=NULL)
        {
            // new_frontier_node is inserted before pt .
            if (pt->previous!=NULL)
            {
                pt->previous->next=new_frontier_node;
                new_frontier_node->next=pt;
                new_frontier_node->previous=pt->previous;
                pt->previous=new_frontier_node;
            }
            else
            {
                // In this case, new_frontier_node becomes the first node of the frontier.
                new_frontier_node->next=pt;
                pt->previous=new_frontier_node;
                frontier_head=new_frontier_node;
            }
        }
        else
        {
            // if pt==NULL, new_frontier_node is inserted at the back of the frontier
            frontier_tail->next=new_frontier_node;
            new_frontier_node->previous=frontier_tail;
            frontier_tail=new_frontier_node;
        }
    }
    
    return 0;
}

// This function initializes the search, i.e. it creates the root node of the search tree
// and the first node of the frontier.
void initialize_search(struct card cb[16][(3*N)/2], int top[16], int method)
{
    int i,j,jj;
    struct tree_node *root=NULL; //the root of the search tree.
    
    //Initialize search tree
    root=(struct tree_node*) malloc(sizeof(struct tree_node));
    root->parent=NULL;
    root->move=-1;
    
    root->mcard0.suit = -1;
    root->mcard0.value = -1;
    root->mcard1.suit = -1;
    root->mcard1.value = -1;
    
    for(jj=0;jj<20;jj++)
        root->children[jj]=NULL;
    
    //Initialization of the card_board
    for (i=0;i<16;i++)
    {
        for (j=0;j<(3*N)/2;j++)
        {
            root->card_board[i][j].suit = -1;
            root->card_board[i][j].value = -1;
        }
        root->top[i] = -1;
    }
    
    for (i=0;i<8;i++)
    {
        for (j=0;j<top[i]+1;j++)
        {
            root->card_board[i][j].suit = cb[i][j].suit;
            root->card_board[i][j].value = cb[i][j].value;
        }
        root->top[i] = top[i];
    }
    
    root->g=0;
    root->h=heuristic(root);
    if (method==best)
        root->f=root->h;
    else if (method==astar)
        root->f=root->g+root->h;
    else
        root->f=0;
    
    //Initialize frontier
   	add_frontier_front(root);
}

// Giving a (solution) leaf-node of the search tree, this function computes
// the moves of the blank that have to be done, starting from the root cardboard,
// in order to go to the leaf node's cardboard.
// Inputs:
//		struct tree_node *solution_node	: A leaf-node
// Output:
//		The sequence of moves that have to be done, starting from the root cardboard,
//		in order to receive the leaf-node's cardboard, is stored into the global variable solution.
void extract_solution(struct tree_node *solution_node)
{
    int i;
    
    struct tree_node *temp_node=solution_node;
    solution_length=solution_node->g;
    
    solution=(struct solution_move*) malloc(solution_length*sizeof(struct solution_move));
    temp_node=solution_node;
    i=solution_length;
    while (temp_node->parent!=NULL)
    {
        i--;
        solution[i].move=temp_node->move;
        solution[i].card0=temp_node->mcard0;
        solution[i].card1=temp_node->mcard1;
        temp_node=temp_node->parent;
    }
}

// This function writes the solution into a file
// Inputs:
//		char* filename	: The name of the file where the solution will be written.
// Outputs:
//		Nothing (apart from the new file)
void write_solution_to_file(char* filename, int solution_length, struct solution_move *solution)
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
        switch(solution[i].move)
    {
        case 0:
            fprintf(fout,"freecell ");
            if (solution[i].card0.suit == 0)
                fprintf(fout, "H");
            else if (solution[i].card0.suit == 1)
                fprintf(fout, "D");
            else if (solution[i].card0.suit == 2)
                fprintf(fout, "S");
            else if (solution[i].card0.suit == 3)
                fprintf(fout, "C");
            fprintf(fout, "%d \n", solution[i].card0.value);
            break;
        case 1:
            fprintf(fout,"stack ");
            if (solution[i].card0.suit == 0)
                fprintf(fout, "H");
            else if (solution[i].card0.suit == 1)
                fprintf(fout, "D");
            else if (solution[i].card0.suit == 2)
                fprintf(fout, "S");
            else if (solution[i].card0.suit == 3)
                fprintf(fout, "C");
            fprintf(fout, "%d ", solution[i].card0.value);
            if (solution[i].card1.suit == 0)
                fprintf(fout, "H");
            else if (solution[i].card1.suit == 1)
                fprintf(fout, "D");
            else if (solution[i].card1.suit == 2)
                fprintf(fout, "S");
            else if (solution[i].card1.suit == 3)
                fprintf(fout, "C");
            fprintf(fout, "%d \n", solution[i].card1.value);
            break;
        case 2:
            fprintf(fout,"new_stack ");
            if (solution[i].card0.suit == 0)
                fprintf(fout, "H");
            else if (solution[i].card0.suit == 1)
                fprintf(fout, "D");
            else if (solution[i].card0.suit == 2)
                fprintf(fout, "S");
            else if (solution[i].card0.suit == 3)
                fprintf(fout, "C");
            fprintf(fout, "%d \n", solution[i].card0.value);
            break;
        case 3:
            fprintf(fout,"foundation ");
            if (solution[i].card0.suit == 0)
                fprintf(fout, "H");
            else if (solution[i].card0.suit == 1)
                fprintf(fout, "D");
            else if (solution[i].card0.suit == 2)
                fprintf(fout, "S");
            else if (solution[i].card0.suit == 3)
                fprintf(fout, "C");
            fprintf(fout, "%d \n", solution[i].card0.value);			
            break;
    }
    fclose(fout);
}


// This function implements at the higest level the search algorithms.
// The various search algorithms differ only in the way the insert
// new nodes into the frontier, so most of the code is commmon for all algorithms.
// Inputs:
//		Nothing, except for the global variables root, frontier_head and frontier_tail.
// Output:
//		NULL --> The problem cannot be solved
//		struct tree_node*	: A pointer to a search-tree leaf node that corresponds to a solution.
struct tree_node *search(int method)
{
    clock_t t;
    int i, err;
    struct frontier_node *temp_frontier_node;
    struct tree_node *current_node;
    
    while (frontier_head!=NULL)
    {
        t=clock();
        if (t-t1>CLOCKS_PER_SEC*TIMEOUT)
        {
            printf("Timeout\n");
            return NULL;
        }
        
        // Extract the first node from the frontier
        current_node=frontier_head->n;
        
        if (is_solution(current_node))
            return current_node;
        
        // Delete the first node of the frontier
        temp_frontier_node=frontier_head;
        frontier_head=frontier_head->next;
        free(temp_frontier_node);
        if (frontier_head==NULL)
            frontier_tail=NULL;
        else
            frontier_head->previous=NULL;
        
        // Find the children of the extracted node
        find_children(current_node, method);
        // Add children to frontier
        for(i=0;i<20;i++)
        {
            if (current_node->children[i]!=NULL)
            {
                if (method==depth)
                    err=add_frontier_front(current_node->children[i]);
                else if (method==breadth)
                    err=add_frontier_back(current_node->children[i]);
                else if (method==best || method==astar)
                    err=add_frontier_in_order(current_node->children[i]);
                if (err<0)
                {
                    printf("Memory exhausted while creating new frontier node. Search is terminated...\n");
                    return NULL;
                }
            }
            else break;
        }
    }
    return NULL;
}

void syntax_message()
{
    printf("puzzle <method> <input-file> <output-file>\n\n");
    printf("where: ");
    printf("<method> = breadth|depth|best|astar\n");
    printf("<input-file> is a file containing a %dx%d puzzle description.\n",N,N);
    printf("<output-file> is the file where the solution will be written.\n");
}

int main(int argc, char** argv)
{
    int err;
    struct tree_node *solution_node;
    struct card card_board[16][(3*N)/2];		// The initial puzzle read from a file
    int method;				// The search algorithm that will be used to solve the puzzle.
    int top[16];
    
    if (argc!=4)
    {
        printf("Wrong number of arguments. Use correct syntax:\n");
        syntax_message();
        return -1;
    }
    
    method=get_method(argv[1]);
    
    if (method<0)
    {
        printf("Wrong method. Use correct syntax:\n");
        syntax_message();
        return -1;
    }
    
    //Reading the card_board from the input file
    err=read_freecell(argv[2], card_board, top);
    if(err<0)
        return -1;
    
    printf("Solving %s using %s...\n",argv[2],argv[1]);
    t1=clock();
    
    //Making the first root node from which the search will begin and add it to the frontier header
    initialize_search(card_board, top, method);
    
    solution_node=search(method);			// The main call
    
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
