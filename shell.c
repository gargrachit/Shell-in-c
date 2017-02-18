#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/resource.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<signal.h>
typedef struct list{
    int pid;
    int length;
    char *comm[100];
    struct list *next;
}list;  
char *s[100],c,path[100],status,*temp,echo[1000],cwd[1024],name[100],*pip[100];
int count,l,count1,flag,i,flag1,redin,redout,stdinCopy,stdoutCopy;
void quit(int num){
    if(num == SIGKILL || num == SIGSTOP || num == SIGINT || num == SIGQUIT || num == SIGCHLD )
        if(c!='\n')
        printf("type quit to exit\n");
}
int spawn_proc (int in, int out)
{
    pid_t pid;
    if ((pid = fork ()) == 0)
    {
        if (in != 0){
            stdinCopy = dup(0);
            dup2 (in, 0);
            close (in);
            redin = 1;
        }
        if (out != 1)
        {
            dup2 (out, 1);
            close (out);
        }
        return execvp (pip[0], pip);
    }
    return pid;
}
void print_echo(){        // function to print echo through array echo which contains the argument to be printed
    int i;
    for( i=0;i<count;i++){
        if( echo[i] != '"' && echo[i] != '\\' ){
            printf("%c",echo[i]);
        }
        else if( echo[i] == '\\' && echo[i+1] == '"' ){
            printf("%c",echo[i+1]);
            i = i+1;
        }
        else if( echo[i] == '\\' && echo[i+1] != '"' )
            printf("\\");
    }
    printf("%c",c);
}
int main(){
    list *back,*head,*tail,*parent;
    back = NULL;
    name[99] = '\0';
    int ret;
    pid_t pid;
    path[0] = '~';
    path[1] = '\0';
    errno = 0;
    struct rusage* memory = malloc(sizeof(struct rusage));
    getrusage(RUSAGE_SELF, memory); 
    redin = 0;
    redout = 0;
    int pipflag = 0;
    //   struct command cmd[100];
    int main_count , j,last_pip;
    while(1){
        int job = 0;
        int kjob = 0;
        int overkill = 0;
        int total_pipes = 0;
        int bg = 0;
        int fg = 0;
        pipflag = 0;
        count1 = 0;
        gethostname(name,99);      // to get the username and store it in array called name
        if(redout  == 1){
            if(dup2(stdoutCopy,1) < 0) return 1;                                                                                          
            close(stdoutCopy);
            redout = 0;
        }
        if(redin  == 1){
            if(dup2(stdinCopy,0) < 0) return 1;                                                                                          
            close(stdinCopy);
            redin = 0;
        }
        signal(SIGINT, quit);  
        signal(SIGQUIT,quit);
        signal(SIGKILL,quit);
        signal(SIGCHLD,quit);
        signal(SIGSTOP,quit);
        printf("%s@%s:%s$", getenv("USER"),name,path);      //printing the prompt 
        status = '0';
        flag = 0;
        flag1 = 0;
        while(1){ 
            //*temp = malloc(100);
            count = 0;
            scanf("%c",&c);     
            if(c == '&')
                bg = 1;
            if( c!='\n'){
                temp = malloc(100);
            }
            if(c == '|'){
                pipflag = 1;
                total_pipes++;
            }
            while( c!='\n' && c!=' ' && c!='\t'){
                temp[count++] = c;
                scanf("%c",&c);	// scanning the command
                if(c == '|'){
                    // printf("pip\n");
                    pipflag  = 1;
                    total_pipes++;
                }
                if(c == '&')
                    bg = 1;
                if(c == ';'){
                    // printf("comma\n");
                    temp[count] = '\0';
                    s[count1++] = temp;
                    s[count1] = 0;
                    pid = fork();
                    ret = execvp(s[0],s);                 // executing the command using execvp function 
                    if(ret < 0){
                        perror("command not found\n");
                        _exit(-1);
                    }
                    _exit(0);
                    wait();
                    count = 0;
                    count1 = 0;
                    scanf("%c",&c);
                }
            }
            if(c!='\n')
                temp[count] = '\0';       // ending the string which contains the command or there argument .
            //printf("%s\n",temp);
            if(flag1 == 1){
                for(i=0;i<=count;i++){
                    echo[i] = temp[i];    // copying the argument for echo command in array and then calling function to print the arguments .
                }
                print_echo();
                if(c == '\n')
                    break;
                else
                    continue;
            }
            if(flag == 1){                // if previous command was cd then we will store the absolute path in an array and pass it to chdir() 
                for(i=0;i<=count;i++){
                    path[i] = temp[i];    
                }
                chdir(path);
                break;
            }
            if(count > 0 && status == 'f'){
                status = 's';
            }
            if( count1 == 0 && count > 0 ){   // implementing the builtin command "echo" , "pwd" , "cd" , and user defined command "pinfo" and "pinfo <pid> "
                if( temp[0] == 'c' && temp[1] == 'd' ){
                    flag = 1;
                    continue;
                }
                else if( temp[0] == 'p' &&  temp[1] == 'i' && temp[2] == 'n' && temp[3] == 'f' && temp[4] == 'o' ){
                    status = 'f';
                }
                else if( temp[0] =='e' && temp[1] == 'c'  && temp[2] == 'h' && temp[3] == 'o' ){
                    flag1 = 1;
                    continue;
                }
                else if(temp[0] == 'p' && temp[1] == 'w' && temp[2] == 'd' ){
                    if( getcwd( cwd , sizeof(cwd )) != NULL )
                        fprintf(stdout,"%s\n", cwd);
                    break;
                }
            }
            if(temp[0] == 'j' && temp[1] == 'o' && temp[2] == 'b' && temp[3] == 's')
                job = 1;
            if(temp[0] == 'k' && temp[1] == 'j' && temp[2] == 'o' && temp[3] == 'b')
                kjob = 1;
            if(temp[0] == 'f' && temp[1] == 'g')
                fg = 1;
            if(temp[0] == 'o' && temp[1] == 'v' && temp[2] == 'e' && temp[3] == 'r' && temp[4] == 'k' && temp[5] == 'i' && temp[6] == 'l' && temp[7] == 'l' )
                overkill = 1;
            if(temp[0] == 'q' && temp[1] == 'u' && temp[2] == 'i' && temp[3] == 't')
                exit(0);
            if(count > 0)
                s[count1++] = temp;
            if( c == '\n')
                break;
        }
        if(status == 'f'){
            printf("pid -- %d\n",getpid());   // if command is only pinfo then printing the pid and continuing to take next command 
            continue;
        }
        if(status == 's'){        // if command is pinfo <pid> then we will store the command "ps -p <pid> -o comm= " in s and pass it to execvp
            s[0] = "ps";
            s[1] = "-p";
            s[2] = temp;
            s[3] = "-o";
            s[4] = "comm=";
            s[5] = 0;
        }
        /*if(status!='s')
          s[count1] = 0;	*/
        if(flag == 1)               // if flag = 1 that means command given was cd that we have already executed so no need to go for execvp 
            continue;
        for(i=count1-1;i>=0;i--){
            if( !strcmp(s[i],">")){
                stdoutCopy = dup(1);
                int f = open(s[i+1], O_RDONLY | O_WRONLY | O_CREAT, S_IRWXU);
                dup2(f,1);
                count1 -= 2;
                close(f);
                redout = 1;
            }
            if( !strcmp(s[i],">>")){
                stdoutCopy = dup(1);
                int f = open(s[i+1], O_RDONLY | O_APPEND | O_WRONLY | O_CREAT, S_IRWXU);
                dup2(f,1);
                count1 -= 2;
                close(f);
                redout = 1;
            }
            if( !strcmp(s[i],"<")){
                stdinCopy = dup(0);
                int gg = open(s[i+1], O_RDONLY );
                dup2(gg,0);
                count1 -= 2;
                close(gg);
                redin = 1;
            }
        }
        /*for(i=0;i<count1;i++)
          printf("without piping %s\n",s[i]);*/
        if(status!='s')
            s[count1] = 0;	
        if( pipflag == 1 ){
            //printf("%d\n",total_pipes);
            int j = 0;
            last_pip = 0;
            int first_pip = 1;
            int local = 0;
            int fd[2];
            int k;
            int in = 0;
            for(i=0;i<count1;i++){
                if( strcmp(s[i],"|") ){
                    pip[j++] = s[i];
                }
                else{
                    /*for(k=0;k<j;k++){
                      printf("%s ",pip[k]);
                      }*/
                    //printf("\n");
                    pip[j] = 0;
                    j = 0;
                    pipe(fd);
                    spawn_proc(in,fd[1]);
                    close(fd[1]);
                    in = fd[0];
                    wait();
                }
            }
            //printf("***********piping***************\n");
            /*  for(k=0;k<j;k++){
                printf("%s ",pip[k]);
                }*/
            pip[j] = 0;
            stdinCopy = dup(0);
            redin = 1;
            if(in != 0)
                dup2(in,0);
            pid = fork();
            if(pid<0){
                perror("Child Proc. not created\n");
                _exit(-1);
            }
            else if(pid == 0){
                ret = execvp(pip[0],pip);                 // executing the command using execvp function 
                if(ret < 0){
                    perror("command not found\n");
                    _exit(-1);
                }
                _exit(0);
            }
            wait();
            continue;
        }
        /* if(job == 1){
           printf("jobs\n");
           temp[0] = -l;
           temp[1] = '\0';
           s[count1++] = temp;
           s[count1] = 0;
           }*/

       /* for(i=0;i<count1;i++)
            printf("without piping %s\n",s[i]);*/
        if(job){
            head = back;
            int counter = 1;
            while(head!=NULL){
                printf("[%d] ",counter);
                for(i=0;i<head->length-1;i++)
                    printf("%s ",head->comm[i]);
                printf("%d\n",head->pid);
                head = head->next;
                counter++;
            }
            continue;
        }

        if(kjob){
            int temppid;
            int counter;
            for(i=0;i<count1;i++)
                printf("%s\n",s[i]);
            printf("kill = %d\n",*s[1] - '0');
            head = back;
            counter = 1;
            while(head!=NULL){
                printf("[%d] ",counter);
                for(i=0;i<head->length-1;i++)
                    printf("%s ",head->comm[i]);
                printf("%d\n",head->pid);
                head = head->next;
                counter++;
            }
            head = back;
            parent = back;
            counter = 1;
            flag = 0;
            while(head!=NULL){
                if(*s[1] - '0' == 1){
                    temppid = back->pid;
                    back = back->next;
                    break;
                }
                if(counter == *s[1] - '0'){
                    temppid = head->pid ;
                    parent->next = head->next;
                    break;
                }
                head = head->next;
                if(flag)
                    parent=parent->next;  
                flag = 1;
                counter++;
            }
            printf("temppid = %d\n",temppid);
            kill(temppid,9);
            continue;
        }
        if(fg){
            int tempid;
            int check = 0;
            int counter;
            head = back;
            parent = back;
            counter = 1;
            int fgflag = 0;
            while(head!=NULL){
                if(*s[1] - '0' == 1){
                    if(back==NULL)
                        printf("error no such background exist\n");
                    else{
                        tempid=back->pid;
                        back = back->next;
                        check = 1;
                    }
                    break;
                }
                if(counter == *s[1] - '0'){
                    tempid = head->pid;
                    parent->next = head->next;
                    check = 1;
                    break;
                }
                head=head->next;
                if(fgflag)
                    parent=parent->next;
                counter++;
                fgflag = 1;
            }
            if(check == 0)
                printf("error no such background exist\n");
            else
                wait(tempid);
            continue;
        }
        if(overkill){
            int killid;
            head = back;
            while(head!=NULL){
                killid = head->pid;
                kill(killid,9);
                head = head->next;
            }
            back = NULL;
            printf("overkill\n");
            continue;
        }
        pid=fork();
        if(bg){
            tail=malloc(sizeof(list));
            tail->pid = pid;
            tail->length = count1;
            for(i=0;i<=count1;i++){
                tail->comm[i] = s[i];
            }
            tail->next=NULL;
            if(back == NULL){
                back = tail;
                //printf("back list\n");
            }
            else{
                head = back;
                while(head->next!=NULL){
                    head = head->next;
                }
                head->next=tail;
            }
            /*head = back;
            while(head != NULL){
                printf("pid = %d lenght=%d\n",head->pid,head->length);
                for(i=0;i<=head->length;i++){
                    printf("%s ",head->comm[i]);
                }
                printf("\n");
                head = head->next;
            }*/
        }
        if(pid<0)
        {
            perror("Child Proc. not created\n");
            _exit(-1);
        }
        else if(pid == 0){
            ret = execvp(s[0],s);                 // executing the command using execvp function 
            if(ret < 0){
                perror("command not found\n");
                _exit(-1);
            }
            _exit(0);
        }
        if(!bg)
            wait(pid);
    }
    return 0;
}

