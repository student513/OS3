#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "lib/user/syscall.h"
/*20191103 inseok : header included*/
#include "threads/vaddr.h"
/**/
#include "threads/synch.h"

/*20191115 PRJ2*/

struct lock open_lock;
/**/
static void syscall_handler (struct intr_frame *);
struct lock lock_file;//11.15 형준
void
syscall_init (void) 
{
  /*20191115 PRJ inseok */
  lock_init(&open_lock);
  /**/
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //1101 형준
  void * temp_esp = f->esp;
  //printf("syscall num : %d\n", *(uint32_t *)(temp_esp));//check
  //printf("f->esp : %x\n",f->esp);//check
  //hex_dump(f->esp, f->esp, 100, 1);//check
  /*if(!is_user_vaddr(f->esp)){//abnormal
        exit(-1);
  }*/

  switch(*(uint32_t *)(temp_esp)){
    case SYS_HALT://
      halt();
      break;
    case SYS_EXIT://
      if(!is_user_vaddr(f->esp + 4)){//abnormal
        exit(-1);
      }
      else{//normal
        exit((int)*(uint32_t *)(f->esp + 4));
      }
     
      break;
    case SYS_EXEC://
      if(!is_user_vaddr(f->esp +4)){
        exit(-1);
      }
      else{
        f->eax = exec((const char*)*(uint32_t *)(f->esp + 4)); //f->eax = exec((const char)*(uint32_t *)(f->esp + 4));
      }
      break;
    case SYS_WAIT://
      if(!is_user_vaddr(f->esp + 4)){
        exit(-1);
      }
      else{
        f->eax = wait((pid_t)*(uint32_t *)(f->esp + 4));
      }
      break;


    //형준 11.12
    case SYS_CREATE://prj2
      if(is_user_vaddr(f->esp+4)&&is_user_vaddr(f->esp+8))
        f->eax = create((const char *)*(uint32_t *)(f->esp+4),(unsigned)*(uint32_t *)(f->esp+8));
      else
        exit(-1);
      break;
    case SYS_REMOVE://prj2
      if(!is_user_vaddr(f->esp+4))
        exit(-1);
      else
        f->eax = remove((const char*)*(uint32_t *)(f->esp+4));
      break;
    case SYS_OPEN://prj2
      if(!is_user_vaddr(f->esp+4))
        exit(-1);
      else 
        f->eax = open((const char*)*(uint32_t *)(f->esp+4));
      break;
    case SYS_FILESIZE://prj2
      if(!is_user_vaddr(f->esp+4))
        exit(-1);
      else
        f->eax = filesize((int)*(uint32_t *)(f->esp+4));
      break;
    //
    
    case SYS_READ://
      if(!is_user_vaddr(f->esp + 4)){exit(-1);}
      else if(!is_user_vaddr(f->esp + 8)){exit(-1);}
      else if(!is_user_vaddr(f->esp + 12)){exit(-1);}
      else{
        f->eax = read((int)*(uint32_t *)(temp_esp+4),(void*)*(uint32_t *)(temp_esp+8),(unsigned)*(uint32_t *)(temp_esp+12));
      }
      break;
    case SYS_WRITE://
      
      f->eax = write((int)*(uint32_t *)(temp_esp+4),(void*)*(uint32_t *)(temp_esp+8),(unsigned)*(uint32_t *)(temp_esp+12));
      break;

    //11.12 형준
    case SYS_SEEK://prj2
      if(is_user_vaddr(f->esp+4)&&is_user_vaddr(f->esp+8))
        seek((int)*(uint32_t *)(f->esp+4),(unsigned)*(uint32_t *)(f->esp+8));
      else
        exit(-1);
      break;
    case SYS_TELL://prj2
      if(!is_user_vaddr(f->esp+4))
        exit(-1);
      else
        f->eax=tell((int)*(uint32_t *)(f->esp+4));
      break;
    case SYS_CLOSE://prj2
      if(!is_user_vaddr(f->esp+4))
        exit(-1);
      else 
        close((int)*(uint32_t *)(f->esp+4));
      break;
    //

      /*191102 inseok*/
    case SYS_FIBONACCI:
      //if(!is_user_vaddr(f->esp + 4)){exit(-1);}
      f->eax = fibonacci((int)*(uint32_t *)(temp_esp+4));
      break;
    case SYS_SUM4:
      f->eax = sum((int)*(uint32_t *)(temp_esp+4),(int)*(uint32_t *)(temp_esp+8),(int)*(uint32_t *)(temp_esp+12),(int)*(uint32_t *)(temp_esp+16));
      break;
      /**/
     default:
      printf("\nsyscall error\n");
  }
  //

  //printf ("system call!\n");
  //thread_exit ();
}

void halt(void){
  shutdown_power_off();
}

