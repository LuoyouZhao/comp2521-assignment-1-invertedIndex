#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "invertedIndex.h"
#include <math.h>
//wrote by LuoyouZhao
char * normaliseWord(char *str){
    int i=0;
    while( str[i] ){
        str[i]=tolower(str[i]);
        i++;
    }
    char end = str[strlen(str)-1];
    if( end=='.' || end==',' || end==';' || end=='?'){
        str[strlen(str)-1]='\0';
    }
    return str;
}

double calculate_tf(char **str, char *word, int length){
    double count=0;
    for(int i=0;i<length;i++){
        if(strcmp(str[i],word)==0) count++;
    }
    double tf=(count/length);
    return tf;
}

void add_FileList(InvertedIndexBST p, char *filename, double tf){
    FileList new=malloc(sizeof(struct FileListNode));
    new->tf=tf;
    new->filename=filename;
    new->next=NULL;
    if(p->fileList==NULL){
        p->fileList=new;
    }else{
        FileList backup=p->fileList;
        while(p->fileList->next!=NULL){
            p->fileList=p->fileList->next;
        }
        p->fileList->next=new;
        p->fileList=backup;
    }
}

InvertedIndexBST add_word(InvertedIndexBST p, char *str){
    if(p==NULL){
        InvertedIndexBST new=malloc(sizeof(struct InvertedIndexNode));
        new->word=str;
        new->fileList=NULL;
        new->left=new->right=NULL;
        return new;
    }
    int diff=strcmp(p->word,str);
    if(diff==0){
        p->word  = str;
    }else if(diff<0){
        p->right = add_word(p->right,str);
    }else if(diff>0){
        p->left  = add_word(p->left,str);
    }
    return p;    
}

InvertedIndexBST find_word(InvertedIndexBST p, char *str){
    if(p==NULL){
        return NULL;
    }else if(strcmp(p->word,str)==0){
        return p;
    }else if(strcmp(p->word,str)<0){
        return find_word(p->right,str);
    }else{
        return find_word(p->left,str);
    }
}

InvertedIndexBST put_file(InvertedIndexBST p, char **str, char *filename, int length){
    char **repeated=malloc(length*sizeof(char *));
    for(int i=0;i<length;i++){
        repeated[i]=malloc(20*sizeof(char));
    }
    int check=0,k=0;                                 //k is the num of repeated words
    for(int i=0;i<length;i++){
        for(int j=0;j<k;j++){
            if(strcmp(repeated[j],str[i])==0){
                check=1;
                break;
            }  
        }
        if(check){
            check=0;
            continue;
        }else{
            check=0;
            repeated[k]=str[i];
            k++;
        }
        if(find_word(p,str[i])==NULL)   p=add_word(p,str[i]);
        add_FileList(find_word(p,str[i]), filename, calculate_tf(str, str[i],length));
    }
    free(repeated);
    return p;
}
char **read_file(char str[1000][20],int normalise,int length){
    char **new_str=malloc(length*sizeof(char *));
    for(int i=0;i<length;i++){
        new_str[i]=malloc(20*sizeof(char));
        normalise ? strcpy(new_str[i],normaliseWord(str[i])) : strcpy(new_str[i],str[i]);
    }
    return new_str;
}

void sort(char **name, int length){
    for(int i=0;i<length;i++){
        int min=i;
        for(int j=i;j<length;j++){
            if (strcmp(name[min],name[j])>0){
                min=j;
            }
        }
        char *swtch=name[i];
        name[i]=name[min];
        name[min]=swtch;
    }
}