void exit(int status){
  int idx=3;
  struct thread *current_t = thread_current();
  current_t -> exit_status = status;//success.  (nonzero == fail to exit)
  printf("%s: exit(%d)\n", thread_name(), status);

 
  while(idx<128){
      if(current_t->fd[idx] !=NULL) //20191114 fd 값이 0일떄안되는듯!!
        close(idx);
      idx++;
  }
  
  thread_exit();
}
int write(int fd, const void *buffer, unsigned size){//11.12 수정필요 // 11.14 수정 형준
  int ret = -1;
  lock_acquire(&open_lock);
  if(!is_user_vaddr(buffer)){
    lock_release(&open_lock);
    exit(-1);  
  }
  if(fd>=3){
    /*
    if (thread_current()->fd[fd]->deny_write) {
        file_deny_write(thread_current()->fd[fd]);
    }*/
    if(thread_current()->fd[fd] != NULL){
      ret = file_write(thread_current()->fd[fd],buffer,size);
    }
    else
    {
      lock_release(&open_lock);
      exit(-1);
    }
  }
  else if(fd==1){
    putbuf(buffer, size);
    ret =  size;
  }
  lock_release(&open_lock);
  return ret;
}
int read(int fd, void* buffer, unsigned size){//11.12 수정필요 // 11.14 if 추가
  
  int ret =-1;
  int i=0;
  uint8_t check;
  lock_acquire(&open_lock);
  if(!is_user_vaddr(buffer)){ //test/read-bad-ptr
    lock_release(&open_lock);
    exit(-1);
  }
  
  if(fd>=3){
    if(thread_current()->fd[fd] != NULL){
      ret = file_read(thread_current()->fd[fd],buffer,size);
    }
    else
    {
      lock_release(&open_lock);
      exit(-1);
    }
  }
  else if(fd==0){
    for (i = 0; i < size; i ++) {
      if (((char *)buffer)[i] == '\0') {
        break;
      }
    }
    ret = i;
  }
  lock_release(&open_lock);
  return ret;
}
int wait(pid_t pid){
  return process_wait(pid);
}
pid_t exec(const char *cmd_line){
  return process_execute(cmd_line); 
}

/*20191102 inseok : functions included*/
int fibonacci(int n){//Return N th value of Fibonacci sequence
  int a = 0,b = 1,ans = 0, i;
  
  if(n==0)
    return 0;
  else if(n==1)
    return 1;
  for(i=2;i<n;i++){
    ans=a+b;
    a=b;
    b=ans;
  }
  //printf("\n피보나치 %d\n",ans);
  return ans;
}
int sum(int a, int b, int c, int d){//Return the sum of a, b, c and d
  return a+b+c+d;
}
/**/

/*PRJ2*/
//11.12 형준
bool create (const char *file, unsigned initial_size){
  if(!file) //create-null
    exit(-1);
  if(!is_user_vaddr(file))//create-bad-ptr
    exit(-1);
  return filesys_create(file,initial_size);
}
bool remove (const char *file){
 //삭제예정
  if(!file)
    exit(-1);
  if(!is_user_vaddr(file))
    exit(-1);
  
  return filesys_remove(file);
}
int open (const char *file){
  struct file* fp;
  int ret =-1;
  int idx =3;

  if(!file) //open-null
    exit(-1);
  if(!is_user_vaddr(file))//open-bad-ptr
    exit(-1);

  lock_acquire(&open_lock);

  fp = filesys_open(file);
  if(fp==NULL){//open-empty
    ret=-1; //수정합시다. inseok
  }
  else{//open normal
    idx = 3;
    
    while(idx<128){
      if(thread_current()->fd[idx]==NULL){
        if (strcmp(thread_current()->name, file) == 0) {
            file_deny_write(fp);
        }  
        break;
      }
      idx++;
    }
    thread_current()->fd[idx] = fp;
    ret = idx; //return fd



  }
  lock_release(&open_lock);
  return ret;
}
int filesize (int fd){
  //return file_length(thread_current()->fd[fd]);
  
  if(thread_current()->fd[fd] != NULL){
    return file_length(thread_current()->fd[fd]);
  }
  else
  {
    exit(-1);
  }
  
}
void seek (int fd, unsigned position){
  //file_seek(thread_current()->fd[fd],position);
  
  if(thread_current()->fd[fd] !=NULL){//있으면
    file_seek(thread_current()->fd[fd],position);
  }
  else
  {
    exit(-1);
  }
}
unsigned tell (int fd){
  //return file_tell(thread_current()->fd[fd]);
  
  if(thread_current()->fd[fd] != NULL){//있으면
    return file_tell(thread_current()->fd[fd]);
  }
  else
  {
    exit(-1);
  }
}
void close (int fd){
  //20191114 : 수정
  struct file* fp;
  
  if(thread_current()->fd[fd] != NULL) {
    fp = thread_current()->fd[fd];
    thread_current()->fd[fd] = NULL;
    return file_close(fp);
  }
  else{//abnormal
    exit(-1);
  }
  //return file_close(thread_current()->fd[fd]);
  /*
  if(thread_current()->fd[fd] !=NULL){
    return file_close(thread_current()->fd[fd]);
  }
  else
  {
    exit(-1);
  }
  */
}
/*PRJ2 done*/