InvertedIndexBST generateInvertedIndex(char *collectionFilename){
    FILE *file = fopen(collectionFilename,"r");
    char str[1000][20];
    int length=0;
    while(fscanf(file, "%s", str[length]) != EOF){
        length++;
    }
    char **file_name = read_file(str,0,length);
    sort(file_name,length);
    InvertedIndexBST p=NULL;
    fclose(file);
    for(int i=0;i<length;i++){
        FILE *file = fopen(file_name[i],"r");
        char str[1000][20];
        int F_length=0;
        while(fscanf(file, "%s", str[F_length]) != EOF){
            F_length++;
        }
        fclose(file);
        char **File = read_file(str,1,F_length);
        p=put_file(p, File, file_name[i],F_length);
    }
    return p;
}
void printnode(FileList L){
    while(L!=NULL){
        printf("%s ",L->filename);
        L=L->next;
    }
}
void printInvertedIndex(InvertedIndexBST tree){
    if(tree!=NULL){
        printInvertedIndex(tree->left);
        printf("%s ",tree->word);
        printnode(tree->fileList);
        printf("\n");
        printInvertedIndex(tree->right);        
    }
}
// ----------------------------------part two ----------------------------------
// -----------------------------------------------------------------------------
int strlength(char *str[]){
    int length=0;
    while(str[length]!= NULL){
        length++;
    }
    return length;
}

double count_filenum(InvertedIndexBST tree){
    FileList f = tree->fileList;
    double i=0.0;
    while(f!=NULL){
        f=f->next;
        i++;
    }
    return i;
}

void sort_TfIdfNode(TfIdfList array[],int length){
    int max = 0;
    for(int i=0;i<length;i++){
        max = i;
        for(int j=i;j<length;j++){
            if(array[j]->tfidf_sum > array[max]->tfidf_sum){
                max = j;
            }else if(array[j]->tfidf_sum == array[max]->tfidf_sum && strcmp(array[j]->filename, array[max]->filename)<0){
                max = j;
            }
        }
        TfIdfList swtch = array[i];
        array[i] = array[max];
        array[max] = swtch;
    }
}

TfIdfList calculateTfIdf(InvertedIndexBST tree, char *searchWord , int D){
    InvertedIndexBST new = find_word(tree, searchWord);
    FileList check = new->fileList;
    int length=0;
    while(check!=NULL){
        length++;
        check=check->next;
    }
    TfIdfList array[length];
    FileList f = new->fileList;
    int i = 0;
    double idf = log10(D/count_filenum(new));
    while(f!=NULL){
        TfIdfList p = malloc(sizeof(struct TfIdfNode));
        p->filename = f->filename;
        p->tfidf_sum = idf * (f->tf);
        array[i] = p;
        f = f->next;
        i++;
    }
    sort_TfIdfNode(array,length);
    for(int i=0;i<length-1;i++){
        array[i]->next = array[i+1];
    }  
    array[length-1]->next=NULL;
    return array[0];
}

int find_TfIdfList(TfIdfList *array, char *filename, int length){
    for(int i=0; i<length;i++){
        if(array[i]==NULL) {
            break;
        }
        if(strcmp(array[i]->filename,filename)==0){
            return i;    
        }
    }
    return -1;
}

TfIdfList retrieve(InvertedIndexBST tree, char* searchWords[] , int D){
    int length=strlength(searchWords);
    TfIdfList array[length];
    TfIdfList for_sort_array[D]; 
    for(int i=0;i<D;i++){
        for_sort_array[i]=NULL;
    }
    for(int i=0;i<length;i++){
        array[i] = calculateTfIdf(tree, searchWords[i], D);
    }
    int check=0;
    for(int i=0;i<length;i++){
        TfIdfList p=array[i];
        while(p!=NULL){
            int index=find_TfIdfList(for_sort_array, p->filename,D);
            if(index<0)                         index=check;
            if(for_sort_array[index]==NULL)     for_sort_array[index] = p;
            else                                for_sort_array[index]->tfidf_sum+=p->tfidf_sum;
            if(index==check)                     check++;
            p=p->next;
        }
    }
    sort_TfIdfNode(for_sort_array,D);
    for(int i=0;i<(D-1);i++){
        for_sort_array[i]->next =for_sort_array[i+1];
    }  
    for_sort_array[D-1]->next=NULL;
    return for_sort_array[0];
}
//15 hours on debugs QAQ